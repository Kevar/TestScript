#include "Compiler.h"

#include <iostream>
#include <iomanip>
#include <cctype>

//----------------------------------------
uint32_t GetOpCodeSize(OpCode op_code)
{
	switch (op_code)
	{
	case OpCode::CREATE_MEMORY:
		return 9;	// emplacement mémoire + taille mémoire
	case OpCode::RESERVE_MEMORY_STACK:
	case OpCode::COPY_MEMORY:
	case OpCode::PUSH_LOCAL_ADDRESS:
	case OpCode::PUSH_GLOBAL_ADDRESS:
	case OpCode::JUMP:
	case OpCode::JUMP_IF:
	case OpCode::CALL_FUNCTION:
	case OpCode::CALL_EXTERNAL_FUNCTION:
	case OpCode::PUSH_CONSTANT_INDEX:
	case OpCode::PUSH_CONSTANT_INT:
	case OpCode::PUSH_CONSTANT_FLOAT:
	case OpCode::PUSH_CONSTANT_STRING:
		return 5;
	case OpCode::PUSH_CONSTANT_BOOL:
		return 2;
	default:
		return 1;
	}
}

//----------------------------------------
static void EmitOperation( CompiledProgram& program, OpCode op_code )
{
	program.Ops[program.OpCount] = op_code;
	program.OpCount++;
}

//----------------------------------------
// Sémantique :
// var test:type;
// var test2:casting_type = (casting_type)test;
static bool EmitCastIfNeeded( CompiledProgram& program, const SemanticAnalyzisData& analyzis_data, RuntimeTypeId type, RuntimeTypeId casting_type )
{
	if( type == casting_type )
		return true;

	const RuntimeType& rType = analyzis_data.Types[type];
	const RuntimeType& rCastingType = analyzis_data.Types[casting_type];

	if( (rType.Category == RuntimeTypeCategory::INT || rType.Category == RuntimeTypeCategory::ENUM) && rCastingType.Category == RuntimeTypeCategory::FLOAT )
	{
		EmitOperation( program, OpCode::CAST_INT_TO_FLOAT );
		return true;
	}

	if( rType.Category == RuntimeTypeCategory::FLOAT && rCastingType.Category == RuntimeTypeCategory::INT )
	{
		EmitOperation( program, OpCode::CAST_FLOAT_TO_INT );
		return true;
	}


	if (rType.Category == RuntimeTypeCategory::ENUM && rCastingType.Category == RuntimeTypeCategory::INT)
	{
		// Pas de cast nécessaire, ces types sont explicitement compatibles dans le layout mémoire
		// #NOTE : on ne teste pas dans l'autre sens, les enums sont fortement typées et ne doivent donc pas pouvoir être assigné depuis un int simple
		return true;
	}

	if (rType.Category == RuntimeTypeCategory::ENUM && rCastingType.Category == RuntimeTypeCategory::FLOAT)
	{
		EmitOperation(program, OpCode::CAST_INT_TO_FLOAT);
		return true;
	}

	return false;
}

//----------------------------------------
static void EmitPushConstant( CompiledProgram& program, int32_t constant_value )
{
	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_INT;

	MemoryBlock mb;

	mb.i32 = constant_value;
	program.Ops[program.OpCount + 1] = (OpCode) mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode) mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode) mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode) mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static void EmitPushConstant(CompiledProgram& program, uint32_t constant_value)
{
	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_INDEX;

	MemoryBlock mb;

	mb.index = constant_value;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static void EmitPushConstant( CompiledProgram& program, float constant_value )
{
	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_FLOAT;

	MemoryBlock mb;

	mb.f32 = constant_value;
	program.Ops[program.OpCount + 1] = (OpCode) mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode) mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode) mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode) mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static void EmitPushConstant( CompiledProgram& program, bool constant_value )
{
	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_BOOL;
	program.Ops[program.OpCount + 1] = (OpCode) ( constant_value ? (uint8_t) 1 : (uint8_t) 0 );

	program.OpCount += 2;
}

