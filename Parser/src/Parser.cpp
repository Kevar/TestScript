#include "Parser.h"

//----------------------------------------
static const char* l_astNames[] =
{
	"STRUCT",
	"ENUM",
	"ENUM_VALUE",
	"BRANCH",
	"FUNCTION",
	"ARGUMENT",
	"SCOPE",
	"VARIABLE_DECLARATION_WO_ASSIGNATION",
	"VARIABLE_ASSIGNATION",
	"FUNCTION_CALL",
	"IF_BLOCK",
	"WHILE_LOOP",
	"CONSTANT_FLOAT",
	"CONSTANT_INT",
	"CONSTANT_BOOL",
	"CONSTANT_CHARACTER",
	"CONSTANT_STRING",
	"SYMBOL_CALL",
	"UNARY_OPERATOR",
	"BINARY_OPERATOR",
	"TERNARY_OPERATOR",
	"MEMBER_CALL",
	"RETURN_CALL",

	"UNKNOWN"
};

//----------------------------------------
static uint32_t l_astNamesCount = sizeof(l_astNames) / sizeof(l_astNames[0]);

#pragma region Token Helpers

//----------------------------------------
static const Token& PeekNextToken( const TokenCollection& tokens, uint32_t token_cursor )
{
	++token_cursor;

	while( token_cursor < tokens.TokenCount && ( tokens.Tokens[token_cursor].Type == TokenType::WHITESPACE || tokens.Tokens[token_cursor].Type == TokenType::COMMENT ) )
		++token_cursor;

	return tokens.Tokens[token_cursor];	// Note : si on est à token_cursor == tokens.TokenCount, on va renvoyer le token END, ce qui marche quand même
}

//----------------------------------------
static const Token& GetNextToken( const TokenCollection& tokens, token_index& token_cursor )
{
	++token_cursor;

	while( token_cursor < tokens.TokenCount && ( tokens.Tokens[token_cursor].Type == TokenType::WHITESPACE || tokens.Tokens[token_cursor].Type == TokenType::COMMENT ) )
		++token_cursor;

	return tokens.Tokens[token_cursor];	// Note : si on est à token_cursor == tokens.TokenCount, on va renvoyer le token END, ce qui marche quand même
}

//----------------------------------------
static bool IsCurrentTokenOfType( const TokenCollection& tokens, uint32_t token_cursor, std::initializer_list<TokenType> types )
{
	TokenType type = tokens.Tokens[token_cursor].Type;

	for( const TokenType& t : types )
	{
		if( type == t )
			return true;
	}

	return false;
}

//----------------------------------------
static bool IsNextTokenOfType( const TokenCollection& tokens, token_index& token_cursor, TokenType& out_type, std::initializer_list<TokenType> types, bool consume )
{
	uint32_t workingCursor = token_cursor;
	++workingCursor;

	while( workingCursor < tokens.TokenCount && ( tokens.Tokens[workingCursor].Type == TokenType::WHITESPACE || tokens.Tokens[workingCursor].Type == TokenType::COMMENT ) )
		++workingCursor;

	out_type = tokens.Tokens[workingCursor].Type;

	if( consume )
		token_cursor = workingCursor;

	for( const TokenType& t : types )
	{
		if( out_type == t )
			return true;
	}

	return false;
}

#pragma endregion

//----------------------------------------
static ASTNode& AddASTNode( ASTNodeCollection& ast, ASTNodeType type, token_index provoking_token, ast_node_index& out_created_node_index )
{
	ASTNode& node = ast.Nodes[ast.NodeCount];
	node.Type = type;
	node.ProvokingToken = provoking_token;
	node.NextNode = 0;		// Pointer vers 0 indique qu'il n'y a pas de next node (0 étant le premier node, il ne peut jamais être le NextNode d'un autre)
	out_created_node_index = ast.NodeCount;
	++ast.NodeCount;

	return node;
}

// Forward declarations
static Error Parse_Scope( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );
static Error Parse_ConditionBlock( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );
static Error Parse_WhileLoop( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );
static Error Parse_Expression( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index, int right_binding_power );
static Error Parse_ArgumentCallList( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );

//----------------------------------------
static bool IsNumeric( char c )
{
	return c >= '0' && c <= '9';
}

