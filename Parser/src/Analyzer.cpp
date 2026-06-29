#include "Analyzer.h"

#include <iostream>
#include <iomanip>

RuntimeTypeId SemanticAnalyzisData::UNKNOWN_TypeId{ 0 };
RuntimeTypeId SemanticAnalyzisData::VOID_TypeId{ 1 };
RuntimeTypeId SemanticAnalyzisData::BOOL_TypeId{ 2 };
RuntimeTypeId SemanticAnalyzisData::INT_TypeId{ 3 };
RuntimeTypeId SemanticAnalyzisData::FLOAT_TypeId{ 4 };
RuntimeTypeId SemanticAnalyzisData::CHAR_TypeId{ 5 };
RuntimeTypeId SemanticAnalyzisData::STRING_TypeId{ 6 };

//----------------------------------------
static const char* l_reservedKeywords[] =
{
	"void",

	"bool",
	"int",
	"float",
	"string",

	"var",

	"struct",
	"enum",

	"func",
	"branch"

	"if",
	"else",
	"while"
};

//----------------------------------------
static uint32_t l_reservedKeywordCount = sizeof( l_reservedKeywords ) / sizeof( l_reservedKeywords[0] );

//----------------------------------------
struct BuiltInFunctionData
{
	const char* Name;
	RuntimeTypeId ReturnTypeId;

	const IExternalFunction* Func;
};

//----------------------------------------
static BuiltInFunctionData l_builtInFunctions[] =
{
	{ "print", SemanticAnalyzisData::VOID_TypeId, nullptr }
};

//----------------------------------------
static uint32_t l_builtInFunctionCount = sizeof( l_builtInFunctions ) / sizeof( l_builtInFunctions[0] );

#pragma region Analysis

//----------------------------------------
// Sémantique :
// var test:type;
// var test2:casting_type = (casting_type)test;
bool IsCastableTo( const SemanticAnalyzisData& analyzis_data, RuntimeTypeId type, RuntimeTypeId casting_type )
{
	if( type >= analyzis_data.TypeCount || casting_type >= analyzis_data.TypeCount )
		return false;

	const RuntimeType& rType = analyzis_data.Types[type];
	const RuntimeType& rCastingType = analyzis_data.Types[casting_type];

	if( rType.Category == RuntimeTypeCategory::UNKNOWN || rCastingType.Category == RuntimeTypeCategory::UNKNOWN )
		return false;

	if( type == casting_type )
		return true;

	if( rType.Category == RuntimeTypeCategory::INT && rCastingType.Category == RuntimeTypeCategory::FLOAT )
		return true;

	if( rType.Category == RuntimeTypeCategory::FLOAT && rCastingType.Category == RuntimeTypeCategory::INT )
		return true;

	return false;
}

//----------------------------------------
static bool GetRuntimeTypeFromTypeName( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analysis_data, token_index type_name, RuntimeTypeId& out_runtime_type )
{
	if( IsTokenContentEquals( text_data, tokens, type_name, "void" ) )
	{
		out_runtime_type = SemanticAnalyzisData::VOID_TypeId;
		return true;
	}
	else if( IsTokenContentEquals( text_data, tokens, type_name, "bool" ) )
	{
		out_runtime_type = SemanticAnalyzisData::BOOL_TypeId;
		return true;
	}
	else if( IsTokenContentEquals( text_data, tokens, type_name, "int" ) )
	{
		out_runtime_type = SemanticAnalyzisData::INT_TypeId;
		return true;
	}
	else if( IsTokenContentEquals( text_data, tokens, type_name, "float" ) )
	{
		out_runtime_type = SemanticAnalyzisData::FLOAT_TypeId;
		return true;
	}
	else if (IsTokenContentEquals(text_data, tokens, type_name, "char"))
	{
		out_runtime_type = SemanticAnalyzisData::CHAR_TypeId;
		return true;
	}
	else if( IsTokenContentEquals( text_data, tokens, type_name, "string" ) )
	{
		out_runtime_type = SemanticAnalyzisData::STRING_TypeId;
		return true;
	}
	else
	{
		for( uint32_t i = 0; i < analysis_data.TypeCount; ++i )
		{
			const RuntimeType& runtimeType = analysis_data.Types[i];

			if( runtimeType.Category == RuntimeTypeCategory::ENUM )
			{
				const EnumTypeDefinition& enumDef = analysis_data.Enums[runtimeType.Index];

				if( AreTokenContentsEqual( text_data, tokens, type_name, ast.Nodes[enumDef.Node].Enum.Name ) )
				{
					out_runtime_type = i;
					return true;
				}
			}
			else if( runtimeType.Category == RuntimeTypeCategory::STRUCT )
			{
				const StructTypeDefinition& structDef = analysis_data.Structs[runtimeType.Index];

				if( AreTokenContentsEqual( text_data, tokens, type_name, ast.Nodes[structDef.Node].Struct.Name ) )
				{
					out_runtime_type = i;
					return true;
				}
			}
		}
	}

	return false;
}

//----------------------------------------
static bool TryGetEnumTypeFromTypeName(const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, token_index type_name, EnumTypeDefinition& out_enum_type)
{
	RuntimeTypeId rtId;

	if (!GetRuntimeTypeFromTypeName(text_data, tokens, ast, analyzis_data, type_name, rtId))
		return false;

	const RuntimeType& rt = analyzis_data.Types[rtId];

	if (rt.Category != RuntimeTypeCategory::ENUM || rt.Index >= analyzis_data.EnumCount)
		return false;

	out_enum_type = analyzis_data.Enums[rt.Index];

	return true;
}

//----------------------------------------
static bool TryGetStructTypeFromTypeName(const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, token_index type_name, StructTypeDefinition& out_struct_type)
{
	RuntimeTypeId rtId;

	if (!GetRuntimeTypeFromTypeName(text_data, tokens, ast, analyzis_data, type_name, rtId))
		return false;

	const RuntimeType& rt = analyzis_data.Types[rtId];

	if (rt.Category != RuntimeTypeCategory::STRUCT || rt.Index >= analyzis_data.StructCount)
		return false;

	out_struct_type = analyzis_data.Structs[rt.Index];

	return true;
}

#pragma region Nouvelle analyse sémantique

//----------------------------------------
static bool IsBuiltInFunction( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TextData& value, BuiltInFunctionData& out_function_data )
{
	// Ancienne version à la main :
	for( uint32_t i = 0; i < l_builtInFunctionCount; ++i )
	{
		if( AreTextEqual( l_builtInFunctions[i].Name, value ) )
		{
			out_function_data = l_builtInFunctions[i];
			return true;
		}
	}

	// Nouvelle version basée sur un catalogue :
	for (uint32_t i = 0; i < ex_func_catalog.Count; ++i)
	{
		const IExternalFunction* func = ex_func_catalog.Catalog[i];
		const char* name = ex_func_catalog.Catalog[i]->GetName();

		if (AreTextEqual(name, value))
		{
			out_function_data = { name, SemanticAnalyzisData::VOID_TypeId, func };
			return true;
		}
	}

	return false;
}

//----------------------------------------
static bool IsBuiltInFunction( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, token_index token, BuiltInFunctionData& out_function_data )
{
	TextData value;

	if( !GetTokenValue( text_data, tokens, token, value ) )
		return false;	// #TODO : Cette erreur ne signifie pas qu'il y a doublon, mais carrément que le token lui même est invalide. A priori on ne devrait jamais arriver là car le token donné est normalement toujours bon.

	return IsBuiltInFunction( text_data, ex_func_catalog, value, out_function_data );
}

//----------------------------------------
static bool IsReservedKeyword( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TextData& value )
{
	for( uint32_t i = 0; i < l_reservedKeywordCount; ++i )
	{
		if( AreTextEqual( l_reservedKeywords[i], value ) )
			return true;
	}

	// On considère les fonctions built-in comme également reserved
	BuiltInFunctionData t;
	return IsBuiltInFunction( text_data, ex_func_catalog, value, t );

	return false;
}


//----------------------------------------
static bool IsReservedKeyword( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, token_index token )
{
	TextData value;

	if( !GetTokenValue( text_data, tokens, token, value ) )
		return false;	// #TODO : Cette erreur ne signifie pas qu'il y a doublon, mais carrément que le token lui même est invalide. A priori on ne devrait jamais arriver là car le token donné est normalement toujours bon.

	return IsReservedKeyword( text_data, ex_func_catalog, value );
}