//----------------------------------------
static void EmitPushConstantString(CompiledProgram& program, uint32_t constant_address)
{
	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_STRING;

	MemoryBlock mb;

	mb.index = constant_address;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static uint32_t EmitJump(CompiledProgram& program, uint32_t instruction_target)
{
	program.Ops[program.OpCount] = OpCode::JUMP;

	MemoryBlock mb;

	mb.index = instruction_target;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;

	return program.OpCount - 5;
}

//----------------------------------------
static uint32_t EmitJumpIf(CompiledProgram& program)
{
	program.Ops[program.OpCount] = OpCode::JUMP_IF;
	
	// On ne fait que réserver la place pour l'emplacement de l'instruction
	program.OpCount += 5;

	return program.OpCount - 5;
}

//----------------------------------------
static void SetJumpTarget(CompiledProgram& program, uint32_t cursor)
{
	if (program.Ops[cursor] != OpCode::JUMP && program.Ops[cursor] != OpCode::JUMP_IF)
		return;

	MemoryBlock mb;

	mb.index = program.OpCount;		// Ce sera la prochaine opération
	program.Ops[cursor + 1] = (OpCode)mb.store[0];
	program.Ops[cursor + 2] = (OpCode)mb.store[1];
	program.Ops[cursor + 3] = (OpCode)mb.store[2];
	program.Ops[cursor + 4] = (OpCode)mb.store[3];
}

//----------------------------------------
static void EmitCallFunction( CompiledProgram& program, uint32_t function_first_operation )
{
	program.Ops[program.OpCount] = OpCode::CALL_FUNCTION;

	MemoryBlock mb;

	mb.index = function_first_operation;
	program.Ops[program.OpCount + 1] = (OpCode) mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode) mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode) mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode) mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static void EmitCallExternalFunction(CompiledProgram& program, uint32_t external_function_index)
{
	program.Ops[program.OpCount] = OpCode::CALL_EXTERNAL_FUNCTION;

	MemoryBlock mb;

	mb.index = external_function_index;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static void ReplaceCallFunction( CompiledProgram& program, uint32_t function_first_operation, uint32_t cursor )
{
	if (program.Ops[cursor] != OpCode::CALL_FUNCTION)
		return;

	MemoryBlock mb;

	mb.index = function_first_operation;
	program.Ops[cursor + 1] = (OpCode) mb.store[0];
	program.Ops[cursor + 2] = (OpCode) mb.store[1];
	program.Ops[cursor + 3] = (OpCode) mb.store[2];
	program.Ops[cursor + 4] = (OpCode) mb.store[3];
}

//----------------------------------------
static void EmitCreateMemory( CompiledProgram& program, uint32_t compiled_name, uint32_t size )
{
	program.Ops[program.OpCount] = OpCode::CREATE_MEMORY;

	MemoryBlock mb;

	mb.index = compiled_name;
	program.Ops[program.OpCount + 1] = (OpCode) mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode) mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode) mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode) mb.store[3];

	mb.index = size;
	program.Ops[program.OpCount + 5] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 6] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 7] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 8] = (OpCode)mb.store[3];

	program.OpCount += 9;
}

//----------------------------------------
static void EmitReserveMemoryStack(CompiledProgram& program, uint32_t reserved_size)
{
	program.Ops[program.OpCount] = OpCode::RESERVE_MEMORY_STACK;

	MemoryBlock mb;

	mb.index = reserved_size;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;
}