//----------------------------------------
static Error ParseCharacter( const TextData& text_data, uint32_t cursor, char& out_value )
{
	// #NOTE : on a déjà testé que le curseur ne dépassait par le text data dans la tokenization, on ne revérifie donc pas ça ici
	++cursor;	// On consomme la quote ouvrante

	char current = text_data.Buffer[cursor];

	if (current == '\\')
	{
		// Cas d'échappement
		++cursor;

		current = text_data.Buffer[cursor];

		// Pour les 2 premiers : oui current et out_value ont la même valeur, mais c'est fortuit : il suffit de voir les cas suivants pour comprendre la logique
		if (current == '\'')
		{
			out_value = '\'';
			return Error::OK;
		}
		else if (current == '\\')
		{
			out_value = '\\';
			return Error::OK;
		}
		else if (current == 'n')
		{
			out_value = '\n';
			return Error::OK;
		}
		else if (current == 'r')
		{
			out_value = '\r';
			return Error::OK;
		}
		else if (current == 't')
		{
			out_value = '\t';
			return Error::OK;
		}
		else
		{
			return Error(ErrorType::INVALID_ESCAPED_CHARACTER, cursor);
		}
	}
	else
	{
		// Cas normal
		out_value = current;
		return Error::OK;
	}
}

//----------------------------------------
// Note : pour les nombres négatifs on passe par une expression avec opérateur unaire '-'
static Error ParseFloat( const TextData& text_data, uint32_t cursor, float& out_value, bool& castable_as_int )
{
	bool pointFound = false;
	out_value = 0.0f;
	castable_as_int = true;

	float decimalMultiplier = 1.0f;

	while( cursor < text_data.Length )
	{
		char c = text_data.Buffer[cursor];

		if( c == '.' )
		{
			if( pointFound )
				return Error(ErrorType::INVALID_NUMBER_FORMAT_MULTIPLE_POINTS, cursor);

			pointFound = true;
			castable_as_int = false;
		}
		else if( !IsNumeric( c ) )
		{
			if( pointFound && text_data.Buffer[cursor - 1] == '.' )
			{
				// On n'a pas de chiffre juste après le point : erreur
				return Error( ErrorType::INVALID_NUMBER_FORMAT_NO_DIGIT_AFTER_POINT, cursor );
			}
			else
			{
				out_value *= decimalMultiplier;
				return Error::OK;
			}
		}
		else
		{
			out_value *= 10.0f;

			if( pointFound )
				decimalMultiplier *= 0.1f;

			switch( c )
			{
				case '0': break;
				case '1': out_value += 1; break;
				case '2': out_value += 2; break;
				case '3': out_value += 3; break;
				case '4': out_value += 4; break;
				case '5': out_value += 5; break;
				case '6': out_value += 6; break;
				case '7': out_value += 7; break;
				case '8': out_value += 8; break;
				case '9': out_value += 9; break;
			}
		}

		++cursor;
	}

	out_value *= decimalMultiplier;
	return Error::OK;
}

//----------------------------------------
static Error Parse_Var( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index, bool global )
{
	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) || !IsTokenContentEquals( text_data, tokens, token_cursor, "var" ) )
		return Error( ErrorType::MISSING_VAR_KEYWORD, token_cursor);

	TokenType tokenType;

	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL }, true ) )
		return Error( ErrorType::MISSING_VAR_NAME, token_cursor );

	token_index varNameToken = token_cursor;

	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::COLON }, true ) )
		return Error( ErrorType::MISSING_VAR_COLON_SEPARATOR, token_cursor );

	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL }, true ) )
		return Error( ErrorType::MISSING_VAR_TYPE, token_cursor );

	token_index varTypeToken = token_cursor;

	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::SEMI_COLON, TokenType::EQUAL }, true ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_DECLARATION_WO_ASSIGNATION, varNameToken, out_created_node_index );

	node.VariableDeclarationWOAssignation.Name = varNameToken;
	node.VariableDeclarationWOAssignation.Type = varTypeToken;
	node.VariableDeclarationWOAssignation.Global = global;

	GetNextToken( tokens, token_cursor );	// On consomme le '=' ou le ';'

	if( tokenType == TokenType::EQUAL )
	{
		if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, node.VariableDeclarationWOAssignation.Value, 0 ) )
			return error;

		if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
		{
			GetNextToken( tokens, token_cursor );
			return Error::OK;
		}
		else
			return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
	}

	return Error::OK;
	//if( tokenType == TokenType::SEMI_COLON )
	//{
	//	ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_DECLARATION, out_created_node_index );

	//	node.VariableDeclaration.Name = varNameToken;
	//	node.VariableDeclaration.Type = varTypeToken;

	//	GetNextToken( tokens, token_cursor );
	//	return Error::OK;
	//}
	//else // if( tokenType == TokenType::EQUAL )
	//{
	//	ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_DECLARATION_ASSIGNATION, out_created_node_index );

	//	node.VariableDeclarationAssignation.Name = varNameToken;
	//	node.VariableDeclarationAssignation.Type = varTypeToken;

	//	GetNextToken( tokens, token_cursor );	// On consomme le '='

	//	if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, node.VariableDeclarationAssignation.Value, 0 ) )
	//		return error;

	//	if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
	//	{
	//		GetNextToken( tokens, token_cursor );
	//		return Error::OK;
	//	}
	//	else
	//		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
	//}
}