//----------------------------------------
struct TokenNameCollection
{
	token_index Names[256];
	uint32_t NameCount{ 0 };
};

//----------------------------------------
static bool AddNameIfUnique( const TextData& text_data, const TokenCollection& tokens, TokenNameCollection& names, token_index new_name)
{
	for( uint32_t i = 0; i < names.NameCount; ++i )
	{
		if( AreTokenContentsEqual( text_data, tokens, names.Names[i], new_name ) )
			return false;
	}

	names.Names[names.NameCount] = new_name;
	++names.NameCount;

	return true;
}

//----------------------------------------
static Error AddNameIfUniqueAndNotReserved( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, TokenNameCollection& names, token_index new_name )
{
	if( IsReservedKeyword( text_data, ex_func_catalog, tokens, new_name ) )
		return Error( ErrorType::RESERVED_KEYWORD, new_name );

	else if( !AddNameIfUnique( text_data, tokens, names, new_name ) )
		return Error( ErrorType::NAME_DUPLICATE, new_name );

	return Error::OK;
}

//----------------------------------------
static Error AnalyseScopeDuplicates( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, ASTNodeCollection& ast, TokenNameCollection& name_collection, ast_node_index scope_node_index )
{
	ASTNode& scopeNode = ast.Nodes[scope_node_index];
	uint32_t scopeNameCountStart = name_collection.NameCount;

	ast_node_index currentInstruction = scopeNode.Scope.FirstInstruction;

	while( currentInstruction != 0 )
	{
		ASTNode& instructionNode = ast.Nodes[currentInstruction];

		switch( instructionNode.Type )
		{
			case ASTNodeType::SCOPE:
				if( Error error = AnalyseScopeDuplicates( text_data, ex_func_catalog, tokens, ast, name_collection, currentInstruction ) )
					return error;
				break;
			case ASTNodeType::IF_BLOCK:
				if( Error error = AnalyseScopeDuplicates( text_data, ex_func_catalog, tokens, ast, name_collection, instructionNode.IfBlock.IfBlock ) )
					return error;

				if( instructionNode.IfBlock.ElseBlock != 0 )
				{
					if( Error error = AnalyseScopeDuplicates( text_data, ex_func_catalog, tokens, ast, name_collection, instructionNode.IfBlock.ElseBlock ) )
						return error;
				}
				break;
			case ASTNodeType::WHILE_LOOP:
				if( Error error = AnalyseScopeDuplicates( text_data, ex_func_catalog, tokens, ast, name_collection, instructionNode.WhileLoop.Block ) )
					return error;
				break;

			case ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION:
				if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, name_collection, instructionNode.VariableDeclarationWOAssignation.Name ) )
					return error;
				break;

			case ASTNodeType::VARIABLE_ASSIGNATION:
			case ASTNodeType::RETURN_CALL:
			case ASTNodeType::FUNCTION_CALL:
				break;
			default:
				return Error( ErrorType::ERROR, 0 );	// On ne devrait jamais arriver là mais par sécurité on remonte une erreur quand même
				break;
		}

		currentInstruction = instructionNode.NextNode;
	}

	name_collection.NameCount = scopeNameCountStart;

	return Error::OK;
}

//----------------------------------------
static Error ComputeStructSize(const TextData& text_data, const TokenCollection& tokens, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data, RuntimeTypeId struct_type_id, RuntimeTypeId root_struct_type_id)
{
	RuntimeType& structRT = analyzis_data.Types[struct_type_id];
	StructTypeDefinition& structDef = analyzis_data.Structs[structRT.Index];

	ASTNode& structNode = ast.Nodes[structDef.Node];

	ast_node_index currentMemberIndex = structNode.Struct.FirstMember;

	while (currentMemberIndex != 0)
	{
		ASTNode& memberNode = ast.Nodes[currentMemberIndex];

		RuntimeTypeId memberRtId;

		// Note : on a déjà validé qu'un membre d'une struct est une variable declaraton
		if (!GetRuntimeTypeFromTypeName(text_data, tokens, ast, analyzis_data, memberNode.VariableDeclarationWOAssignation.Type, memberRtId))
			return Error(ErrorType::UNKNOWN_TYPE, memberNode.VariableDeclarationWOAssignation.Type);

		RuntimeType& memberRT = analyzis_data.Types[memberRtId];

		if (memberRT.Category == RuntimeTypeCategory::STRUCT)
		{
			// Si le type de ce membre correspond au type racine, on a une référence cyclique de type
			if (memberRtId == root_struct_type_id)
				return Error(ErrorType::CYCLIC_TYPE_REFERENCE, memberNode.VariableDeclarationWOAssignation.Type);

			// Si la taille est de 0, ça signifie qu'il faut le calculer
			// #NOTE : dans tous les cas on teste la référence cyclique en premier, sinon on ne va pas détecter les types qui se référencent eux-mêmes
			if (memberRT.Size == 0)
			{
				if (Error error = ComputeStructSize(text_data, tokens, ast, analyzis_data, memberRtId, root_struct_type_id))
					return error;
			}
		}

		structRT.Size += memberRT.Size;

		currentMemberIndex = memberNode.NextNode;
	}

	return Error::OK;
}

//----------------------------------------
static Error BuildTypeSizes(const TextData& text_data, const TokenCollection& tokens, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data)
{
	for (RuntimeTypeId i = 0; i < analyzis_data.TypeCount; ++i)
	{
		if (analyzis_data.Types[i].Category == RuntimeTypeCategory::STRUCT)
		{
			if (Error error = ComputeStructSize(text_data, tokens, ast, analyzis_data, i, i))
				return error;
		}
	}

	return Error::OK;
}