//----------------------------------------
static void EmitPushAddress(CompiledProgram& program, uint32_t address, bool global)
{
	program.Ops[program.OpCount] = global ? OpCode::PUSH_GLOBAL_ADDRESS : OpCode::PUSH_LOCAL_ADDRESS;

	MemoryBlock mb;

	mb.index = address;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static void EmitOffsetAddress(CompiledProgram& program, uint32_t offset)
{
	EmitPushConstant(program, offset);

	program.Ops[program.OpCount] = OpCode::OFFSET_ADDRESS;

	//MemoryBlock mb;
	//
	//mb.index = offset;
	//program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	//program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	//program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	//program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount++;
}

//----------------------------------------
static void EmitCopyMemory(CompiledProgram& program, uint32_t size)
{
	program.Ops[program.OpCount] = OpCode::COPY_MEMORY;

	MemoryBlock mb;

	mb.index = size;
	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	program.OpCount += 5;
}

//----------------------------------------
static bool AddFunctionReference( CompilationData& compilation_data, token_index name, uint32_t first_operation )
{
	if( compilation_data.FunctionRefCount >= 127 )
	{
		std::cout << "[ERROR] Too many functions!\n";
		return false;
	}

	FunctionDefRef& funcRef = compilation_data.FunctionRefs[compilation_data.FunctionRefCount];
	funcRef.Name = name;
	funcRef.FirstOperation = first_operation;
	++compilation_data.FunctionRefCount;

	return true;
}

//----------------------------------------
static Error GetFunctionReference( const CompilationData& compilation_data, const TextData& text_data, const TokenCollection& tokens, token_index name, uint32_t& out_first_operation )
{
	for( uint32_t i = 0; i < compilation_data.FunctionRefCount; ++i )
	{
		const FunctionDefRef& funcRef = compilation_data.FunctionRefs[i];

		if( AreTokenContentsEqual( text_data, tokens, funcRef.Name, name ) )
		{
			out_first_operation = funcRef.FirstOperation;
			return Error::OK;
		}
	}

	return Error(ErrorType::UNKNOWN_FUNCTION, name);
}

//----------------------------------------
static Error CompileNode( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, const ExternalFunctionCatalog& ex_func_catalog, ast_node_index cursor, CompilationData& compilation_data, CompiledProgram& program )
{
	const ASTNode& node = ast.Nodes[cursor];

	switch( node.Type )
	{
		case ASTNodeType::ENUM:
		{
			return Error::OK;	// Une enum n'a pas besoin d'être compilée
		}
		break;
		case ASTNodeType::ARGUMENT:
		{
			// On traite les arguments en ordre inverse pour correspondre à l'ordre des données dans la stack d'exécution
			if( node.NextNode != 0 )
			{
				if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.NextNode, compilation_data, program ) )
					return error;
			}

			EmitCreateMemory( program, node.Argument.LocalAddress, 1 );		// #NOTE : comme on utilise les structs en référence, la taille reste bonne, sinon il faudrait créer un espace mémoire complet
			EmitPushAddress(program, node.Argument.LocalAddress, false);

			EmitOperation(program, OpCode::POP_TO_ADDRESS);
		}
		break;
		case ASTNodeType::FUNCTION:
		{
			if( IsTokenContentEquals( text_data, tokens, node.Function.Name, "main" ) )
				compilation_data.MainNameToken = node.Function.Name;

			AddFunctionReference( compilation_data, node.Function.Name, program.OpCount );

			EmitReserveMemoryStack(program, node.Function.ReservedMemorySize);

			// On gère la récupération des arguments
			if( node.Function.FirstArgument != 0 )
			{
				if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.Function.FirstArgument, compilation_data, program ) )
					return error;
			}
		
			if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.Function.Body, compilation_data, program ) )
				return error;

			// #NOTE : si la dernière instruction d'une fonction est un return explicite, on peut déjà avoir un EXIT_FUNCTION.
			// Mais on n'optimise pas car cet exit peut être dans une branche de condition ou une boucle.
			// Pour garantir que tout se passe bien, on émet donc nécessairement une opération après compilation du contenu de la fonction
			EmitOperation( program, OpCode::EXIT_FUNCTION );
		}
		break;
		case ASTNodeType::FUNCTION_CALL:
		{
			// Functions built-in (aka external function aka standard library)
			bool externalFunctionFound = false;
			uint32_t externalFunctionIndex = 0;
			for (externalFunctionIndex = 0; externalFunctionIndex < ex_func_catalog.Count; ++externalFunctionIndex)
			{
				const IExternalFunction* func = ex_func_catalog.Catalog[externalFunctionIndex];

				if (IsTokenContentEquals(text_data, tokens, node.FunctionCall.Name, func->GetName()))
				{
					// #TODO : pour l'instant on ne fait que des fonctions sans paramètre pour tester la tuyauterie
					// Code à fusionner à la partie traitant les appels de fonctions utilisateur mais en remplaçant le EmitCallFunction par le EmitCallExternalFunction
					//EmitCallExternalFunction(program, i);
					externalFunctionFound = true;
					break;
				}
			}

			//if (!externalFunctionFound)
			{
				// Fonction built-in manuelle "print"
				if (IsTokenContentEquals(text_data, tokens, node.FunctionCall.Name, "print"))
				{
					if (node.FunctionCall.FirstArgument == 0)
						return Error(ErrorType::ARGUMENT_COUNT_MISMATCH, node.ProvokingToken);

					const ASTNode& functionFirstArgNode = ast.Nodes[node.FunctionCall.FirstArgument];

					if (functionFirstArgNode.NextNode != 0)	// Pas le droit à plus d'1 argument
						return Error(ErrorType::ARGUMENT_COUNT_MISMATCH, node.ProvokingToken);

					bool buildBuffer = functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId && functionFirstArgNode.Type != ASTNodeType::SYMBOL_CALL && functionFirstArgNode.Type != ASTNodeType::FUNCTION_CALL;

					if (buildBuffer)
						EmitOperation(program, OpCode::START_STRING_BUFFER);

					if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.FunctionCall.FirstArgument, compilation_data, program))
						return error;

					if (buildBuffer)
						EmitOperation(program, OpCode::END_STRING_BUFFER);

					if (functionFirstArgNode.Usage == UsageContext::ADDRESS)
						EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

					// #NOTE : dans cette version, c'est la fonction appelée (comme l'opérateur d'ailleurs) qui a pour responsabilité de gérer la stack. En l'occurence ici on consomme la variable donnée à afficher
					if (functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::VOID_TypeId)
					{
						return Error(ErrorType::INVALID_TYPE, functionFirstArgNode.ProvokingToken);
					}
					else if (functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::FLOAT_TypeId)
					{
						EmitOperation(program, OpCode::DEBUG_PRINT_FLOAT);
						EmitOperation(program, OpCode::POP);
					}
					else if (functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::INT_TypeId)
					{
						EmitOperation(program, OpCode::DEBUG_PRINT_INT);
						EmitOperation(program, OpCode::POP);
					}
					else if (functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::BOOL_TypeId)
					{
						EmitOperation(program, OpCode::DEBUG_PRINT_BOOL);
						EmitOperation(program, OpCode::POP);
					}
					else if (functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::CHAR_TypeId)
					{
						EmitOperation(program, OpCode::DEBUG_PRINT_CHAR);
						EmitOperation(program, OpCode::POP);
					}
					else if (functionFirstArgNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId)
					{
						EmitOperation(program, OpCode::DEBUG_PRINT_STRING);
						EmitOperation(program, OpCode::POP);
					}
					else
					{
						const RuntimeType& rType = analyzis_data.Types[functionFirstArgNode.ContextTypeId];

						if (rType.Category == RuntimeTypeCategory::ENUM)
						{
							EmitOperation(program, OpCode::DEBUG_PRINT_INT);
							EmitOperation(program, OpCode::POP);
						}
						else if (rType.Category == RuntimeTypeCategory::STRUCT)
						{
							// #TODO : print le contenu de la stuct suppose de conserver des metadata de type utilisable à l'exécution
							return Error(ErrorType::ERROR, functionFirstArgNode.ProvokingToken);
						}
						else
						{
							return Error(ErrorType::ERROR, functionFirstArgNode.ProvokingToken);
						}
					}
				}
				else
				{
					// On évalue les arguments de la fonction
					ast_node_index currentArg = node.FunctionCall.FirstArgument;

					while (currentArg != 0)
					{
						const ASTNode& currentArgNode = ast.Nodes[currentArg];

						bool buildBuffer = currentArgNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId && currentArgNode.Type != ASTNodeType::SYMBOL_CALL && currentArgNode.Type != ASTNodeType::FUNCTION_CALL;

						if (buildBuffer)
							EmitOperation(program, OpCode::START_STRING_BUFFER);

						if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, currentArg, compilation_data, program))
							return error;

						if (buildBuffer)
							EmitOperation(program, OpCode::END_STRING_BUFFER);

						// Important : pour un appel de fonction, on veut passer les instances de struct par référence. Il faut donc bien garder l'adresse et ne pas en résoudre sa valeur
						if (currentArgNode.Usage == UsageContext::ADDRESS && analyzis_data.Types[currentArgNode.ContextTypeId].Category != RuntimeTypeCategory::STRUCT)
							EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

						currentArg = currentArgNode.NextNode;
					}

					if(externalFunctionFound)
						EmitCallExternalFunction(program, externalFunctionIndex);
					else
						EmitCallFunction(program, node.FunctionCall.Name);		// Note : on utilise le Name (qui est un token_index) pour indiquer quelle fonction on appelle. On remplace ça par la véritable première instruction lorsqu'on résoud les références de fonctions (voir ResolveFunctionReferences)

					// Si cet appel n'est pas "utilisé" et que la fonction renvoie une valeur, il faut pop cette valeur !
					if (node.FunctionCall.ConsumeResult)
						EmitOperation(program, OpCode::POP);
				}
			}
		}
		break;
		case ASTNodeType::SCOPE:
		{
			ast_node_index currentInstruction = node.Scope.FirstInstruction;

			while( currentInstruction != 0 )
			{
				if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, currentInstruction, compilation_data, program ) )
					return error;

				currentInstruction = ast.Nodes[currentInstruction].NextNode;
			}
		}
		break;
		case ASTNodeType::IF_BLOCK:
		{
			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.IfBlock.Condition, compilation_data, program))
				return error;

			uint32_t jumpOpCursor = EmitJumpIf(program);

			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.IfBlock.IfBlock, compilation_data, program))
				return error;

			uint32_t jumpToAfterElse = EmitJump(program, 0);

			SetJumpTarget(program, jumpOpCursor);

			if (node.IfBlock.ElseBlock != 0)
			{
				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.IfBlock.ElseBlock, compilation_data, program))
					return error;
			}

			SetJumpTarget(program, jumpToAfterElse);
		}
		break;
		case ASTNodeType::WHILE_LOOP:
		{
			uint32_t loopStartInstruction = program.OpCount;

			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.WhileLoop.Condition, compilation_data, program))
				return error;

			uint32_t jumpOpCursor = EmitJumpIf(program);

			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.WhileLoop.Block, compilation_data, program))
				return error;

			EmitJump(program, loopStartInstruction);

			SetJumpTarget(program, jumpOpCursor);
		}
		break;

		case ASTNodeType::CONSTANT_FLOAT:
		{
			EmitPushConstant( program, node.ConstantFloat.Value );
		}
		break;
		case ASTNodeType::CONSTANT_INT:
		{
			EmitPushConstant( program, node.ConstantInt.Value );
		}
		break;
		case ASTNodeType::CONSTANT_BOOL:
		{
			EmitPushConstant( program, node.ConstantBool.Value );
		}
		break;
		case ASTNodeType::CONSTANT_CHARACTER:
		{
			EmitPushConstant(program, node.ConstantChar.Value);
		}
		break;
		case ASTNodeType::CONSTANT_STRING:
		{
			bool found = false;
			for (uint32_t i = 0; i < compilation_data.StringRefCount; ++i)
			{
				const StringRef& sr = compilation_data.StringRefs[i];

				if (sr.Node == cursor)
				{
					EmitPushConstantString(program, sr.ConstantAddress);
					found = true;
					break;
				}
			}

			if (!found)
				return Error(ErrorType::UNREFERENCED_STRING, node.ProvokingToken);
		}
		break;
		case ASTNodeType::UNARY_OPERATOR:
		{
			if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.UnaryOperator.Operand, compilation_data, program ) )
				return error;

			const ASTNode& operandNode = ast.Nodes[node.UnaryOperator.Operand];

			if (operandNode.Usage == UsageContext::ADDRESS)
				EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

			if( IsTokenContentEquals( text_data, tokens, node.UnaryOperator.Operator, "-" ) )
			{
				if( node.ContextTypeId == SemanticAnalyzisData::FLOAT_TypeId )
					EmitOperation( program, OpCode::NEGATE_FLOAT );
				else if (node.ContextTypeId == SemanticAnalyzisData::INT_TypeId)
					EmitOperation( program, OpCode::NEGATE_INT );
				else
					return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
			}
			else if( IsTokenContentEquals( text_data, tokens, node.UnaryOperator.Operator, "!" ) )
			{
				if (node.ContextTypeId == SemanticAnalyzisData::BOOL_TypeId)
					EmitOperation( program, OpCode::NEGATE_BOOL );
				else
					return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
			}
			else
				return Error(ErrorType::UNKNOWN_OPERATOR, node.ProvokingToken);
		}
		break;
		case ASTNodeType::BINARY_OPERATOR:
		{
			// #NOTE : le ContextTypeId du premier opérande fait foi dans les types de tous les opérandes, les casts et vérifications ont été gérés dans l'analyse
			const ASTNode& firstOperandNode = ast.Nodes[node.BinaryOperator.FirstOperand];
			const ASTNode& secondOperandNode = ast.Nodes[node.BinaryOperator.SecondOperand];

			// #HACK ? Dans le cas très particulier de la concaténation de 2 string, il faut les traiter dans l'ordre inverse sinon les opérateurs émis ne le seront pas dans le bon ordre
			// Mais attention : seulement dans le cas d'une concaténation "directe", si on concatène avec un autre opérateur de concaténation, il faut au contraire laisser dans le cas classique
			if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "+") && firstOperandNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId)
			{
				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.BinaryOperator.FirstOperand, compilation_data, program))
					return error;

				if (firstOperandNode.Usage == UsageContext::ADDRESS)
					EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

				// Le premier opérateur de concaténation est nécessairement une string mais on doit éluder certains types comme les opérateurs binaires
				if (firstOperandNode.Type == ASTNodeType::SYMBOL_CALL ||
					firstOperandNode.Type == ASTNodeType::CONSTANT_STRING ||
					firstOperandNode.Type == ASTNodeType::FUNCTION_CALL)
				{
					if (firstOperandNode.NativeTypeId == SemanticAnalyzisData::STRING_TypeId)
						EmitOperation(program, OpCode::APPEND_STRING_TO_STRING_BUFFER);
					else
						return Error(ErrorType::ERROR, node.ProvokingToken);
				}
				
				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.BinaryOperator.SecondOperand, compilation_data, program))
					return error;

				if (secondOperandNode.Usage == UsageContext::ADDRESS)
					EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

				if (secondOperandNode.NativeTypeId == SemanticAnalyzisData::BOOL_TypeId)
					EmitOperation(program, OpCode::APPEND_BOOL_TO_STRING_BUFFER);
				else if (secondOperandNode.NativeTypeId == SemanticAnalyzisData::INT_TypeId)
					EmitOperation(program, OpCode::APPEND_INT_TO_STRING_BUFFER);
				else if (secondOperandNode.NativeTypeId == SemanticAnalyzisData::FLOAT_TypeId)
					EmitOperation(program, OpCode::APPEND_FLOAT_TO_STRING_BUFFER);
				else if (secondOperandNode.NativeTypeId == SemanticAnalyzisData::CHAR_TypeId)
					EmitOperation(program, OpCode::APPEND_CHAR_TO_STRING_BUFFER);
				else if (secondOperandNode.NativeTypeId == SemanticAnalyzisData::STRING_TypeId)
					EmitOperation(program, OpCode::APPEND_STRING_TO_STRING_BUFFER);
				else
					return Error(ErrorType::ERROR, node.ProvokingToken);
			}
			else
			{
				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.BinaryOperator.FirstOperand, compilation_data, program))
					return error;

				if (firstOperandNode.Usage == UsageContext::ADDRESS)
					EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.BinaryOperator.SecondOperand, compilation_data, program))
					return error;

				if (secondOperandNode.Usage == UsageContext::ADDRESS)
					EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

				RuntimeTypeCategory firstTypeCategory = analyzis_data.Types[firstOperandNode.ContextTypeId].Category;

				// Raccourci pour ramener un enum à un int pour simplifier
				if (firstTypeCategory == RuntimeTypeCategory::ENUM)
					firstTypeCategory = RuntimeTypeCategory::INT;

				if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "+"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::ADD_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::ADD_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "-"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::SUBSTRACT_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::SUBSTRACT_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "*"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::MULTIPLY_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::MULTIPLY_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "/"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::DIVIDE_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::DIVIDE_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "<"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::LESS_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::LESS_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "<="))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::LESS_EQUAL_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::LESS_EQUAL_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, ">"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::MORE_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::MORE_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, ">="))
				{
					if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::MORE_EQUAL_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::MORE_EQUAL_FLOAT);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "&&"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::BOOL)
						EmitOperation(program, OpCode::AND);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "||"))
				{
					if (firstTypeCategory == RuntimeTypeCategory::BOOL)
						EmitOperation(program, OpCode::OR);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "=="))
				{
					if (firstTypeCategory == RuntimeTypeCategory::BOOL)
						EmitOperation(program, OpCode::EQUALS_BOOL);
					else if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::EQUALS_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::EQUALS_FLOAT);
					//else if (firstTypeCategory == RuntimeTypeCategory::CHAR)
					//	EmitOperation(program, OpCode::EQUALS_CHAR);
					//else if (firstTypeCategory == RuntimeTypeCategory::STRING)
					//	EmitOperation(program, OpCode::EQUALS_STRING);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
				else if (IsTokenContentEquals(text_data, tokens, node.BinaryOperator.Operator, "!="))
				{
					if (firstTypeCategory == RuntimeTypeCategory::BOOL)
						EmitOperation(program, OpCode::NOT_EQUALS_BOOL);
					else if (firstTypeCategory == RuntimeTypeCategory::INT)
						EmitOperation(program, OpCode::NOT_EQUALS_INT);
					else if (firstTypeCategory == RuntimeTypeCategory::FLOAT)
						EmitOperation(program, OpCode::NOT_EQUALS_FLOAT);
					//else if (firstTypeCategory == RuntimeTypeCategory::CHAR)
					//	EmitOperation(program, OpCode::NOT_EQUALS_CHAR);
					//else if (firstTypeCategory == RuntimeTypeCategory::STRING)
					//	EmitOperation(program, OpCode::NOT_EQUALS_STRING);
					else
						return Error(ErrorType::INVALID_TYPE, node.ProvokingToken);
				}
			}
		}
		break;
		case ASTNodeType::STRUCT:
		{
			return Error::OK;	// Une structure n'a pas besoin d'être compilée
		}
		break;
		case ASTNodeType::RETURN_CALL:
		{
			const ASTNode& valueNode = ast.Nodes[node.ReturnCall.Value];

			bool buildBuffer = valueNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId && valueNode.Type != ASTNodeType::SYMBOL_CALL && valueNode.Type != ASTNodeType::FUNCTION_CALL;

			if (buildBuffer)
				EmitOperation(program, OpCode::START_STRING_BUFFER);

			if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.ReturnCall.Value, compilation_data, program ) )
				return error;

			if (buildBuffer)
				EmitOperation(program, OpCode::END_STRING_BUFFER);

			EmitOperation( program, OpCode::EXIT_FUNCTION );
		}
		break;
		case ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION:
		{
			const RuntimeType& rt = analyzis_data.Types[node.ContextTypeId];
			
			EmitCreateMemory( program, node.VariableDeclarationWOAssignation.LocalAddress, rt.Size);

			if( node.VariableDeclarationWOAssignation.Value != 0 )
			{
				const ASTNode& valueNode = ast.Nodes[node.VariableDeclarationWOAssignation.Value];

				bool buildBuffer = valueNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId && valueNode.Type != ASTNodeType::SYMBOL_CALL && valueNode.Type != ASTNodeType::FUNCTION_CALL;

				if (buildBuffer)
					EmitOperation(program, OpCode::START_STRING_BUFFER);

				if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.VariableDeclarationWOAssignation.Value, compilation_data, program ) )
					return error;

				if (buildBuffer)
					EmitOperation(program, OpCode::END_STRING_BUFFER);

				const RuntimeType& nodeRT = analyzis_data.Types[node.NativeTypeId];

				if (nodeRT.Category == RuntimeTypeCategory::STRUCT)
				{
					EmitPushAddress(program, node.VariableDeclarationWOAssignation.LocalAddress, node.VariableDeclarationWOAssignation.Global);

					EmitCopyMemory(program, nodeRT.Size);
				}
				else
				{
					if (valueNode.Usage == UsageContext::ADDRESS)
						EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

					EmitPushAddress(program, node.VariableDeclarationWOAssignation.LocalAddress, node.VariableDeclarationWOAssignation.Global);

					EmitOperation(program, OpCode::POP_TO_ADDRESS);
				}
			}
		}
		break;
		case ASTNodeType::VARIABLE_ASSIGNATION:
		{
			const ASTNode& valueNode = ast.Nodes[node.VariableAssignation.Value];

			bool buildBuffer = valueNode.ContextTypeId == SemanticAnalyzisData::STRING_TypeId && valueNode.Type != ASTNodeType::SYMBOL_CALL && valueNode.Type != ASTNodeType::FUNCTION_CALL;

			if (buildBuffer)
				EmitOperation(program, OpCode::START_STRING_BUFFER);

			// Attention ! Value avant Name ! Le nom va émettre l'adresse où écrire et le POP_TO_ADDRESS s'attend à avoir l'adresse sur le dessus de la stack
			if( Error error = CompileNode( text_data, tokens, ast, analyzis_data, ex_func_catalog, node.VariableAssignation.Value, compilation_data, program ) )
				return error;

			if (buildBuffer)
				EmitOperation(program, OpCode::END_STRING_BUFFER);

			const RuntimeType& nodeRT = analyzis_data.Types[node.NativeTypeId];

			if (nodeRT.Category == RuntimeTypeCategory::STRUCT)
			{
				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.VariableAssignation.NameX, compilation_data, program))
					return error;

				EmitCopyMemory(program, nodeRT.Size);
			}
			else
			{
				if (valueNode.Usage == UsageContext::ADDRESS)
					EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);

				if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.VariableAssignation.NameX, compilation_data, program))
					return error;

				EmitOperation(program, OpCode::POP_TO_ADDRESS);
			}
		}
		break;
		case ASTNodeType::SYMBOL_CALL:
		{
			bool isStruct = analyzis_data.Types[node.ContextTypeId].Category == RuntimeTypeCategory::STRUCT;

			switch (node.SymbolCall.Context)
			{
				case SymbolContext::GLOBAL:
					EmitPushAddress(program, node.SymbolCall.LocalAddress, true);
					if (node.Usage == UsageContext::ADDRESS_OF_ADDRESS)
						EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);
					break;
				case SymbolContext::SCOPE:
					EmitPushAddress(program, node.SymbolCall.LocalAddress, false);
					if (node.Usage == UsageContext::ADDRESS_OF_ADDRESS)
						EmitOperation(program, OpCode::PUSH_FROM_ADDRESS);
					break;
				case SymbolContext::STRUCT_MEMBER:
					EmitOffsetAddress(program, node.SymbolCall.CompiledOffset);
					break;
				case SymbolContext::ENUM_TYPE_NAME:
					// On ne fait rien, l'analyse sémantique à compilé la valeur à utiliser dans le membre
					break;
				case SymbolContext::ENUM_MEMBER:
					EmitPushConstant(program, node.SymbolCall.CompiledValue);
					break;
			}
		}
		break;
		case ASTNodeType::MEMBER_CALL:
		{
			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.MemberCall.Parent, compilation_data, program))
				return error;

			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, node.MemberCall.Member, compilation_data, program))
				return error;
		}
		break;
		default:
			return Error(ErrorType::ERROR, node.ProvokingToken);
	}

	if (!EmitCastIfNeeded(program, analyzis_data, node.NativeTypeId, node.ContextTypeId))
		return Error(ErrorType::INVALID_CAST, node.ProvokingToken);

	return Error::OK;
}