//----------------------------------------
static Error Parse_Instruction( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	// #NOTE : la valeur par défaut de out_created_node_index est déjà bonne pour indiquer "pas de lien". On peut donc sortir de cette fonction sans l'affecter si besoin
	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType == TokenType::OPENED_CURLY_BRACE )
	{
		return Parse_Scope( text_data, tokens, token_cursor, ast, out_created_node_index );
	}
	else if( tokenType == TokenType::LABEL )
	{
		if( IsTokenContentEquals( text_data, tokens, token_cursor, "if" ) )
		{
			// if block
			return Parse_ConditionBlock( text_data, tokens, token_cursor, ast, out_created_node_index );
		}
		else if( IsTokenContentEquals( text_data, tokens, token_cursor, "while" ) )
		{
			return Parse_WhileLoop( text_data, tokens, token_cursor, ast, out_created_node_index );
		}
		else if( IsTokenContentEquals( text_data, tokens, token_cursor, "return" ) )
		{
			ASTNode& node = AddASTNode( ast, ASTNodeType::RETURN_CALL, token_cursor, out_created_node_index );
			node.ReturnCall.Value = 0;

			GetNextToken( tokens, token_cursor );	// On consomme le return

			// Simple instruction return sans paramètre
			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
			{
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}

			if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, node.ReturnCall.Value, 0 ) )
				return error;

			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
			{
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}
			else
				return Error(ErrorType::UNEXPECTED_TOKEN, token_cursor);
		}
		else if( IsTokenContentEquals( text_data, tokens, token_cursor, "var" ) )
		{
			return Parse_Var( text_data, tokens, token_cursor, ast, out_created_node_index, false );
		}
		else
		{
			if (Error error = Parse_Expression(text_data, tokens, token_cursor, ast, out_created_node_index, 0))
				return error;

			ASTNode& createdNode = ast.Nodes[out_created_node_index];

			if (createdNode.Type != ASTNodeType::FUNCTION_CALL && createdNode.Type != ASTNodeType::VARIABLE_ASSIGNATION)
				return Error(ErrorType::INVALID_INSTRUCTION, createdNode.ProvokingToken);

			if (IsCurrentTokenOfType(tokens, token_cursor, { TokenType::SEMI_COLON }))
			{
				GetNextToken(tokens, token_cursor);
				return Error::OK;
			}
			else
				return Error(ErrorType::UNEXPECTED_TOKEN, token_cursor);

			//token_index nameToken = token_cursor;

			//if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::EQUAL, TokenType::OPENED_PARENTHESIS }, false ) )
			//	return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

			//if( tokenType == TokenType::EQUAL )
			//{
			//	GetNextToken( tokens, token_cursor );
			//	ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_ASSIGNATION, nameToken, out_created_node_index );

			//	node.VariableAssignation.Name = nameToken;

			//	GetNextToken( tokens, token_cursor );	// On consomme le '='

			//	if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, node.VariableAssignation.Value, 0 ) )
			//		return error;

			//	if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
			//	{
			//		GetNextToken( tokens, token_cursor );
			//		return Error::OK;
			//	}
			//	else
			//		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

			//}
			//else // if( tokenType == TokenType::OPENED_PARENTHESIS )
			//{
			//	ASTNode& node = AddASTNode( ast, ASTNodeType::FUNCTION_CALL, nameToken, out_created_node_index );
			//	node.FunctionCall.Name = nameToken;
			//	node.FunctionCall.FirstArgument = 0;	// Par défaut : sans argument
			//	node.FunctionCall.ConsumeResult = true;	// On est directement à la racine d'une instruction, il faut donc consommer le résultat car aucune valeur ne va la récupérer

			//	GetNextToken( tokens, token_cursor );	// On consomme le nom de la fonction

			//	if( Error error = Parse_ArgumentCallList( text_data, tokens, token_cursor, ast, node.FunctionCall.FirstArgument ) )
			//		return error;

			//	if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
			//	{
			//		GetNextToken( tokens, token_cursor );
			//		return Error::OK;
			//	}
			//	else
			//		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
			//}
		}
	}
	else
	{
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
	}
}