//----------------------------------------
static Error AnalyzeTypesAndDuplicates( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data )
{
	// On initialise les infos de type built-in
	analyzis_data.Types[SemanticAnalyzisData::UNKNOWN_TypeId] = { RuntimeTypeCategory::UNKNOWN, 0, 1 };
	analyzis_data.Types[SemanticAnalyzisData::VOID_TypeId] = { RuntimeTypeCategory::VOID, 0, 1 };
	analyzis_data.Types[SemanticAnalyzisData::BOOL_TypeId] = { RuntimeTypeCategory::BOOL, 0, 1 };
	analyzis_data.Types[SemanticAnalyzisData::INT_TypeId] = { RuntimeTypeCategory::INT, 0, 1 };
	analyzis_data.Types[SemanticAnalyzisData::FLOAT_TypeId] = { RuntimeTypeCategory::FLOAT, 0, 1 };
	analyzis_data.Types[SemanticAnalyzisData::CHAR_TypeId] = { RuntimeTypeCategory::CHAR, 0, 1 };
	analyzis_data.Types[SemanticAnalyzisData::STRING_TypeId] = { RuntimeTypeCategory::STRING, 0, 1 };
	analyzis_data.TypeCount = 7;

	TokenNameCollection knownGlobalNames;

	// On répertorie les définitions d'enum, struct, variables globales et fonction
	//	> On en profite pour détecter les doublons et collisions avec les mots clés réservés
	ast_node_index currentGlobalNodeIndex = 0;
	do
	{
		ASTNode& node = ast.Nodes[currentGlobalNodeIndex];

		if( node.Type == ASTNodeType::ENUM )
		{
			if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownGlobalNames, node.Enum.Name ) )
				return error;
			
			EnumTypeDefinition& enumDef = analyzis_data.Enums[analyzis_data.EnumCount];
			++analyzis_data.EnumCount;

			enumDef.Node = currentGlobalNodeIndex;

			RuntimeType& runtimeType = analyzis_data.Types[analyzis_data.TypeCount];
			++analyzis_data.TypeCount;

			runtimeType.Category = RuntimeTypeCategory::ENUM;
			runtimeType.Index = analyzis_data.EnumCount - 1;
			runtimeType.Size = 1;

			enumDef.TypeId = analyzis_data.TypeCount - 1;
			node.NativeTypeId = enumDef.TypeId;
			node.ContextTypeId = node.NativeTypeId;
		}
		else if( node.Type == ASTNodeType::STRUCT )
		{
			if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownGlobalNames, node.Struct.Name ) )
				return error;
			
			StructTypeDefinition& structDef = analyzis_data.Structs[analyzis_data.StructCount];
			++analyzis_data.StructCount;

			structDef.Node = currentGlobalNodeIndex;

			RuntimeType& runtimeType = analyzis_data.Types[analyzis_data.TypeCount];
			++analyzis_data.TypeCount;

			runtimeType.Category = RuntimeTypeCategory::STRUCT;
			runtimeType.Index = analyzis_data.StructCount - 1;
			runtimeType.Size = 0;	// Indique que la taille est à calculer (fait plus tard)

			structDef.TypeId = analyzis_data.TypeCount - 1;
			node.NativeTypeId = structDef.TypeId;
			node.ContextTypeId = node.NativeTypeId;
		}
		else if( node.Type == ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION )
		{
			if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownGlobalNames, node.VariableDeclarationWOAssignation.Name ) )
				return error;
		}
		else if( node.Type == ASTNodeType::FUNCTION )
		{
			if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownGlobalNames, node.Function.Name ) )
				return error;

			FunctionDefinition& funcDef = analyzis_data.Functions[analyzis_data.FunctionCount];
			++analyzis_data.FunctionCount;

			funcDef.Node = currentGlobalNodeIndex;
		}

		currentGlobalNodeIndex = node.NextNode;
	} while( currentGlobalNodeIndex != 0 );


	// On vérifie que les options d'enums existent, ne sont ni en doublon (localement, ils peuvent avoir le même nom qu'un autre élément global), ni sur un mot clé réservé
	currentGlobalNodeIndex = 0;

	do
	{
		const ASTNode& node = ast.Nodes[currentGlobalNodeIndex];

		if( node.Type == ASTNodeType::ENUM )
		{
			ast_node_index enumValue = node.Enum.FirstValue;

			if( enumValue == 0 )
				return Error( ErrorType::EMPTY_ENUM, node.Enum.Name );

			TokenNameCollection knownNames;

			int compiledValue = 0;

			do 
			{
				ASTNode& enumValueNode = ast.Nodes[enumValue];

				if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownNames, enumValueNode.EnumValue.Value ) )
					return error;

				enumValueNode.EnumValue.CompiledValue = compiledValue;
				enumValueNode.NativeTypeId = node.NativeTypeId;
				enumValueNode.ContextTypeId = enumValueNode.NativeTypeId;
				++compiledValue;

				enumValue = enumValueNode.NextNode;
			} while( enumValue != 0 );
		}

		currentGlobalNodeIndex = node.NextNode;
	} while( currentGlobalNodeIndex != 0 );


	// On vérifie que les membres de struct existent, ne sont ni en doublon (localement, ils peuvent avoir le même nom qu'un autre élément global), ni sur un mot clé réservé
	currentGlobalNodeIndex = 0;

	do
	{
		const ASTNode& node = ast.Nodes[currentGlobalNodeIndex];

		if( node.Type == ASTNodeType::STRUCT )
		{
			ast_node_index structMember = node.Struct.FirstMember;

			if( structMember == 0 )
				return Error( ErrorType::EMPTY_STRUCT, node.Struct.Name );

			TokenNameCollection knownNames;

			do
			{
				const ASTNode& structMemberNode = ast.Nodes[structMember];

				token_index structMemberName = structMemberNode.VariableDeclarationWOAssignation.Name;

				if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownNames, structMemberName ) )
					return error;

				structMember = structMemberNode.NextNode;
			} while( structMember != 0 );
		}

		currentGlobalNodeIndex = node.NextNode;
	} while( currentGlobalNodeIndex != 0 );


	// On identifie les doublons de noms de variables dans les scopes de fonctions
	currentGlobalNodeIndex = 0;

	do
	{
		ASTNode& node = ast.Nodes[currentGlobalNodeIndex];

		if( node.Type == ASTNodeType::FUNCTION || node.Type == ASTNodeType::BRANCH )
		{
			uint32_t functionNameCountStart = knownGlobalNames.NameCount;

			ASTNode& functionNode = node.Type == ASTNodeType::FUNCTION ? node : ast.Nodes[node.Branch.Function];

			// On gère les arguments
			ast_node_index argumentNodeIndex = functionNode.Function.FirstArgument;

			while( argumentNodeIndex != 0 )
			{
				ASTNode& argumentNode = ast.Nodes[argumentNodeIndex];

				if( Error error = AddNameIfUniqueAndNotReserved( text_data, ex_func_catalog, tokens, knownGlobalNames, argumentNode.Argument.Name ) )
					return error;
				
				argumentNodeIndex = argumentNode.NextNode;
			}

			if( Error error = AnalyseScopeDuplicates( text_data, ex_func_catalog, tokens, ast, knownGlobalNames, functionNode.Function.Body ) )
				return error;

			knownGlobalNames.NameCount = functionNameCountStart;
		}
		
		currentGlobalNodeIndex = node.NextNode;

	} while( currentGlobalNodeIndex != 0 );


	if( Error error = BuildTypeSizes( text_data, tokens, ast, analyzis_data) )
		return error;

	return Error::OK;
}


//----------------------------------------
struct NamedVariableAnalyzisData
{
	token_index Name;
	bool Global;
	bool AddressOfAddress{ false };
	RuntimeTypeId NativeTypeId;
	uint32_t LocalAddress;
};

//----------------------------------------
struct NamedVariableAnalyzisDataCollection
{
	NamedVariableAnalyzisData Names[256];
	uint32_t NamesCount{ 0 };
};

//----------------------------------------
static uint32_t GetTypeSize( RuntimeTypeId type_id, const SemanticAnalyzisData& analyzis_data )
{
	return analyzis_data.Types[type_id].Size;
}

//----------------------------------------
enum class NodeAnalyzisContextType : uint8_t
{
	GLOBAL_SCOPE,
	FUNCTION_SCOPE,
	ENUM_MEMBER_CALL,
	STRUCT_MEMBER_CALL,

	OTHER
};

//----------------------------------------
struct NodeAnalyzisContext
{
	NodeAnalyzisContextType Type{ NodeAnalyzisContextType::GLOBAL_SCOPE };
	NodeAnalyzisContext* ParentContext{ nullptr };
	union
	{
		RuntimeTypeId FunctionReturnTypeId;
		RuntimeTypeId EnumTypeId;
		RuntimeTypeId StructTypeId;
	};
};

//----------------------------------------
static RuntimeTypeId GetCurrentFunctionReturnType(const NodeAnalyzisContext& context)
{
	if (context.Type == NodeAnalyzisContextType::FUNCTION_SCOPE)
		return context.FunctionReturnTypeId;
	else if (context.ParentContext != nullptr)
		return GetCurrentFunctionReturnType(*context.ParentContext);
	else
		return 0;
}