//----------------------------------------
static Error ResolveFunctionReferences( const TextData& text_data, const TokenCollection& tokens, const CompilationData& compilation_data, CompiledProgram& program )
{
	uint32_t i = 0;

	while( i < program.OpCount )
	{
		OpCode op = program.Ops[i];

		if( op == OpCode::CALL_FUNCTION )
		{
			MemoryBlock mb;
			mb.store[0] = (uint8_t) program.Ops[i + 1];
			mb.store[1] = (uint8_t) program.Ops[i + 2];
			mb.store[2] = (uint8_t) program.Ops[i + 3];
			mb.store[3] = (uint8_t) program.Ops[i + 4];

			uint32_t opIndexUsedAsTokenIndex = mb.index;

			if (Error error = GetFunctionReference(compilation_data, text_data, tokens, opIndexUsedAsTokenIndex, mb.index))
				return error;

			program.Ops[i + 1] = (OpCode) mb.store[0];
			program.Ops[i + 2] = (OpCode) mb.store[1];
			program.Ops[i + 3] = (OpCode) mb.store[2];
			program.Ops[i + 4] = (OpCode) mb.store[3];
		}

		i += GetOpCodeSize(op);
	}

	return Error::OK;
}

//----------------------------------------
Error CompileAST( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, const ExternalFunctionCatalog& ex_func_catalog, CompiledProgram& out_program, CompilationData& out_compilation_data )
{
	// On construit le buffer de mémoire constante (utilisée uniquement par les string pour l'instant
	for (uint32_t i = 0; i < ast.NodeCount; ++i)
	{
		const ASTNode& node = ast.Nodes[i];

		if (node.Type == ASTNodeType::CONSTANT_STRING)
		{
			TextData stringData;

			if (!GetTokenValue(text_data, tokens, node.ConstantString.StrValue, stringData))
				return Error(ErrorType::MISSING_STRING_DATA, node.ConstantString.StrValue);

			StringRef& sr = out_compilation_data.StringRefs[out_compilation_data.StringRefCount];
			++out_compilation_data.StringRefCount;

			sr.Node = i;
			sr.ConstantAddress = out_program.ConstantTop;

			MemoryBlock& header = out_program.Constants[out_program.ConstantTop];
			++out_program.ConstantTop;

			header.index = 0;

			uint32_t read = 0;
			bool escape = false;		// La chaîne brute du code possède les guillemets et les caractères échappés qu'il faut traiter ici

			while (read < stringData.Length)
			{
				MemoryBlock& mb = out_program.Constants[out_program.ConstantTop];
				++out_program.ConstantTop;

				uint32_t subRead = 0;
				uint32_t subWrite = 0;

				while (subWrite < 4 && read + subRead < stringData.Length)
				{
					char cToWrite = stringData.Buffer[read + subRead];

					if (escape)
					{
						escape = false;

						if (cToWrite == 'n')
							cToWrite = '\n';
						else if (cToWrite == 't')
							cToWrite = '\t';
					}
					else if (cToWrite == '\\')
					{
						escape = true;
						cToWrite = '\0';
					}
					else if (cToWrite == '"')
					{
						cToWrite = '\0';
					}

					if(cToWrite != '\0')
					{
						mb.c8_4[subWrite] = cToWrite;
						++header.index;
						++subWrite;
						

						escape = false;
					}

					++subRead;
				}

				// A cause des échappements, on peut se retrouver avec un memory block alloué pour rien. On le libère s'il se trouve qu'on n'a rien écrit
				if (subWrite == 0)
				{
					--out_program.ConstantTop;
				}
				else
				{
					while (subWrite < 4)
					{
						mb.c8_4[subWrite] = '\0';

						++subWrite;
					}
				}

				read += subRead;
			}
		}
	}

	// On compile les déclaration/assignation des variables globales
	uint32_t cursor = 0;

	do
	{
		if (ast.Nodes[cursor].Type == ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION)
		{
			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, cursor, out_compilation_data, out_program))
				return error;
		}

		cursor = ast.Nodes[cursor].NextNode;
	} while (cursor != 0);

	// Reserving Entry Point
	uint32_t entryPointOp = out_program.OpCount;
	EmitCallFunction( out_program, 0 );
	EmitOperation( out_program, OpCode::EXIT );

	// On compile le reste des instructions
	cursor = 0;

	do
	{
		if (ast.Nodes[cursor].Type != ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION)
		{
			if (Error error = CompileNode(text_data, tokens, ast, analyzis_data, ex_func_catalog, cursor, out_compilation_data, out_program))
				return error;
		}

		cursor = ast.Nodes[cursor].NextNode;
	} while( cursor != 0 );

	// Setting Entry Point
	ReplaceCallFunction( out_program, out_compilation_data.MainNameToken, entryPointOp);		// On donne le token représentant le nom de la fonction main à l'appel de la fonction initiale. Ce sera résolu ensuite dans ResolveFunctionReferences juste après

	if (Error error = ResolveFunctionReferences(text_data, tokens, out_compilation_data, out_program))
		return error;

	return Error::OK;
}