#pragma region Arguments

	//----------------------------------------
static Error Parse_ArgumentDeclaration( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::LABEL )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& node = AddASTNode( ast, ASTNodeType::ARGUMENT, token_cursor, out_created_node_index );
	node.Argument.Name = token_cursor;

	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::COLON }, true ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL }, true ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	node.Argument.Type = token_cursor;

	GetNextToken( tokens, token_cursor );	// On consomme le label du type

	return Error::OK;
}

//----------------------------------------
static Error Parse_ArgumentDeclarationList( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::OPENED_PARENTHESIS )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ast_node_index* linkChain = &out_created_node_index;

	GetNextToken( tokens, token_cursor );

	while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_PARENTHESIS } ) )
	{
		if( Error error = Parse_ArgumentDeclaration( text_data, tokens, token_cursor, ast, *linkChain ) )
			return error;

		linkChain = &ast.Nodes[*linkChain].NextNode;

		if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COMMA } ) )
			GetNextToken( tokens, token_cursor );
	}

	GetNextToken( tokens, token_cursor );	// On consomme la parenthèse fermante

	return Error::OK;
}

//----------------------------------------
static Error Parse_ArgumentCallList( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::OPENED_PARENTHESIS )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ast_node_index* linkChain = &out_created_node_index;

	GetNextToken( tokens, token_cursor );

	while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_PARENTHESIS } ) )
	{
		if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, *linkChain, 0 ) )
			return error;

		linkChain = &ast.Nodes[*linkChain].NextNode;

		if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COMMA } ) )
			GetNextToken( tokens, token_cursor );
	}

	GetNextToken( tokens, token_cursor );	// On consomme la parenthèse fermante

	return Error::OK;
}

#pragma endregion

#pragma region Blocks

	//----------------------------------------
static Error Parse_Scope( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::OPENED_CURLY_BRACE )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& node = AddASTNode( ast, ASTNodeType::SCOPE, token_cursor, out_created_node_index );
	node.Scope.FirstInstruction = 0;

	ast_node_index* linkChain = &node.Scope.FirstInstruction;

	GetNextToken( tokens, token_cursor );

	while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_CURLY_BRACE } ) )
	{
		if( Error error = Parse_Instruction( text_data, tokens, token_cursor, ast, *linkChain ) )
			return error;

		linkChain = &ast.Nodes[*linkChain].NextNode;
	}

	GetNextToken( tokens, token_cursor );	// On consomme l'accolade fermante

	return Error::OK;
}

//----------------------------------------
static Error Parse_Struct( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	token_index structToken = token_cursor;

	GetNextToken( tokens, token_cursor );	// On consomme le "struct"

	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::LABEL )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& structNode = AddASTNode( ast, ASTNodeType::STRUCT, structToken, out_created_node_index );
	structNode.Struct.Name = token_cursor;
	structNode.Struct.FirstMember = 0;

	GetNextToken( tokens, token_cursor );

	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::OPENED_CURLY_BRACE } ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ast_node_index* linkChain = &structNode.Struct.FirstMember;

	GetNextToken( tokens, token_cursor );

	while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_CURLY_BRACE } ) )
	{
		if( Error error = Parse_Var( text_data, tokens, token_cursor, ast, *linkChain, false ) )
			return error;

		linkChain = &ast.Nodes[*linkChain].NextNode;
	}

	GetNextToken( tokens, token_cursor );	// On consomme l'accolade fermante

	return Error::OK;
}

//----------------------------------------
static Error Parse_Enum( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	token_index enumToken = token_cursor;

	GetNextToken( tokens, token_cursor );	// On consomme le "enum"

	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::LABEL )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& enumNode = AddASTNode( ast, ASTNodeType::ENUM, enumToken, out_created_node_index );
	enumNode.Enum.Name = token_cursor;
	enumNode.Enum.FirstValue = 0;

	GetNextToken( tokens, token_cursor );

	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::OPENED_CURLY_BRACE } ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ast_node_index* linkChain = &enumNode.Enum.FirstValue;

	GetNextToken( tokens, token_cursor );

	while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_CURLY_BRACE } ) )
	{
		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) )
			return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

		ASTNode& enumValueNode = AddASTNode( ast, ASTNodeType::ENUM_VALUE, token_cursor, *linkChain );
		enumValueNode.EnumValue.Value = token_cursor;
		linkChain = &enumValueNode.NextNode;

		GetNextToken( tokens, token_cursor );

		if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COMMA } ) )
			GetNextToken( tokens, token_cursor );
	}

	GetNextToken( tokens, token_cursor );	// On consomme l'accolade fermante

	return Error::OK;
}