//----------------------------------------
static Error SetNodeLocalAddressAndTypeId( const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, NamedVariableAnalyzisDataCollection& names, ast_node_index node_index, NodeAnalyzisContext& context)
{
	ASTNode& node = ast.Nodes[node_index];

	// On vérifie les cohérences node <> context
	if (context.Type == NodeAnalyzisContextType::ENUM_MEMBER_CALL && node.Type != ASTNodeType::SYMBOL_CALL)
		return Error(ErrorType::NOT_AN_ENUM_MEMBER_EXPRESSION, node.ProvokingToken);
	else if(context.Type == NodeAnalyzisContextType::STRUCT_MEMBER_CALL && node.Type != ASTNodeType::SYMBOL_CALL)
		return Error(ErrorType::NOT_A_STRUCT_MEMBER_EXPRESSION, node.ProvokingToken);

	switch( node.Type )
	{
		// Nodes "sources"
		case ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION:
		{
			if( !GetRuntimeTypeFromTypeName( text_data, tokens, ast, analyzis_data, node.VariableDeclarationWOAssignation.Type, node.NativeTypeId ) )
				return Error( ErrorType::UNKNOWN_TYPE, node.VariableDeclarationWOAssignation.Type );

			node.ContextTypeId = node.NativeTypeId;

			if( names.NamesCount == 0 )
				node.VariableDeclarationWOAssignation.LocalAddress = 0;
			else
			{
				NamedVariableAnalyzisData& previousName = names.Names[names.NamesCount - 1];
				node.VariableDeclarationWOAssignation.LocalAddress = previousName.LocalAddress + GetTypeSize( previousName.NativeTypeId, analyzis_data );
			}

			NamedVariableAnalyzisData& name = names.Names[names.NamesCount];
			++names.NamesCount;

			name.NativeTypeId = node.NativeTypeId;
			name.Name = node.VariableDeclarationWOAssignation.Name;
			name.LocalAddress = node.VariableDeclarationWOAssignation.LocalAddress;
			name.Global = node.VariableDeclarationWOAssignation.Global;
			name.AddressOfAddress = false;	// C'est la valeur par défaut, MAIS, comme on recycle la structure de donnée de names, on a les anciennes valeurs !

			if( node.VariableDeclarationWOAssignation.Value != 0 )
			{
				if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.VariableDeclarationWOAssignation.Value, context) )
					return error;

				ast.Nodes[node.VariableDeclarationWOAssignation.Value].ContextTypeId = node.ContextTypeId;
			}

			return Error::OK;
		}
		case ASTNodeType::ARGUMENT:
		{
			if( !GetRuntimeTypeFromTypeName( text_data, tokens, ast, analyzis_data, node.Argument.Type, node.NativeTypeId ) )
				return Error( ErrorType::UNKNOWN_TYPE, node.Argument.Type );

			node.ContextTypeId = node.NativeTypeId;
			node.Usage = analyzis_data.Types[node.ContextTypeId].Category == RuntimeTypeCategory::STRUCT ? UsageContext::ADDRESS_OF_ADDRESS : UsageContext::ADDRESS;

			if( names.NamesCount == 0 )
				node.Argument.LocalAddress = 0;
			else
			{
				NamedVariableAnalyzisData& previousName = names.Names[names.NamesCount - 1];
				node.Argument.LocalAddress = previousName.LocalAddress + GetTypeSize( previousName.NativeTypeId, analyzis_data);
			}

			NamedVariableAnalyzisData& name = names.Names[names.NamesCount];
			++names.NamesCount;

			name.NativeTypeId = node.NativeTypeId;
			name.Name = node.Argument.Name;
			name.LocalAddress = node.Argument.LocalAddress;
			name.Global = false;
			name.AddressOfAddress = true;

			return Error::OK;
		}

		// Nodes "références"
		case ASTNodeType::VARIABLE_ASSIGNATION:
		{
			if (Error error = SetNodeLocalAddressAndTypeId(text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.VariableAssignation.NameX, context))
				return error;

			if (Error error = SetNodeLocalAddressAndTypeId(text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.VariableAssignation.Value, context))
				return error;

			node.NativeTypeId = ast.Nodes[node.VariableAssignation.Value].ContextTypeId;
			node.ContextTypeId = node.NativeTypeId;

			return Error::OK;
		}
		case ASTNodeType::SYMBOL_CALL:
		{
			if (context.Type == NodeAnalyzisContextType::ENUM_MEMBER_CALL)
			{
				const ASTNode& enumNode = ast.Nodes[analyzis_data.Enums[analyzis_data.Types[context.EnumTypeId].Index].Node];

				ast_node_index currentEnumValueNodeIndex = enumNode.Enum.FirstValue;

				while (currentEnumValueNodeIndex != 0)
				{
					ASTNode& enumValueNode = ast.Nodes[currentEnumValueNodeIndex];

					if (AreTokenContentsEqual(text_data, tokens, enumValueNode.EnumValue.Value, node.SymbolCall.Name))
					{
						node.NativeTypeId = context.EnumTypeId;
						node.ContextTypeId = node.NativeTypeId;
						node.Usage = UsageContext::VALUE;
						node.SymbolCall.Context = SymbolContext::ENUM_MEMBER;
						node.SymbolCall.CompiledValue = enumValueNode.EnumValue.CompiledValue;

						return Error::OK;
					}

					currentEnumValueNodeIndex = enumValueNode.NextNode;
				}

				// On n'a pas trouvé de valeur d'enum qui corresponde
				return Error(ErrorType::UNKNOWN_ENUM_VALUE, node.ProvokingToken);
			}
			else if (context.Type == NodeAnalyzisContextType::STRUCT_MEMBER_CALL)
			{
				const ASTNode& structNode = ast.Nodes[analyzis_data.Structs[analyzis_data.Types[context.StructTypeId].Index].Node];

				ast_node_index currentStructMemberNodeIndex = structNode.Struct.FirstMember;

				uint32_t offset = 0;
				while (currentStructMemberNodeIndex != 0)
				{
					ASTNode& structMemberNode = ast.Nodes[currentStructMemberNodeIndex];

					if (AreTokenContentsEqual(text_data, tokens, structMemberNode.VariableDeclarationWOAssignation.Name, node.SymbolCall.Name))
					{
						node.NativeTypeId = structMemberNode.ContextTypeId;
						node.ContextTypeId = node.NativeTypeId;
						node.Usage = UsageContext::ADDRESS;
						node.SymbolCall.CompiledOffset = offset;
						node.SymbolCall.Context = SymbolContext::STRUCT_MEMBER;

						return Error::OK;
					}

					currentStructMemberNodeIndex = structMemberNode.NextNode;
					++offset;
				}

				// On n'a pas trouvé de valeur d'enum qui corresponde
				return Error(ErrorType::UNKNOWN_STRUCT_MEMBER, node.ProvokingToken);
			}
			else
			{
				// On teste si c'est une variable
				for (uint32_t i = 0; i < names.NamesCount; ++i)
				{
					const NamedVariableAnalyzisData& name = names.Names[i];
					if (AreTokenContentsEqual(text_data, tokens, name.Name, node.SymbolCall.Name))
					{
						node.NativeTypeId = name.NativeTypeId;
						node.ContextTypeId = name.NativeTypeId;
						node.Usage = name.AddressOfAddress ? UsageContext::ADDRESS_OF_ADDRESS : UsageContext::ADDRESS;
						node.SymbolCall.LocalAddress = name.LocalAddress;
						node.SymbolCall.Context = name.Global ? SymbolContext::GLOBAL : SymbolContext::SCOPE;
						return Error::OK;
					}
				}

				// On teste si c'est un nom de type enum
				EnumTypeDefinition enumDef;
				if (TryGetEnumTypeFromTypeName(text_data, tokens, ast, analyzis_data, node.SymbolCall.Name, enumDef))
				{
					node.NativeTypeId = enumDef.TypeId;
					node.ContextTypeId = enumDef.TypeId;
					node.SymbolCall.LocalAddress = 0;
					node.SymbolCall.Context = SymbolContext::ENUM_TYPE_NAME;

					return Error::OK;
				}

				return Error(ErrorType::VAR_USE_BEFORE_DECLARATION, node.SymbolCall.Name);
			}
		}

		// Nodes "expressions"
		case ASTNodeType::CONSTANT_BOOL:
			node.NativeTypeId = SemanticAnalyzisData::BOOL_TypeId;
			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;
			return Error::OK;
		case ASTNodeType::CONSTANT_INT:
			node.NativeTypeId = SemanticAnalyzisData::INT_TypeId;
			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;
			return Error::OK;
		case ASTNodeType::CONSTANT_FLOAT:
			node.NativeTypeId = SemanticAnalyzisData::FLOAT_TypeId;
			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;
			return Error::OK;
		case ASTNodeType::CONSTANT_CHARACTER:
			node.NativeTypeId = SemanticAnalyzisData::CHAR_TypeId;
			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;
			return Error::OK;
		case ASTNodeType::CONSTANT_STRING:
			node.NativeTypeId = SemanticAnalyzisData::STRING_TypeId;
			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;		// #TODO : Techniquement c'est une adresse, mais la gestion des string est encore fragile et faire ça permet de correctement s'en servir dans le print
			return Error::OK;

		case ASTNodeType::UNARY_OPERATOR:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.UnaryOperator.Operand, context) )
				return error;

			node.NativeTypeId = ast.Nodes[node.UnaryOperator.Operand].ContextTypeId;
			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;

			const RuntimeType& rType = analyzis_data.Types[node.NativeTypeId];

			if (rType.Category == RuntimeTypeCategory::ENUM)
				node.ContextTypeId = SemanticAnalyzisData::INT_TypeId;

			if (IsTokenContentEquals(text_data, tokens, node.UnaryOperator.Operator, "-"))
			{
				if (node.ContextTypeId == SemanticAnalyzisData::INT_TypeId || node.ContextTypeId == SemanticAnalyzisData::FLOAT_TypeId )
					return Error::OK;
			}
			else if (IsTokenContentEquals(text_data, tokens, node.UnaryOperator.Operator, "!"))
			{
				if (node.ContextTypeId == SemanticAnalyzisData::BOOL_TypeId)
					return Error::OK;
			}

			return Error( ErrorType::INVALID_TYPE, node.UnaryOperator.Operator );
		}
		case ASTNodeType::BINARY_OPERATOR:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.BinaryOperator.FirstOperand, context) )
				return error;

			ASTNode& firstOperandNode = ast.Nodes[node.BinaryOperator.FirstOperand];

			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.BinaryOperator.SecondOperand, context) )
				return error;

			ASTNode& secondOperandNode = ast.Nodes[node.BinaryOperator.SecondOperand];

			const RuntimeType& firstType = analyzis_data.Types[firstOperandNode.ContextTypeId];
			const RuntimeType& secondType = analyzis_data.Types[secondOperandNode.ContextTypeId];

			// On réduit les catégories à enum à la catégorie int pour simplifier les calculs
			RuntimeTypeId firstTypeId = (firstType.Category == RuntimeTypeCategory::INT || firstType.Category == RuntimeTypeCategory::ENUM) ? SemanticAnalyzisData::INT_TypeId : firstOperandNode.ContextTypeId;
			RuntimeTypeId secondTypeId = (secondType.Category == RuntimeTypeCategory::INT || secondType.Category == RuntimeTypeCategory::ENUM) ? SemanticAnalyzisData::INT_TypeId : secondOperandNode.ContextTypeId;

			// Cas spécial : concaténation de string
			if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "+") && firstTypeId == SemanticAnalyzisData::STRING_TypeId /*&& secondTypeId == SemanticAnalyzisData::STRING_TypeId*/)
			{
				node.NativeTypeId = SemanticAnalyzisData::STRING_TypeId;
			}
			// Opérateurs numériques
			else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "+") || 
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "-") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "*") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "/"))
			{
				if (firstTypeId != SemanticAnalyzisData::INT_TypeId && firstTypeId != SemanticAnalyzisData::FLOAT_TypeId)
					return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, firstOperandNode.ProvokingToken);

				if (secondTypeId != SemanticAnalyzisData::INT_TypeId && secondTypeId != SemanticAnalyzisData::FLOAT_TypeId)
					return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, secondOperandNode.ProvokingToken);

				if (firstTypeId == secondTypeId)
					node.NativeTypeId = firstTypeId;
				else if (firstTypeId == SemanticAnalyzisData::INT_TypeId)
				{
					firstOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
					node.NativeTypeId = SemanticAnalyzisData::FLOAT_TypeId;
				}
				else if (secondTypeId == SemanticAnalyzisData::INT_TypeId)
				{
					secondOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
					node.NativeTypeId = SemanticAnalyzisData::FLOAT_TypeId;
				}
			}
			// Opérateurs logiques numériques
			else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "<") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "<=") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, ">") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, ">="))
			{
				if (firstTypeId != SemanticAnalyzisData::INT_TypeId && firstTypeId != SemanticAnalyzisData::FLOAT_TypeId)
					return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, firstOperandNode.ProvokingToken);

				if (secondTypeId != SemanticAnalyzisData::INT_TypeId && secondTypeId != SemanticAnalyzisData::FLOAT_TypeId)
					return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, secondOperandNode.ProvokingToken);

				if (firstTypeId != secondTypeId)
				{
					if (firstTypeId == SemanticAnalyzisData::INT_TypeId)
						firstOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
					else if (secondTypeId == SemanticAnalyzisData::INT_TypeId)
						secondOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
				}

				node.NativeTypeId = SemanticAnalyzisData::BOOL_TypeId;
			}
			// Opérateurs logiques booléens
			else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "&&") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "||"))
			{
				if (firstTypeId != SemanticAnalyzisData::BOOL_TypeId)
					return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, firstOperandNode.ProvokingToken);

				if (secondTypeId != SemanticAnalyzisData::BOOL_TypeId)
					return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, secondOperandNode.ProvokingToken);

				node.NativeTypeId = SemanticAnalyzisData::BOOL_TypeId;
			}
			// Opérateurs hybrides
			else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "==") ||
				IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "!="))
			{
				// #TODO : revoir ça plus clairement et notamment gérer la comparaison de chaînes de caractère

				if (firstTypeId != SemanticAnalyzisData::BOOL_TypeId || secondTypeId != SemanticAnalyzisData::BOOL_TypeId)
				{
					if (firstTypeId != SemanticAnalyzisData::INT_TypeId && firstTypeId != SemanticAnalyzisData::FLOAT_TypeId)
						return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, firstOperandNode.ProvokingToken);

					if (secondTypeId != SemanticAnalyzisData::INT_TypeId && secondTypeId != SemanticAnalyzisData::FLOAT_TypeId)
						return Error(ErrorType::INCOMPATIBLE_OPERAND_TYPES, secondOperandNode.ProvokingToken);

					if (firstTypeId != secondTypeId)
					{
						if (firstTypeId == SemanticAnalyzisData::INT_TypeId)
							firstOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
						else if (secondTypeId == SemanticAnalyzisData::INT_TypeId)
							secondOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
					}
				}

				node.NativeTypeId = SemanticAnalyzisData::BOOL_TypeId;
			}

			node.ContextTypeId = node.NativeTypeId;
			node.Usage = UsageContext::VALUE;

			return Error::OK;
		}
		case ASTNodeType::TERNARY_OPERATOR:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.TernaryOperator.FirstOperand, context) )
				return error;

			ASTNode& firstOperandNode = ast.Nodes[node.TernaryOperator.FirstOperand];

			if (firstOperandNode.ContextTypeId != SemanticAnalyzisData::BOOL_TypeId)
				return Error(ErrorType::INVALID_TYPE, firstOperandNode.ProvokingToken);

			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.TernaryOperator.SecondOperand, context) )
				return error;

			ASTNode& secondOperandNode = ast.Nodes[node.TernaryOperator.SecondOperand];

			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.TernaryOperator.ThirdOperand, context) )
				return error;

			ASTNode& thirdOperandNode = ast.Nodes[node.TernaryOperator.ThirdOperand];

			if( secondOperandNode.ContextTypeId == thirdOperandNode.ContextTypeId )
			{
				node.NativeTypeId = secondOperandNode.ContextTypeId;
			}
			else if( secondOperandNode.ContextTypeId == SemanticAnalyzisData::INT_TypeId && thirdOperandNode.ContextTypeId == SemanticAnalyzisData::FLOAT_TypeId )
			{
				secondOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
				node.NativeTypeId = SemanticAnalyzisData::FLOAT_TypeId;
			}
			else if( secondOperandNode.ContextTypeId == SemanticAnalyzisData::FLOAT_TypeId && thirdOperandNode.ContextTypeId == SemanticAnalyzisData::INT_TypeId )
			{
				thirdOperandNode.ContextTypeId = SemanticAnalyzisData::FLOAT_TypeId;
				node.NativeTypeId = SemanticAnalyzisData::FLOAT_TypeId;
			}
			else
				return Error( ErrorType::INCOMPATIBLE_OPERAND_TYPES, node.TernaryOperator.Operator );	// En l'état, aucune des combinaisons qui reste n'est valide

			node.ContextTypeId = node.NativeTypeId;

			if(firstOperandNode.Usage == UsageContext::ADDRESS && secondOperandNode.Usage == UsageContext::ADDRESS)
				node.Usage = UsageContext::ADDRESS;
			else
				node.Usage = UsageContext::VALUE;

			return Error::OK;
		}

		case ASTNodeType::MEMBER_CALL:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.MemberCall.Parent, context) )
				return error;

			ASTNode& parentNode = ast.Nodes[node.MemberCall.Parent];

			const RuntimeType& rt = analyzis_data.Types[parentNode.ContextTypeId];

			if (rt.Category == RuntimeTypeCategory::ENUM && parentNode.Type == ASTNodeType::SYMBOL_CALL)
			{
				NodeAnalyzisContext enumCallContext{ NodeAnalyzisContextType::ENUM_MEMBER_CALL, &context, parentNode.ContextTypeId };

				if (Error error = SetNodeLocalAddressAndTypeId(text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.MemberCall.Member, enumCallContext))
					return error;

				node.NativeTypeId = ast.Nodes[node.MemberCall.Member].ContextTypeId;
				node.ContextTypeId = node.NativeTypeId;
				node.Usage = UsageContext::VALUE;

				return Error::OK;
			}
			else if (rt.Category == RuntimeTypeCategory::STRUCT && (parentNode.Type == ASTNodeType::SYMBOL_CALL || parentNode.Type == ASTNodeType::FUNCTION_CALL))
			{
				NodeAnalyzisContext enumCallContext{ NodeAnalyzisContextType::STRUCT_MEMBER_CALL, &context, parentNode.ContextTypeId };

				if (Error error = SetNodeLocalAddressAndTypeId(text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.MemberCall.Member, enumCallContext))
					return error;

				node.NativeTypeId = ast.Nodes[node.MemberCall.Member].ContextTypeId;
				node.ContextTypeId = node.NativeTypeId;
				node.Usage = UsageContext::ADDRESS;

				return Error::OK;
			}
			else
			{
				// Ce n'est pas un noeud exprimant un objet possédant des membres
				return Error(ErrorType::NOT_A_COMPOSED_TYPE, parentNode.ProvokingToken);
			}
		}
		case ASTNodeType::FUNCTION_CALL:
		{
			// Type des arguments
			ast_node_index currentArgument = node.FunctionCall.FirstArgument;

			while( currentArgument != 0 )
			{
				if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, currentArgument, context) )
					return error;

				currentArgument = ast.Nodes[currentArgument].NextNode;
			}
			
			// Type de retour
			// Fonctions built-in
			BuiltInFunctionData fData;
			if( IsBuiltInFunction( text_data, ex_func_catalog, tokens, node.FunctionCall.Name, fData ) )
			{
				if (fData.Func != nullptr)
				{
					fData.ReturnTypeId = fData.Func->GetReturnType();

					// On vérifie que les arguments correspondent
					uint32_t currentArgumentDefinition = 0;
					ast_node_index currentArgumentCall = node.FunctionCall.FirstArgument;
					uint32_t argCount = fData.Func->GetArgumentCount();

					while(currentArgumentDefinition < argCount && currentArgumentCall != 0)
					{
						RuntimeTypeId currentArgumentTypeId = fData.Func->GetArgumentType(currentArgumentDefinition);
						ASTNode& currentArgumentCallNode = ast.Nodes[currentArgumentCall];

						if (!IsCastableTo(analyzis_data, currentArgumentCallNode.ContextTypeId, currentArgumentTypeId))
							return Error(ErrorType::ARGUMENT_TYPE_MISMATCH, currentArgumentCallNode.ProvokingToken);
						else
							currentArgumentCallNode.ContextTypeId = currentArgumentTypeId;

						++currentArgumentDefinition;
						currentArgumentCall = currentArgumentCallNode.NextNode;
					}

					if(currentArgumentDefinition != argCount)
						return Error(ErrorType::ARGUMENT_COUNT_MISMATCH, node.FunctionCall.Name);
				}
				
				node.NativeTypeId = fData.ReturnTypeId;
				node.ContextTypeId = node.NativeTypeId;
				// Si on doit consommer le résultat mais qu'en fait il n'y en a pas, on corrige le flag
				if (node.FunctionCall.ConsumeResult && fData.ReturnTypeId == SemanticAnalyzisData::VOID_TypeId)
					node.FunctionCall.ConsumeResult = false;
				return Error::OK;
			}

			// Fonctions utilisateur
			for( uint32_t i = 0; i < analyzis_data.FunctionCount; ++i )
			{
				ASTNode& functionNode = ast.Nodes[analyzis_data.Functions[i].Node];

				if( AreTokenContentsEqual( text_data, tokens, node.FunctionCall.Name, functionNode.Function.Name ) )
				{
					node.NativeTypeId = functionNode.NativeTypeId;
					node.ContextTypeId = node.NativeTypeId;
					node.Usage = functionNode.Usage;

					// Si on doit consommer le résultat mais qu'en fait il n'y en a pas, on corrige le flag
					if (node.FunctionCall.ConsumeResult && functionNode.ContextTypeId == SemanticAnalyzisData::VOID_TypeId)
						node.FunctionCall.ConsumeResult = false;

					// On vérifie que les arguments correspondent
					ast_node_index currentArgumentDefinition = functionNode.Function.FirstArgument;
					ast_node_index currentArgumentCall = node.FunctionCall.FirstArgument;

					while( currentArgumentDefinition != 0 && currentArgumentCall != 0 )
					{
						ASTNode& argumentDefinitionNode = ast.Nodes[currentArgumentDefinition];
						ASTNode& argumentCallNode = ast.Nodes[currentArgumentCall];

						if (!IsCastableTo(analyzis_data, argumentCallNode.ContextTypeId, argumentDefinitionNode.ContextTypeId))
							return Error(ErrorType::ARGUMENT_TYPE_MISMATCH, argumentCallNode.ProvokingToken);
						else
							argumentCallNode.ContextTypeId = argumentDefinitionNode.ContextTypeId;

						currentArgumentDefinition = argumentDefinitionNode.NextNode;
						currentArgumentCall = argumentCallNode.NextNode;
					}

					if( currentArgumentDefinition != currentArgumentCall )	// Les 2 doivent être à 0
						return Error( ErrorType::ARGUMENT_COUNT_MISMATCH, node.FunctionCall.Name );

					return Error::OK;
				}
			}

			return Error( ErrorType::UNKNOWN_FUNCTION, node.FunctionCall.Name );
		}
		case ASTNodeType::RETURN_CALL:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.ReturnCall.Value, context) )
				return error;

			node.NativeTypeId = GetCurrentFunctionReturnType(context);
			node.ContextTypeId = node.NativeTypeId;

			ASTNode& returnValueNode = ast.Nodes[node.ReturnCall.Value];
			returnValueNode.ContextTypeId = node.ContextTypeId;

			if( !IsCastableTo( analyzis_data, returnValueNode.NativeTypeId, returnValueNode.ContextTypeId) )
				return Error( ErrorType::INCOMPATIBLE_RETURN_TYPE, node.ProvokingToken );

			return Error::OK;
		}

		// Nodes "instruction"
		case ASTNodeType::SCOPE:
		{
			// #NOTE : on ne passe pas par ce code pour le scope racine d'une fonction, car on ne doit pas faire la manip sur les names
			// Ce code est reproduit directement dans la gestion des fonctions dans la méthode SetLocalAddressAndTypeId
			uint32_t scopeStartNameCount = names.NamesCount;

			ast_node_index currentInstruction = node.Scope.FirstInstruction;

			while( currentInstruction != 0 )
			{
				if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, currentInstruction, context) )
					return error;

				currentInstruction = ast.Nodes[currentInstruction].NextNode;
			}

			names.NamesCount = scopeStartNameCount;

			node.NativeTypeId = SemanticAnalyzisData::VOID_TypeId;
			node.ContextTypeId = SemanticAnalyzisData::VOID_TypeId;

			return Error::OK;
		}
		case ASTNodeType::IF_BLOCK:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.IfBlock.Condition, context) )
				return error;

			ASTNode& conditionNode = ast.Nodes[node.IfBlock.Condition];

			if (conditionNode.ContextTypeId != SemanticAnalyzisData::BOOL_TypeId)
				return Error(ErrorType::INVALID_TYPE, conditionNode.ProvokingToken);

			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.IfBlock.IfBlock, context) )
				return error;

			if( node.IfBlock.ElseBlock != 0 )
			{
				if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.IfBlock.ElseBlock, context) )
					return error;
			}

			node.NativeTypeId = SemanticAnalyzisData::VOID_TypeId;
			node.ContextTypeId = SemanticAnalyzisData::VOID_TypeId;

			return Error::OK;
		}
		case ASTNodeType::WHILE_LOOP:
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.WhileLoop.Condition, context) )
				return error;

			ASTNode& conditionNode = ast.Nodes[node.WhileLoop.Condition];

			if (conditionNode.ContextTypeId != SemanticAnalyzisData::BOOL_TypeId)
				return Error(ErrorType::INVALID_TYPE, conditionNode.ProvokingToken);

			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, node.WhileLoop.Block, context) )
				return error;

			node.NativeTypeId = SemanticAnalyzisData::VOID_TypeId;
			node.ContextTypeId = SemanticAnalyzisData::VOID_TypeId;

			return Error::OK;
		}

		// NOTE : Struct, Enum, Function et Branch sont gérés à plus haut niveau car ils doivent être traités en premier pour que le reste fonctionne

		default:
			return Error( ErrorType::ERROR, 0 );
	}

	return Error( ErrorType::ERROR, 0 );
}