//----------------------------------------
static Error Parse_Func( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	token_index funcToken = token_cursor;

	GetNextToken( tokens, token_cursor );	// On consomme le "func" (ou le "branch" car on mutualise le code de parsing entre les deux)

	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	if( tokenType != TokenType::LABEL )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& functionNode = AddASTNode( ast, ASTNodeType::FUNCTION, token_cursor, out_created_node_index );
	functionNode.Function.Name = token_cursor;
	functionNode.Function.FirstArgument = 0;

	GetNextToken( tokens, token_cursor );

	if( Error error = Parse_ArgumentDeclarationList( text_data, tokens, token_cursor, ast, functionNode.Function.FirstArgument ) )
		return error;

	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COLON } ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	GetNextToken( tokens, token_cursor );

	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	functionNode.Function.ReturnType = token_cursor;

	GetNextToken( tokens, token_cursor );

	return Parse_Scope( text_data, tokens, token_cursor, ast, functionNode.Function.Body );
}

//----------------------------------------
static Error Parse_Branch( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	ASTNode& branchNode = AddASTNode( ast, ASTNodeType::BRANCH, token_cursor, out_created_node_index );

	return Parse_Func( text_data, tokens, token_cursor, ast, branchNode.Branch.Function );
}

//----------------------------------------
static Error Parse_ConditionBlock( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) || !IsTokenContentEquals( text_data, tokens, token_cursor, "if" ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& ifNode = AddASTNode( ast, ASTNodeType::IF_BLOCK, token_cursor, out_created_node_index );

	GetNextToken( tokens, token_cursor );

	if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, ifNode.IfBlock.Condition, 0 ) )
		return error;

	if( Error error = Parse_Scope( text_data, tokens, token_cursor, ast, ifNode.IfBlock.IfBlock ) )
		return error;

	if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) && IsTokenContentEquals( text_data, tokens, token_cursor, "else" ) )
	{
		TokenType tokenType;
		if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL, TokenType::OPENED_CURLY_BRACE }, true ) )
			return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

		if( tokenType == TokenType::LABEL && IsTokenContentEquals( text_data, tokens, token_cursor, "if" ) )
			return Parse_ConditionBlock( text_data, tokens, token_cursor, ast, ifNode.IfBlock.ElseBlock );
		else
			return Parse_Scope( text_data, tokens, token_cursor, ast, ifNode.IfBlock.ElseBlock );
	}
	else
	{
		ifNode.IfBlock.ElseBlock = 0;

		return Error::OK;
	}
}

//----------------------------------------
static Error Parse_WhileLoop( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) || !IsTokenContentEquals( text_data, tokens, token_cursor, "while" ) )
		return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

	ASTNode& whileNode = AddASTNode( ast, ASTNodeType::WHILE_LOOP, token_cursor, out_created_node_index );

	GetNextToken( tokens, token_cursor );

	if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, whileNode.WhileLoop.Condition, 0 ) )
		return error;

	if( Error error = Parse_Scope( text_data, tokens, token_cursor, ast, whileNode.WhileLoop.Block ) )
		return error;

	return Error::OK;
}

#pragma endregion

#pragma region Expression

//----------------------------------------
// Pratt Parsing
//
// https://www.youtube.com/watch?v=Nlqv6NtBXcA&t=1171s
// https://crockford.com/javascript/tdop/tdop.html
// https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
//
// Règles :
//	* Le token courant quand on appelle les fonctions est déjà le token à tester/traiter
//  * Une opération (NUD, LED ou EXPR) consomme tous les tokens qu'elle traite : en sortie on est donc déjà sur le token suivant
//	* NUD ne sera appelé que pour le token de début d'expression, soit :
//		> Un opérateur unaire préfixe (comme !) qui crée son ASTNode, parse la suite comme une expression et retourne son node
//		> Un opérande qui crée son ASTNode et retourne son node
//		> Une sous-expression explicite (parenthèses) qui consomme la parenthèse, parse la suite comme une expression, consomme la parenthèse fermante et retourne le node de la sous expression
//	* LED est appelé de manière itérative sur tous les tokens suivants de l'expression (pas le premier)

//----------------------------------------
struct BindingPower
{
	TokenType Token;
	int LeftBindingPower;
	int RightBindingPower;
};

static BindingPower l_bindingPowers[] =
{
	{ TokenType::WHITESPACE, 0, 0 },

	{ TokenType::OPENED_CURLY_BRACE, 0, 0 },
	{ TokenType::CLOSED_CURLY_BRACE, 0, 0 },
	{ TokenType::OPENED_PARENTHESIS, 0, 0 },
	{ TokenType::CLOSED_PARENTHESIS, 0, 0 },
	{ TokenType::OPENED_SQUARE_BRACKET, 0, 0 },
	{ TokenType::CLOSED_SQUARE_BRACKET, 0, 0 },

	{ TokenType::COLON, 0, 0 },
	{ TokenType::SEMI_COLON, 0, 0 },
	{ TokenType::COMMA, 0, 0 },

	{ TokenType::EQUAL, 10, 11 },

	{ TokenType::PLUS, 50, 51 },
	{ TokenType::MINUS, 50, 51 },
	{ TokenType::STAR, 60, 61 },
	{ TokenType::SLASH, 60, 61 },

	{ TokenType::DOT, 80, 81 },

	{ TokenType::INFERIOR, 40, 41 },
	{ TokenType::SUPERIOR, 40, 41 },

	{ TokenType::INFERIOR_EQUAL, 40, 41 },
	{ TokenType::SUPERIOR_EQUAL, 40, 41 },

	{ TokenType::COMPARISON, 40, 41 },
	{ TokenType::COMPARISON_NOT, 40, 41 },

	{ TokenType::AND, 30, 31 },
	{ TokenType::OR, 30, 31 },

	{ TokenType::BIT_AND, 30, 31 },
	{ TokenType::BIT_OR, 30, 31 },

	{ TokenType::EXCLAMATION_MARK, 70, 71 },
	{ TokenType::QUESTION_MARK, 20, 21 },

	{ TokenType::NUMBER, 0, 0 },
	{ TokenType::LABEL, 0, 0 },
	{ TokenType::STRING, 0, 0 },
	{ TokenType::COMMENT, 0, 0 },

	{ TokenType::UNDEFINED, 0, 0 },

	{ TokenType::END, 0, 0 }
};