//----------------------------------------
static Error SetLocalAddressAndTypeId(const TextData& text_data, const ExternalFunctionCatalog& ex_func_catalog, const TokenCollection& tokens, ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data)
{
	NamedVariableAnalyzisDataCollection names;
	NodeAnalyzisContext globalContext{ NodeAnalyzisContextType::GLOBAL_SCOPE, nullptr, 0 };

	// On définit les informations du scope global
	ast_node_index currentNodeIndex = 0;

	do
	{
		ASTNode& currentNode = ast.Nodes[currentNodeIndex];

		if (currentNode.Type == ASTNodeType::ENUM)
		{
			if (!GetRuntimeTypeFromTypeName(text_data, tokens, ast, analyzis_data, currentNode.Enum.Name, currentNode.NativeTypeId))
				return Error(ErrorType::UNKNOWN_TYPE, currentNode.Enum.Name);			// Impossible normalement, l'enum est nécessairement enregistrée comme type

			currentNode.ContextTypeId = currentNode.NativeTypeId;
		}
		else if( currentNode.Type == ASTNodeType::STRUCT )
		{
			ast_node_index currentMember = currentNode.Struct.FirstMember;

			NamedVariableAnalyzisDataCollection structNames;

			while( currentMember != 0 )
			{
				if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, structNames, currentMember, globalContext) )
					return error;

				currentMember = ast.Nodes[currentMember].NextNode;
			}

			if (!GetRuntimeTypeFromTypeName(text_data, tokens, ast, analyzis_data, currentNode.Struct.Name, currentNode.NativeTypeId))
				return Error(ErrorType::UNKNOWN_TYPE, currentNode.Struct.Name);			// Impossible normalement, la struct est nécessairement enregistrée comme type

			currentNode.ContextTypeId = currentNode.NativeTypeId;
		}
		else if( currentNode.Type == ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION )
		{
			if( Error error = SetNodeLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data, names, currentNodeIndex, globalContext) )
				return error;
		}
		else if( currentNode.Type == ASTNodeType::FUNCTION || currentNode.Type == ASTNodeType::BRANCH )
		{
			ast_node_index actualFunctionNodeIndex = currentNode.Type == ASTNodeType::FUNCTION ? currentNodeIndex : currentNode.Branch.Function;

			ASTNode& functionNode = ast.Nodes[actualFunctionNodeIndex];

			if( !GetRuntimeTypeFromTypeName( text_data, tokens, ast, analyzis_data, functionNode.Function.ReturnType, functionNode.NativeTypeId ) )
				return Error( ErrorType::UNKNOWN_TYPE, functionNode.Function.ReturnType );

			functionNode.ContextTypeId = functionNode.NativeTypeId;

			if (currentNode.Type == ASTNodeType::BRANCH)
			{
				currentNode.NativeTypeId = functionNode.NativeTypeId;
				currentNode.ContextTypeId = functionNode.ContextTypeId;
			}
		}

		currentNodeIndex = currentNode.NextNode;

	} while( currentNodeIndex != 0 );


	// On définit les informations des scopes de fonctions
	currentNodeIndex = 0;

	uint32_t globalNameCount = names.NamesCount;

	do
	{
		ASTNode& currentNode = ast.Nodes[currentNodeIndex];

		if( currentNode.Type == ASTNodeType::FUNCTION || currentNode.Type == ASTNodeType::BRANCH )
		{
			ast_node_index actualFunctionNodeIndex = currentNode.Type == ASTNodeType::FUNCTION ? currentNodeIndex : currentNode.Branch.Function;

			ASTNode& functionNode = ast.Nodes[actualFunctionNodeIndex];

			names.NamesCount = globalNameCount;	// Il faut reset la collection de noms sinon on va réutiliser des noms d'un ancien scope fonctionnel

			NodeAnalyzisContext functionContext{ NodeAnalyzisContextType::FUNCTION_SCOPE, &globalContext, functionNode.NativeTypeId };

			// Arguments
			{
				ast_node_index currentArgument = functionNode.Function.FirstArgument;

				while (currentArgument != 0)
				{
					if (Error error = SetNodeLocalAddressAndTypeId(text_data, ex_func_catalog, tokens, ast, analyzis_data, names, currentArgument, functionContext))
						return error;

					currentArgument = ast.Nodes[currentArgument].NextNode;
				}
			}

			// Corps
			{
				// Code repris tel quel de SetNodeLocalAddressAndTypeId sauf la gestion du names.NamesCount car à la racine d'une fonction on veut conserver cette information
				ASTNode& scopeNode = ast.Nodes[functionNode.Function.Body];

				ast_node_index currentInstruction = scopeNode.Scope.FirstInstruction;

				while (currentInstruction != 0)
				{
					if (Error error = SetNodeLocalAddressAndTypeId(text_data, ex_func_catalog, tokens, ast, analyzis_data, names, currentInstruction, functionContext))
						return error;

					currentInstruction = ast.Nodes[currentInstruction].NextNode;
				}
			}

			// #TODO : c'est un peu trop, notamment les instances de struct passées en argument sont comptabilisées entièrement au lieu de 1
			functionNode.Function.ReservedMemorySize = 0;

			for (uint32_t i = globalNameCount; i < names.NamesCount; ++i)
			{
				functionNode.Function.ReservedMemorySize += analyzis_data.Types[names.Names[i].NativeTypeId].Size;
			}
		}

		currentNodeIndex = currentNode.NextNode;

	} while( currentNodeIndex != 0 );

	return Error::OK;
}