//----------------------------------------
static Error Parse_NUD( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
{
	TokenType currentTokenType = tokens.Tokens[token_cursor].Type;

	switch( currentTokenType )
	{
		case TokenType::MINUS:
		case TokenType::PLUS:
		case TokenType::EXCLAMATION_MARK:
		{
			ASTNode& node = AddASTNode( ast, ASTNodeType::UNARY_OPERATOR, token_cursor, out_created_node_index );
			node.UnaryOperator.Operator = token_cursor;
			GetNextToken( tokens, token_cursor );
			return Parse_Expression( text_data, tokens, token_cursor, ast, node.UnaryOperator.Operand, l_bindingPowers[(uint32_t) currentTokenType].RightBindingPower );
		}
		break;
		case TokenType::OPENED_PARENTHESIS:
		{
			GetNextToken( tokens, token_cursor );
			if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, out_created_node_index, /*l_bindingPowers[(uint32_t) currentTokenType].RightBindingPower*/0 ) )
				return error;
			if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_PARENTHESIS } ) )
				return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
			GetNextToken( tokens, token_cursor );

			return Error::OK;
		}
		break;
		case TokenType::LABEL:
		{
			// 3 cas : FUNCTION_CALL ou SYMBOL_CALL ou CONSTANT_BOOL
			if( PeekNextToken( tokens, token_cursor ).Type == TokenType::OPENED_PARENTHESIS )
			{
				ASTNode& node = AddASTNode( ast, ASTNodeType::FUNCTION_CALL, token_cursor, out_created_node_index );
				node.FunctionCall.Name = token_cursor;
				node.FunctionCall.FirstArgument = 0;	// Par défaut : sans argument
				node.FunctionCall.ConsumeResult = false;	// On est dans une expression, il ne faut donc pas consommer le résultat car la valeur va être utilisée

				GetNextToken( tokens, token_cursor );	// On consomme le nom de la fonction

				return Parse_ArgumentCallList( text_data, tokens, token_cursor, ast, node.FunctionCall.FirstArgument );
			}
			else if( IsTokenContentEquals( text_data, tokens, token_cursor, "true" ) )
			{
				ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_BOOL, token_cursor, out_created_node_index );
				node.ConstantBool.StrValue = token_cursor;
				node.ConstantBool.Value = true;
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}
			else if( IsTokenContentEquals( text_data, tokens, token_cursor, "false" ) )
			{
				ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_BOOL, token_cursor, out_created_node_index );
				node.ConstantBool.StrValue = token_cursor;
				node.ConstantBool.Value = false;
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}
			else
			{
				ASTNode& node = AddASTNode( ast, ASTNodeType::SYMBOL_CALL, token_cursor, out_created_node_index );
				node.SymbolCall.Name = token_cursor;
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}
		}
		break;
		case TokenType::CHARACTER:
		{
			ASTNode& node = AddASTNode(ast, ASTNodeType::CONSTANT_CHARACTER, token_cursor, out_created_node_index);
			node.ConstantChar.StrValue = token_cursor;

			if (Error error = ParseCharacter(text_data, tokens.Tokens[token_cursor].TextPosition, node.ConstantChar.Value))
				return error;

			GetNextToken(tokens, token_cursor);
			return Error::OK;
		}
		break;
		case TokenType::STRING:
		{
			ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_STRING, token_cursor, out_created_node_index );
			node.ConstantString.StrValue = token_cursor;
			GetNextToken( tokens, token_cursor );
			return Error::OK;
		}
		break;
		case TokenType::NUMBER:
		{
			bool castableAsInt;
			float numberValue;
			if( Error error = ParseFloat( text_data, tokens.Tokens[token_cursor].TextPosition, numberValue, castableAsInt ) )
				return error;	// Normalement il est impossible d'arriver là car la tokenization a déjà traité les cas problématiques

			if( castableAsInt )
			{
				ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_INT, token_cursor, out_created_node_index );
				node.ConstantInt.StrValue = token_cursor;
				node.ConstantInt.Value = (int32_t) numberValue;
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}
			else
			{
				ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_FLOAT, token_cursor, out_created_node_index );
				node.ConstantFloat.StrValue = token_cursor;
				node.ConstantFloat.Value = numberValue;
				GetNextToken( tokens, token_cursor );
				return Error::OK;
			}
		}
		break;
		default:
			return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
			break;
	}

	return Error::OK;
}

//----------------------------------------
static Error Parse_LED( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index left_node, ast_node_index& out_created_node_index )
{
	TokenType currentTokenType = tokens.Tokens[token_cursor].Type;

	switch( currentTokenType )
	{
		case TokenType::PLUS:
		case TokenType::MINUS:
		case TokenType::STAR:
		case TokenType::SLASH:
		case TokenType::INFERIOR:
		case TokenType::SUPERIOR:
		case TokenType::INFERIOR_EQUAL:
		case TokenType::SUPERIOR_EQUAL:
		case TokenType::COMPARISON:
		case TokenType::COMPARISON_NOT:
		case TokenType::AND:
		case TokenType::OR:
		case TokenType::BIT_AND:
		case TokenType::BIT_OR:
		{
			ASTNode& node = AddASTNode( ast, ASTNodeType::BINARY_OPERATOR, token_cursor, out_created_node_index );
			node.BinaryOperator.Operator = token_cursor;
			node.BinaryOperator.FirstOperand = left_node;
			GetNextToken( tokens, token_cursor );
			return Parse_Expression( text_data, tokens, token_cursor, ast, node.BinaryOperator.SecondOperand, l_bindingPowers[(uint32_t) currentTokenType].LeftBindingPower );
		}
		break;
		case TokenType::QUESTION_MARK:
		{
			ASTNode& node = AddASTNode( ast, ASTNodeType::TERNARY_OPERATOR, token_cursor, out_created_node_index );
			node.TernaryOperator.Operator = token_cursor;
			node.TernaryOperator.FirstOperand = left_node;
			GetNextToken( tokens, token_cursor );
			if( Error error = Parse_Expression( text_data, tokens, token_cursor, ast, node.TernaryOperator.SecondOperand, l_bindingPowers[(uint32_t) currentTokenType].LeftBindingPower ) )
				return error;

			if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COLON } ) )
				return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );

			GetNextToken( tokens, token_cursor );
			return Parse_Expression( text_data, tokens, token_cursor, ast, node.TernaryOperator.ThirdOperand, l_bindingPowers[(uint32_t) TokenType::COLON].LeftBindingPower );
		}
		break;
		case TokenType::DOT:		// Très similaire à Binary Operator, à fusionner ? Ou à l'inverse être plus spécifique sur tous les opérateurs binaires dans l'AST et avoir AST_ADD, AST_SUBSTRACT, etc. ?
		{
			ASTNode& node = AddASTNode( ast, ASTNodeType::MEMBER_CALL, token_cursor, out_created_node_index );
			node.MemberCall.Parent = left_node;
			GetNextToken( tokens, token_cursor );
			return Parse_Expression( text_data, tokens, token_cursor, ast, node.MemberCall.Member, l_bindingPowers[(uint32_t)currentTokenType].LeftBindingPower);
		}

		// Tentative de correction de l'erreur qui empêche de parser une chaîne de member call à gauche
		case TokenType::EQUAL:
		{
			ASTNode& node = AddASTNode(ast, ASTNodeType::VARIABLE_ASSIGNATION, token_cursor, out_created_node_index);
			node.VariableAssignation.NameX = left_node;
			GetNextToken(tokens, token_cursor);
			return Parse_Expression(text_data, tokens, token_cursor, ast, node.VariableAssignation.Value, l_bindingPowers[(uint32_t)currentTokenType].LeftBindingPower);
		}

		default:
			return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
			break;
	}
}