#pragma endregion


//----------------------------------------
Error AnalyzeAST( const TextData& text_data, const TokenCollection& tokens, const ExternalFunctionCatalog& ex_func_catalog, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data )
{
	if( Error error = AnalyzeTypesAndDuplicates( text_data, ex_func_catalog, tokens, ast, analyzis_data ) )
		return error;

	if( Error error = SetLocalAddressAndTypeId( text_data, ex_func_catalog, tokens, ast, analyzis_data ) )
		return error;

	return Error::OK;
}

#pragma endregion

#pragma region Printing

//----------------------------------------
static void PrintWithIndent( const char* content, uint32_t indent )
{
	while( indent > 0 )
	{
		std::cout << "  ";
		--indent;
	}

	std::cout << content;
}

//----------------------------------------
static void PrintTypeId(const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, RuntimeTypeId type_id)
{
	const RuntimeType& rType = analyzis_data.Types[type_id];

	switch (rType.Category)
	{
	case RuntimeTypeCategory::UNKNOWN:
		std::cout << "[UNKNOWN]";
		break;
	case RuntimeTypeCategory::VOID:
		std::cout << "[VOID]";
		break;
	case RuntimeTypeCategory::BOOL:
		std::cout << "[BOOL]";
		break;
	case RuntimeTypeCategory::INT:
		std::cout << "[INT]";
		break;
	case RuntimeTypeCategory::FLOAT:
		std::cout << "[FLOAT]";
		break;
	case RuntimeTypeCategory::CHAR:
		std::cout << "[CHAR]";
		break;
	case RuntimeTypeCategory::STRING:
		std::cout << "[STRING]";
		break;
	case RuntimeTypeCategory::ENUM:
	{
		std::cout << "[ENUM ";
		const EnumTypeDefinition& enumDef = analyzis_data.Enums[rType.Index];
		PrintTokenValue(text_data, tokens, ast.Nodes[enumDef.Node].Enum.Name, false);
		std::cout << "]";
	}
	break;
	case RuntimeTypeCategory::STRUCT:
	{
		std::cout << "[STRUCT ";
		const StructTypeDefinition& structDef = analyzis_data.Structs[rType.Index];
		PrintTokenValue(text_data, tokens, ast.Nodes[structDef.Node].Struct.Name, false);
		std::cout << "]";
	}
	break;
	}
}

//----------------------------------------
static void PrintASTNode( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, uint32_t ast_cursor, uint32_t indent )
{
	bool printTokenId = true;

	if( ast_cursor < ast.NodeCount )
	{
		const ASTNode& node = ast.Nodes[ast_cursor];

		PrintWithIndent(GetASTNodeName(node.Type), indent);
		std::cout << ": ";
		PrintTypeId(text_data, tokens, ast, analyzis_data, node.NativeTypeId);
		std::cout << " > ";
		PrintTypeId(text_data, tokens, ast, analyzis_data, node.ContextTypeId);
		std::cout << "\n";

		switch( node.Type )
		{
			case ASTNodeType::BRANCH:
				PrintWithIndent( "Function:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.Branch.Function, indent + 2 );
				break;
			case ASTNodeType::FUNCTION:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.Function.Name, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Return Type: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.Function.ReturnType, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Arguments:\n", indent + 1 );
				if( node.Function.FirstArgument != 0 )
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.Function.FirstArgument, indent + 2 );
				PrintWithIndent( "Body:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.Function.Body, indent + 2 );
				break;
			case ASTNodeType::ARGUMENT:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.Argument.Name, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Compiled Name: ", indent + 1 );
				std::cout << node.Argument.LocalAddress << "\n";
				PrintWithIndent( "Type: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.Argument.Type, printTokenId);
				std::cout << "\n";
				break;
			case ASTNodeType::SCOPE:
				if( node.Scope.FirstInstruction != 0 )
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.Scope.FirstInstruction, indent + 1 );
				break;
			
			case ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.VariableDeclarationWOAssignation.Name, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Compiled Name: ", indent + 1 );
				std::cout << node.VariableDeclarationWOAssignation.LocalAddress << (node.VariableDeclarationWOAssignation.Global ? " (global)" : "") << "\n";
				PrintWithIndent( "Type: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.VariableDeclarationWOAssignation.Type, printTokenId);
				std::cout << "\n";
				if( node.VariableDeclarationWOAssignation.Value != 0 )
				{
					PrintWithIndent( "Value:\n", indent + 1 );
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.VariableDeclarationWOAssignation.Value, indent + 2 );
				}
				break;
			case ASTNodeType::VARIABLE_ASSIGNATION:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintASTNode(text_data, tokens, ast, analyzis_data, node.VariableAssignation.NameX, indent + 2);
				PrintWithIndent( "Value:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.VariableAssignation.Value, indent + 2 );
				break;
			case ASTNodeType::FUNCTION_CALL:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.FunctionCall.Name, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Arguments:\n", indent + 1 );
				if( node.FunctionCall.FirstArgument != 0 )
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.FunctionCall.FirstArgument, indent + 2 );
				break;
			case ASTNodeType::CONSTANT_CHARACTER:
				PrintWithIndent("Value: ", indent + 1);
				PrintTokenValue(text_data, tokens, node.ConstantChar.StrValue, printTokenId);
				std::cout << "\n";
				break;
			case ASTNodeType::CONSTANT_STRING:
				PrintWithIndent( "Value: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.ConstantString.StrValue, printTokenId);
				std::cout << "\n";
				break;
			case ASTNodeType::CONSTANT_FLOAT:
				PrintWithIndent( "String Value: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.ConstantFloat.StrValue, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Float Value: ", indent + 1 );
				std::cout << node.ConstantFloat.Value << "\n";
				break;
			case ASTNodeType::CONSTANT_INT:
				PrintWithIndent( "String Value: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.ConstantInt.StrValue, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Int Value: ", indent + 1 );
				std::cout << node.ConstantInt.Value << "\n";
				break;
			case ASTNodeType::CONSTANT_BOOL:
				PrintWithIndent( "String Value: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.ConstantBool.StrValue, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Bool Value: ", indent + 1 );
				std::cout << ( node.ConstantBool.Value ? "true" : "false" ) << "\n";
				break;
			case ASTNodeType::SYMBOL_CALL:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.SymbolCall.Name, printTokenId);
				switch (node.SymbolCall.Context)
				{
					case SymbolContext::ENUM_TYPE_NAME:
						std::cout << " (enum type)\n";
						break;
					case SymbolContext::ENUM_MEMBER:
						std::cout << " (enum member)\n";
						break;
					case SymbolContext::STRUCT_MEMBER:
						std::cout << " (struct member)\n";
						break;
					case SymbolContext::SCOPE:
						std::cout << " (variable)\n";
						PrintWithIndent("Compiled Name: ", indent + 1);
						std::cout << node.SymbolCall.LocalAddress << "\n";
						break;
					case SymbolContext::GLOBAL:
						std::cout << " (global variable)\n";
						PrintWithIndent("Compiled Name: ", indent + 1);
						std::cout << node.SymbolCall.LocalAddress << "\n";
						break;
				}
				break;
			case ASTNodeType::UNARY_OPERATOR:
				PrintWithIndent( "Operator: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.UnaryOperator.Operator, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Operand:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.UnaryOperator.Operand, indent + 2 );
				break;
			case ASTNodeType::BINARY_OPERATOR:
				PrintWithIndent( "Operator: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.BinaryOperator.Operator, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "First Operand:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.BinaryOperator.FirstOperand, indent + 2 );
				PrintWithIndent( "Second Operand:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.BinaryOperator.SecondOperand, indent + 2 );
				break;
			case ASTNodeType::TERNARY_OPERATOR:
				PrintWithIndent( "Operator: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.TernaryOperator.Operator, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "First Operand:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.TernaryOperator.FirstOperand, indent + 2 );
				PrintWithIndent( "Second Operand:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.TernaryOperator.SecondOperand, indent + 2 );
				PrintWithIndent( "Third Operand:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.TernaryOperator.ThirdOperand, indent + 2 );
				break;
			case ASTNodeType::MEMBER_CALL:
				PrintWithIndent( "Parent:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.MemberCall.Parent, indent + 2 );
				PrintWithIndent( "Member:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.MemberCall.Member, indent + 2 );
				break;
			case ASTNodeType::RETURN_CALL:
				PrintWithIndent( "Value:\n", indent + 1 );
				if( node.ReturnCall.Value != 0 )
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.ReturnCall.Value, indent + 2 );
				break;
			case ASTNodeType::IF_BLOCK:
				PrintWithIndent( "Condition:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.IfBlock.Condition, indent + 2 );
				PrintWithIndent( "If Block:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.IfBlock.IfBlock, indent + 2 );
				if( node.IfBlock.ElseBlock != 0 )
				{
					PrintWithIndent( "Else Block:\n", indent + 1 );
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.IfBlock.ElseBlock, indent + 2 );
				}
				break;
			case ASTNodeType::WHILE_LOOP:
				PrintWithIndent( "Condition:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.WhileLoop.Condition, indent + 2 );
				PrintWithIndent( "Block:\n", indent + 1 );
				PrintASTNode( text_data, tokens, ast, analyzis_data, node.WhileLoop.Block, indent + 2 );
				break;
			case ASTNodeType::STRUCT:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.Struct.Name, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Members:\n", indent + 1 );
				if( node.Struct.FirstMember != 0 )
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.Struct.FirstMember, indent + 2 );
				break;
			case ASTNodeType::ENUM:
				PrintWithIndent( "Name: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.Enum.Name, printTokenId);
				std::cout << "\n";
				PrintWithIndent( "Values:\n", indent + 1 );
				if( node.Enum.FirstValue != 0 )
					PrintASTNode( text_data, tokens, ast, analyzis_data, node.Enum.FirstValue, indent + 2 );
				break;
			case ASTNodeType::ENUM_VALUE:
				PrintWithIndent( "Value: ", indent + 1 );
				PrintTokenValue( text_data, tokens, node.EnumValue.Value, printTokenId);
				std::cout << " = " << node.EnumValue.CompiledValue << "\n";
				break;
			default:
				break;
		}

		// Chaînage générique, utilisé pour la liste d'arguments, d'instructions, etc.
		if( node.NextNode != 0 )
			PrintASTNode( text_data, tokens, ast, analyzis_data, node.NextNode, indent );
	}
}

//----------------------------------------
void PrintAST( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data )
{
	std::cout << "########## AST:\n";

	PrintASTNode( text_data, tokens, ast, analyzis_data, 0, 0 );
}

//----------------------------------------
void PrintAnalyzis( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data )
{
	std::cout << "########## TYPES:\n";

	for (RuntimeTypeId i = 0; i < analyzis_data.TypeCount; ++i)
	{
		std::cout << "  ";
		PrintTypeId(text_data, tokens, ast, analyzis_data, i);
		std::cout << " size: " << GetTypeSize(i, analyzis_data) << "\n";
	}
}

#pragma endregion