//----------------------------------------
static Error Parse_Expression( const TextData& text_data, const TokenCollection& tokens, token_index& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index, int right_binding_power )
{
	if( Error error = Parse_NUD( text_data, tokens, token_cursor, ast, out_created_node_index ) )
		return error;

	TokenType currentTokenType = tokens.Tokens[token_cursor].Type;

	while( l_bindingPowers[(uint32_t) currentTokenType].LeftBindingPower > right_binding_power )
	{
		if( Error error = Parse_LED( text_data, tokens, token_cursor, ast, out_created_node_index, out_created_node_index ) )	// 1er out_created_node_index : copié dans les arguments, pas référencé
			return error;

		currentTokenType = tokens.Tokens[token_cursor].Type;
	}

	return Error::OK;
}

#pragma endregion

//----------------------------------------
Error ParseTokens( const TextData& text_data, const TokenCollection& tokens, ASTNodeCollection& out_ast )
{
	uint32_t token_cursor = 0;

	ast_node_index sentinelNode;
	ast_node_index* createdNode = &sentinelNode;

	while( token_cursor < tokens.TokenCount )
	{
		const Token& token = tokens.Tokens[token_cursor];

		switch( token.Type )
		{
			case TokenType::COMMENT:		++token_cursor; break;
			case TokenType::WHITESPACE:		++token_cursor; break;
			case TokenType::LABEL:
				if( IsTokenContentEquals( text_data, tokens, token_cursor, "branch" ) )
				{
					if( Error error = Parse_Branch( text_data, tokens, token_cursor, out_ast, *createdNode ) )
						return error;
					createdNode = &out_ast.Nodes[*createdNode].NextNode;
				}
				else if( IsTokenContentEquals( text_data, tokens, token_cursor, "func" ) )
				{
					if( Error error = Parse_Func( text_data, tokens, token_cursor, out_ast, *createdNode ) )
						return error;
					createdNode = &out_ast.Nodes[*createdNode].NextNode;
				}
				else if( IsTokenContentEquals( text_data, tokens, token_cursor, "struct" ) )
				{
					if( Error error = Parse_Struct( text_data, tokens, token_cursor, out_ast, *createdNode ) )
						return error;
					createdNode = &out_ast.Nodes[*createdNode].NextNode;
				}
				else if( IsTokenContentEquals( text_data, tokens, token_cursor, "enum" ) )
				{
					if( Error error = Parse_Enum( text_data, tokens, token_cursor, out_ast, *createdNode ) )
						return error;
					createdNode = &out_ast.Nodes[*createdNode].NextNode;
				}
				else if( IsTokenContentEquals( text_data, tokens, token_cursor, "var" ) )
				{
					if( Error error = Parse_Var( text_data, tokens, token_cursor, out_ast, *createdNode, true ) )
						return error;
					createdNode = &out_ast.Nodes[*createdNode].NextNode;
				}
				else
					return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
				break;
			default:
				return Error( ErrorType::UNEXPECTED_TOKEN, token_cursor );
		}
	}

	return Error::OK;
}

//----------------------------------------
const char* GetASTNodeName( ASTNodeType ast_node_type )
{
	if( (uint32_t)ast_node_type < l_astNamesCount)
		return l_astNames[(uint32_t)ast_node_type];
	else
		return "#ERROR INVALID ASTNodeType";
}