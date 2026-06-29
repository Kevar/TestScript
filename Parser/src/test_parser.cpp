#include "test_parser.h"

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

//#pragma region Utilities
//
////----------------------------------------
//void PrintSlice( const char* buffer, uint32_t from, uint32_t to )
//{
//	uint32_t i = from;
//	while( i < to && buffer[i] != '\0' )
//	{
//		std::cout << buffer[i];
//		++i;
//	}
//}
//
////----------------------------------------
//void PrintSlice( const char* buffer, uint32_t count )
//{
//	PrintSlice( buffer, 0, count );
//}
//
//#pragma endregion

//----------------------------------------
namespace PARSER
{
//#pragma region FileData
//
//	//----------------------------------------
//	bool LoadFile( const char* path, FileData& out_file_data )
//	{
//		bool success = false;
//
//		out_file_data.Path = path;
//
//		FILE* file;
//		errno_t error = fopen_s( &file, out_file_data.Path, "r" );
//
//		if( file != nullptr && error == 0 )
//		{
//			if( fseek( file, 0, SEEK_END ) == 0 )
//			{
//				long size = ftell( file );	// Note : ça semble être une taille maximale mais les données lues sont souvent plus courtes d'1 ou 2 caractères. Pas encore compris pourquoi
//				if( size != -1 )
//				{
//					out_file_data.Buffer = new char[size];
//
//					if( fseek( file, 0, SEEK_SET ) == 0 )
//					{
//						out_file_data.Length = fread( out_file_data.Buffer, sizeof( char ), size, file );
//
//						if( ferror( file ) == 0 )
//						{
//							success = true;
//						}
//					}
//				}
//			}
//
//			fclose( file );
//		}
//
//		if( !success )
//		{
//			delete[] out_file_data.Buffer;
//			out_file_data.Buffer = nullptr;
//			out_file_data.Length = 0;
//		}
//
//		return success;
//	}
//
//	//----------------------------------------
//	void CloseFile( FileData& file_data )
//	{
//		delete[] file_data.Buffer;
//		file_data.Buffer = nullptr;
//		file_data.Length = 0;
//	}
//
//	//----------------------------------------
//	void PrintFile( const FileData& file_data )
//	{
//		for( uint32_t i = 0; i < file_data.Length; ++i )
//		{
//			std::cout << file_data.Buffer[i];
//		}
//	}
//
//#pragma endregion

//#pragma region Tokenization
//
//	//----------------------------------------
//	static const char* l_tokenNames[TokenType::COUNT] =
//	{
//		"WHITESPACE",
//
//		"OPENED_CURLY_BRACE",
//		"CLOSED_CURLY_BRACE",
//		"OPENED_PARENTHESIS",
//		"CLOSED_PARENTHESIS",
//		"OPENED_SQUARE_BRACKET",
//		"CLOSED_SQUARE_BRACKET",
//
//		"COLON",
//		"SEMI_COLON",
//		"COMMA",
//		"EQUAL",
//
//		"PLUS",
//		"MINUS",
//		"STAR",
//		"SLASH",
//
//		"DOT",
//
//		"INFERIOR",
//		"SUPERIOR",
//		
//		"INFERIOR_EQUAL",
//		"SUPERIOR_EQUAL",
//
//		"COMPARISON",
//		"COMPARISON_NOT",
//
//		"AND",
//		"OR",
//
//		"BIT_AND",
//		"BIT_OR",
//
//		"EXCLAMATION_MARK",
//		"QUESTION_MARK",
//
//		"NUMBER",
//		"LABEL",
//		"STRING",
//		"COMMENT",
//
//		"UNDEFINED",
//
//		"END"
//	};
//
//	//----------------------------------------
//	static bool IsNextCharacterIs( const FileData& file_data, uint32_t cursor, char c )
//	{
//		return cursor + 1 < file_data.Length && file_data.Buffer[cursor + 1] == c;
//	}
//
//	//----------------------------------------
//	static bool IsWhitespace( char c )
//	{
//		return c == ' ' || c == '\t' || c == '\r' || c == '\n';
//	}
//
//	//----------------------------------------
//	static bool IsLetter( char c )
//	{
//		return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
//	}
//
//	//----------------------------------------
//	static bool IsNumeric( char c )
//	{
//		return c >= '0' && c <= '9';
//	}
//
//	//----------------------------------------
//	static bool IsNumericOperator( char c )
//	{
//		return c == '+' || c == '-' || c == '*' || c == '/';
//	}
//
//	//----------------------------------------
//	static bool IsLabelCharacter( char c )
//	{
//		return IsLetter( c ) || IsNumeric( c ) || c == '_';
//	}
//
//	//----------------------------------------
//	static void ParseUndefined( const FileData& file_data, Token& token, uint32_t& cursor )
//	{
//		++cursor;
//
//		token.Type = TokenType::UNDEFINED;
//
//		while( cursor < file_data.Length && !IsWhitespace( file_data.Buffer[cursor] ) )
//		{
//			// #TODO : il faudrait détecter le début d'un commentaire et switcher dessus si besoin
//			++cursor;
//		}
//	}
//
//	//----------------------------------------
//	static void ParseWhitespace( const FileData& file_data, Token& token, uint32_t& cursor )
//	{
//		++cursor;
//
//		token.Type = TokenType::WHITESPACE;
//
//		while( cursor < file_data.Length && IsWhitespace( file_data.Buffer[cursor] ) )
//			++cursor;
//	}
//
//	//----------------------------------------
//	static void ParseNumeric( const FileData& file_data, Token& token, uint32_t& cursor )
//	{
//		++cursor;
//
//		token.Type = TokenType::NUMBER;
//
//		bool pointFound = false;
//
//		while( cursor < file_data.Length )
//		{
//			char c = file_data.Buffer[cursor];
//
//			if( c == '.' )
//			{
//				if( pointFound )
//				{
//					ParseUndefined( file_data, token, cursor );		// Double point : erreur
//					return;
//				}
//
//				pointFound = true;
//			}
//			else if( !IsNumeric( c ) )
//			{
//				if( pointFound && file_data.Buffer[cursor - 1] == '.' )
//				{
//					ParseUndefined( file_data, token, cursor );		// On n'a pas de chiffre juste après le point : erreur
//					return;
//				}
//				else
//				{
//					return;
//				}
//			}
//
//			++cursor;
//		}
//	}
//
//	//----------------------------------------
//	static void ParseLabel( const FileData& file_data, Token& token, uint32_t& cursor )
//	{
//		++cursor;
//
//		token.Type = TokenType::LABEL;
//
//		while( cursor < file_data.Length && IsLabelCharacter( file_data.Buffer[cursor] ) )
//			++cursor;
//	}
//
//	//----------------------------------------
//	static void ParseString( const FileData& file_data, Token& token, uint32_t& cursor )
//	{
//		++cursor;
//
//		token.Type = TokenType::UNDEFINED;		// On ne va le basculer en STRING que si on trouve un double quote de fermeture propre
//		bool escaping = false;
//
//		while( cursor < file_data.Length )
//		{
//			char c = file_data.Buffer[cursor];
//
//			if( c == '"' )
//			{
//				if( !escaping )
//				{
//					token.Type = TokenType::STRING;
//					++cursor;
//					return;
//				}
//				else
//					escaping = false;
//			}
//			else if( c == '\\' )
//			{
//				escaping = !escaping;
//			}
//			else if( escaping )
//			{
//				// #TODO : Gérer les autres caractères valides à échapper (comme \t)
//				// Et pour les caractères invalides, on a donc une string malformée, donc on doit remonter l'erreur mais on continue de parser quand même
//				escaping = false;
//			}
//
//			++cursor;
//		}
//	}
//
//	//----------------------------------------
//	static void ParseCommentOrSlash( const FileData& file_data, Token& token, uint32_t& cursor )
//	{
//		++cursor;
//
//		if( cursor >= file_data.Length || file_data.Buffer[cursor] != '/' )
//		{
//			token.Type = TokenType::SLASH;
//		}
//		else
//		{
//			token.Type = TokenType::COMMENT;
//			while( cursor < file_data.Length && file_data.Buffer[cursor] != '\n' )
//				++cursor;
//		}
//	}
//
//	//----------------------------------------
//	void ParseText( const FileData& file_data, TokenCollection& out_tokens )
//	{
//		out_tokens.TokenCount = 0;
//		uint32_t cursor = 0;
//
//		while( cursor < file_data.Length )
//		{
//			char c = file_data.Buffer[cursor];
//
//			Token& token = out_tokens.Tokens[out_tokens.TokenCount];
//			token.Position = cursor;
//			out_tokens.TokenCount++;
//
//			switch( c )
//			{
//				case '{':	token.Type = TokenType::OPENED_CURLY_BRACE;		++cursor;	break;
//				case '}':	token.Type = TokenType::CLOSED_CURLY_BRACE;		++cursor;	break;
//				case '[':	token.Type = TokenType::OPENED_SQUARE_BRACKET;	++cursor;	break;
//				case ']':	token.Type = TokenType::CLOSED_SQUARE_BRACKET;	++cursor;	break;
//				case '(':	token.Type = TokenType::OPENED_PARENTHESIS;		++cursor;	break;
//				case ')':	token.Type = TokenType::CLOSED_PARENTHESIS;		++cursor;	break;
//				case ':':	token.Type = TokenType::COLON;					++cursor;	break;
//				case ';':	token.Type = TokenType::SEMI_COLON;				++cursor;	break;
//				case ',':	token.Type = TokenType::COMMA;					++cursor;	break;
//				case '!':
//				{
//					if( IsNextCharacterIs( file_data, cursor, '=' ) )
//					{
//						token.Type = TokenType::COMPARISON_NOT;
//						cursor += 2;
//					}
//					else
//					{
//						token.Type = TokenType::EXCLAMATION_MARK;
//						++cursor;
//					}
//				}
//				break;
//				case '?':	token.Type = TokenType::QUESTION_MARK;			++cursor;	break;
//				case '=':
//				{
//					if( IsNextCharacterIs( file_data, cursor, '=' ) )
//					{
//						token.Type = TokenType::COMPARISON;
//						cursor += 2;
//					}
//					else
//					{
//						token.Type = TokenType::EQUAL;
//						++cursor;
//					}
//				}
//				break;
//				case '+':	token.Type = TokenType::PLUS;					++cursor;	break;
//				case '-':	token.Type = TokenType::MINUS;					++cursor;	break;
//				case '*':	token.Type = TokenType::STAR;					++cursor;	break;
//				case '.':	token.Type = TokenType::DOT;					++cursor;	break;
//				case '<':
//				{
//					if( IsNextCharacterIs( file_data, cursor, '=' ) )
//					{
//						token.Type = TokenType::INFERIOR_EQUAL;
//						cursor += 2;
//					}
//					else
//					{
//						token.Type = TokenType::INFERIOR;
//						++cursor;
//					}
//				}
//				break;
//				case '>':
//				{
//					if( IsNextCharacterIs( file_data, cursor, '=' ) )
//					{
//						token.Type = TokenType::SUPERIOR_EQUAL;
//						cursor += 2;
//					}
//					else
//					{
//						token.Type = TokenType::SUPERIOR;
//						++cursor;
//					}
//				}
//				break;
//				case '&':
//				{
//					if( IsNextCharacterIs( file_data, cursor, '&' ) )
//					{
//						token.Type = TokenType::AND;
//						cursor += 2;
//					}
//					else
//					{
//						token.Type = TokenType::BIT_AND;
//						++cursor;
//					}
//				}
//				break;
//				case '|':
//				{
//					if( IsNextCharacterIs( file_data, cursor, '|' ) )
//					{
//						token.Type = TokenType::OR;
//						cursor += 2;
//					}
//					else
//					{
//						token.Type = TokenType::BIT_OR;
//						++cursor;
//					}
//				}
//				break;
//				case '/':	ParseCommentOrSlash( file_data, token, cursor );			break;
//				case '"':	ParseString( file_data, token, cursor );					break;
//				default:	
//					if( IsLetter( c ) )
//						ParseLabel( file_data, token, cursor );
//					else if( IsWhitespace( c ) )
//						ParseWhitespace( file_data, token, cursor );
//					else if(IsNumeric(c) )
//						ParseNumeric( file_data, token, cursor );
//					else
//						ParseUndefined( file_data, token, cursor );
//					break;
//			}
//		}
//
//		Token& token = out_tokens.Tokens[out_tokens.TokenCount];
//		token.Type = TokenType::END;
//		token.Position = cursor;
//		// out_tokens.TokenCount++;		// Le END est une sentinelle au cas où on dépasse le nombre de token
//	}
//
//	//----------------------------------------
//	void PrintTokens( const FileData& file_data, const TokenCollection& tokens, bool display_whitespaces )
//	{
//		for( uint32_t i = 0; i < tokens.TokenCount - 1; ++i )		// -1 pour retirer le token END qui est là uniquement pour des raisons techniques
//		{
//			const Token& token = tokens.Tokens[i];
//
//			if( !display_whitespaces && token.Type == TokenType::WHITESPACE )
//				continue;
//
//			const Token& nextToken = tokens.Tokens[i + 1];
//
//			std::cout << "@[" << token.Position << ":" << nextToken.Position << "] " << l_tokenNames[(uint32_t)token.Type] << "\n";
//
//			if( token.Type == TokenType::LABEL || token.Type == TokenType::STRING || token.Type == TokenType::COMMENT || token.Type == TokenType::UNDEFINED )
//			{
//				std::cout << "\t";
//				PrintSlice( file_data.Buffer, token.Position, nextToken.Position );
//				std::cout << "\n";
//			}
//		}
//	}
//
//#pragma endregion

#pragma region AST

	//// Forward declarations
	//static bool Parse_Scope( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );
	//static bool Parse_ConditionBlock( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );
	//static bool Parse_WhileLoop( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index );
	//static bool Parse_Expression( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index, int right_binding_power );

	////----------------------------------------
	//static const char* l_astNames[ASTNodeType::COUNT] =
	//{
	//	"STRUCT",
	//	"ENUM",
	//	"ENUM_VALUE",
	//	"BRANCH",
	//	"FUNCTION",
	//	"ARGUMENT",
	//	"SCOPE",
	//	"VARIABLE_DECLARATION",
	//	"VARIABLE_ASSIGNATION",
	//	"VARIABLE_DECLARATION_ASSIGNATION",
	//	"FUNCTION_CALL",
	//	"IF_BLOCK",
	//	"WHILE_LOOP",
	//	"CONSTANT_FLOAT",
	//	"CONSTANT_INT",
	//	"CONSTANT_BOOL",
	//	"CONSTANT_STRING",
	//	"VARIABLE_CALL",
	//	"UNARY_OPERATOR",
	//	"BINARY_OPERATOR",
	//	"MEMBER_CALL",
	//	"RETURN_CALL",

	//	"UNKNOWN"
	//};

//#pragma region Helpers
//
//	////----------------------------------------
//	//// Note : pour les nombres négatifs on passe par une expression avec opérateur unaire '-'
//	//static bool ParseFloat( const FileData& file_data, uint32_t cursor, float& out_value, bool& castable_as_int)
//	//{
//	//	bool pointFound = false;
//	//	out_value = 0.0f;
//	//	castable_as_int = true;
//
//	//	float decimalMultiplier = 1.0f;
//
//	//	while( cursor < file_data.Length )
//	//	{
//	//		char c = file_data.Buffer[cursor];
//
//	//		if( c == '.' )
//	//		{
//	//			if( pointFound )
//	//				return false;
//	//		
//	//			pointFound = true;
//	//			castable_as_int = false;
//	//		}
//	//		else if( !IsNumeric( c ) )
//	//		{
//	//			if( pointFound && file_data.Buffer[cursor - 1] == '.' )
//	//			{
//	//				// On n'a pas de chiffre juste après le point : erreur
//	//				return false;
//	//			}
//	//			else
//	//			{
//	//				out_value *= decimalMultiplier;
//	//				return true;
//	//			}
//	//		}
//	//		else
//	//		{
//	//			out_value *= 10.0f;
//
//	//			if( pointFound )
//	//				decimalMultiplier *= 0.1f;
//
//	//			switch( c )
//	//			{
//	//				case '0': break;
//	//				case '1': out_value += 1; break;
//	//				case '2': out_value += 2; break;
//	//				case '3': out_value += 3; break;
//	//				case '4': out_value += 4; break;
//	//				case '5': out_value += 5; break;
//	//				case '6': out_value += 6; break;
//	//				case '7': out_value += 7; break;
//	//				case '8': out_value += 8; break;
//	//				case '9': out_value += 9; break;
//	//			}
//	//		}
//
//	//		++cursor;
//	//	}
//
//	//	out_value *= decimalMultiplier;
//	//	return true;
//	//}
//
//	////----------------------------------------
//	//static bool IsTokenContentEquals( const FileData& file_data, const TokenCollection& tokens, uint32_t token_cursor, const char* comparison )
//	//{
//	//	if( token_cursor >= tokens.TokenCount - 1 )
//	//		return false;
//	//
//	//	const Token& token = tokens.Tokens[token_cursor];
//	//	const Token& nextToken = tokens.Tokens[token_cursor + 1];
//	//
//	//	uint32_t readCursor = 0;
//	//	uint32_t tokenLength = nextToken.Position - token.Position;
//	//
//	//	while( readCursor < tokenLength && comparison[readCursor] != '\0' )
//	//	{
//	//		if( file_data.Buffer[token.Position + readCursor] != comparison[readCursor] )
//	//			return false;
//	//
//	//		++readCursor;
//	//	}
//	//
//	//	if( readCursor < tokenLength )			// Il reste des caractères dans le token
//	//		return false;
//	//
//	//	if( comparison[readCursor] != '\0' )	// Il reste des caractères dans la chaîne de comparaison
//	//		return false;
//	//
//	//	return true;
//	//}
//
//	////----------------------------------------
//	//static const Token& PeekNextToken( const TokenCollection& tokens, uint32_t token_cursor )
//	//{
//	//	++token_cursor;
//
//	//	while( token_cursor < tokens.TokenCount && ( tokens.Tokens[token_cursor].Type == TokenType::WHITESPACE || tokens.Tokens[token_cursor].Type == TokenType::COMMENT ) )
//	//		++token_cursor;
//
//	//	return tokens.Tokens[token_cursor];	// Note : si on est à token_cursor == tokens.TokenCount, on va renvoyer le token END, ce qui marche quand même
//	//}
//
//	////----------------------------------------
//	//static const Token& GetNextToken( const TokenCollection& tokens, uint32_t& token_cursor )
//	//{
//	//	++token_cursor;
//
//	//	while( token_cursor < tokens.TokenCount && ( tokens.Tokens[token_cursor].Type == TokenType::WHITESPACE || tokens.Tokens[token_cursor].Type == TokenType::COMMENT ) )
//	//		++token_cursor;
//
//	//	return tokens.Tokens[token_cursor];	// Note : si on est à token_cursor == tokens.TokenCount, on va renvoyer le token END, ce qui marche quand même
//	//}
//
//	////----------------------------------------
//	//static bool IsCurrentTokenOfType( const TokenCollection& tokens, uint32_t token_cursor, std::initializer_list<TokenType> types )
//	//{
//	//	TokenType type = tokens.Tokens[token_cursor].Type;
//
//	//	for( const TokenType& t : types )
//	//	{
//	//		if( type == t )
//	//			return true;
//	//	}
//
//	//	return false;
//	//}
//
//	////----------------------------------------
//	//static bool IsNextTokenOfType( const TokenCollection& tokens, uint32_t& token_cursor, TokenType& out_type, std::initializer_list<TokenType> types, bool consume )
//	//{
//	//	uint32_t workingCursor = token_cursor;
//	//	++workingCursor;
//
//	//	while( workingCursor < tokens.TokenCount && ( tokens.Tokens[workingCursor].Type == TokenType::WHITESPACE || tokens.Tokens[workingCursor].Type == TokenType::COMMENT ) )
//	//		++workingCursor;
//
//	//	out_type = tokens.Tokens[workingCursor].Type;
//
//	//	if( consume )
//	//		token_cursor = workingCursor;
//
//	//	for( const TokenType& t : types )
//	//	{
//	//		if( out_type == t )
//	//			return true;
//	//	}
//
//	//	return false;
//	//}
//
//	////----------------------------------------
//	//static ASTNode& AddASTNode( ASTNodeCollection& ast, ASTNodeType type, ast_node_index& out_created_node_index )
//	//{
//	//	ASTNode& node = ast.Nodes[ast.NodeCount];
//	//	node.Type = type;
//	//	node.NextNode = 0;		// Pointer vers 0 indique qu'il n'y a pas de next node (0 étant le premier node, il ne peut jamais être le NextNode d'un autre)
//	//	out_created_node_index = ast.NodeCount;
//	//	++ast.NodeCount;
//
//	//	return node;
//	//}
//
//#pragma endregion

//#pragma region Argument lists
//
//	//----------------------------------------
//	static bool Parse_ArgumentDeclaration( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::LABEL )
//			return false;
//
//		ASTNode& node = AddASTNode( ast, ASTNodeType::ARGUMENT, out_created_node_index );
//		node.Argument.Name = token_cursor;
//
//		if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::COLON }, true ) )
//			return false;
//
//		if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL }, true ) )
//			return false;
//
//		node.Argument.Type = token_cursor;
//
//		GetNextToken( tokens, token_cursor );	// On consomme le label du type
//
//		return true;
//	}
//
//	//----------------------------------------
//	static bool Parse_ArgumentDeclarationList( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::OPENED_PARENTHESIS )
//			return false;
//
//		ast_node_index* linkChain = &out_created_node_index;
//
//		GetNextToken( tokens, token_cursor );
//
//		while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_PARENTHESIS } ) )
//		{
//			if( !Parse_ArgumentDeclaration( file_data, tokens, token_cursor, ast, *linkChain ) )
//				return false;
//
//			linkChain = &ast.Nodes[*linkChain].NextNode;
//
//			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COMMA } ) )
//				GetNextToken( tokens, token_cursor );
//		}
//
//		GetNextToken( tokens, token_cursor );	// On consomme la parenthèse fermante
//
//		return true;
//	}
//
//	//----------------------------------------
//	static bool Parse_ArgumentCallList( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::OPENED_PARENTHESIS )
//			return false;
//
//		ast_node_index* linkChain = &out_created_node_index;
//
//		GetNextToken( tokens, token_cursor );
//
//		while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_PARENTHESIS } ) )
//		{
//			if( !Parse_Expression( file_data, tokens, token_cursor, ast, *linkChain, 0 ) )
//				return false;
//
//			linkChain = &ast.Nodes[*linkChain].NextNode;
//
//			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COMMA } ) )
//				GetNextToken( tokens, token_cursor );
//		}
//
//		GetNextToken( tokens, token_cursor );	// On consomme la parenthèse fermante
//
//		return true;
//	}
//
//#pragma endregion

	////----------------------------------------
	//static bool Parse_Var( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
	//{
	//	if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) || !IsTokenContentEquals( file_data, tokens, token_cursor, "var" ) )
	//		return false;

	//	TokenType tokenType;

	//	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL }, true ) )
	//		return false;

	//	token_index varNameToken = token_cursor;

	//	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::COLON }, true ) )
	//		return false;

	//	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL }, true ) )
	//		return false;

	//	token_index varTypeToken = token_cursor;

	//	if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::SEMI_COLON, TokenType::EQUAL }, true ) )
	//		return false;

	//	if( tokenType == TokenType::SEMI_COLON )
	//	{
	//		ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_DECLARATION, out_created_node_index );

	//		node.VariableDeclaration.Name = varNameToken;
	//		node.VariableDeclaration.Type = varTypeToken;

	//		GetNextToken( tokens, token_cursor );
	//		return true;
	//	}
	//	else // if( tokenType == TokenType::EQUAL )
	//	{
	//		ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_DECLARATION_ASSIGNATION, out_created_node_index );

	//		node.VariableDeclarationAssignation.Name = varNameToken;
	//		node.VariableDeclarationAssignation.Type = varTypeToken;

	//		GetNextToken( tokens, token_cursor );	// On consomme le '='

	//		if( !Parse_Expression( file_data, tokens, token_cursor, ast, node.VariableDeclarationAssignation.Value, 0 ) )
	//			return false;

	//		if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
	//		{
	//			GetNextToken( tokens, token_cursor );
	//			return true;
	//		}
	//		else
	//			return false;
	//	}
	//}

	////----------------------------------------
	//static bool Parse_Instruction( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
	//{
	//	// #NOTE : la valeur par défaut de out_created_node_index est déjà bonne pour indiquer "pas de lien". On peut donc sortir de cette fonction sans l'affecter si besoin
	//	TokenType tokenType = tokens.Tokens[token_cursor].Type;

	//	if( tokenType == TokenType::OPENED_CURLY_BRACE )
	//	{
	//		return Parse_Scope( file_data, tokens, token_cursor, ast, out_created_node_index );
	//	}
	//	else if( tokenType == TokenType::LABEL )
	//	{
	//		if( IsTokenContentEquals( file_data, tokens, token_cursor, "if" ) )
	//		{
	//			// if block
	//			return Parse_ConditionBlock( file_data, tokens, token_cursor, ast, out_created_node_index );
	//		}
	//		else if( IsTokenContentEquals( file_data, tokens, token_cursor, "while" ) )
	//		{
	//			return Parse_WhileLoop( file_data, tokens, token_cursor, ast, out_created_node_index );
	//		}
	//		else if( IsTokenContentEquals( file_data, tokens, token_cursor, "return" ) )
	//		{
	//			ASTNode& node = AddASTNode( ast, ASTNodeType::RETURN_CALL, out_created_node_index );
	//			node.ReturnCall.Value = 0;

	//			GetNextToken( tokens, token_cursor );	// On consomme le return

	//			// Simple instruction return sans paramètre
	//			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
	//			{
	//				GetNextToken( tokens, token_cursor );
	//				return true;
	//			}

	//			if( !Parse_Expression( file_data, tokens, token_cursor, ast, node.ReturnCall.Value, 0 ) )
	//				return false;

	//			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
	//			{
	//				GetNextToken( tokens, token_cursor );
	//				return true;
	//			}
	//			else
	//				return false;
	//		}
	//		else if( IsTokenContentEquals( file_data, tokens, token_cursor, "var" ) )
	//		{
	//			return Parse_Var( file_data, tokens, token_cursor, ast, out_created_node_index );
	//		}
	//		else
	//		{
	//			token_index nameToken = token_cursor;

	//			if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::EQUAL, TokenType::OPENED_PARENTHESIS }, false ) )
	//				return false;

	//			if( tokenType == TokenType::EQUAL )
	//			{
	//				GetNextToken( tokens, token_cursor );
	//				ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_ASSIGNATION, out_created_node_index );

	//				node.VariableAssignation.Name = nameToken;

	//				GetNextToken( tokens, token_cursor );	// On consomme le '='

	//				if( !Parse_Expression( file_data, tokens, token_cursor, ast, node.VariableAssignation.Value, 0 ) )
	//					return false;

	//				if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
	//				{
	//					GetNextToken( tokens, token_cursor );
	//					return true;
	//				}
	//				else
	//					return false;

	//			}
	//			else // if( tokenType == TokenType::OPENED_PARENTHESIS )
	//			{
	//				ASTNode& node = AddASTNode( ast, ASTNodeType::FUNCTION_CALL, out_created_node_index );
	//				node.FunctionCall.Name = nameToken;
	//				node.FunctionCall.FirstArgument = 0;	// Par défaut : sans argument

	//				GetNextToken( tokens, token_cursor );	// On consomme le nom de la fonction

	//				if( !Parse_ArgumentCallList( file_data, tokens, token_cursor, ast, node.FunctionCall.FirstArgument ) )
	//					return false;

	//				if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::SEMI_COLON } ) )
	//				{
	//					GetNextToken( tokens, token_cursor );
	//					return true;
	//				}
	//				else
	//					return false;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}

//#pragma region Blocks
//
//	//----------------------------------------
//	static bool Parse_Scope( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::OPENED_CURLY_BRACE )
//			return false;
//
//		ASTNode& node = AddASTNode( ast, ASTNodeType::SCOPE, out_created_node_index );
//		node.Scope.FirstInstruction = 0;
//
//		ast_node_index* linkChain = &node.Scope.FirstInstruction;
//		
//		GetNextToken( tokens, token_cursor );
//
//		while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_CURLY_BRACE } ) )
//		{
//			if( !Parse_Instruction( file_data, tokens, token_cursor, ast, *linkChain ) )
//				return false;
//
//			linkChain = &ast.Nodes[*linkChain].NextNode;
//		}
//
//		GetNextToken( tokens, token_cursor );	// On consomme l'accolade fermante
//
//		return true;
//	}
//
//	//----------------------------------------
//	static bool Parse_Struct( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		GetNextToken( tokens, token_cursor );	// On consomme le "struct"
//
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::LABEL )
//			return false;
//
//		ASTNode& structNode = AddASTNode( ast, ASTNodeType::STRUCT, out_created_node_index );
//		structNode.Struct.Name = token_cursor;
//		structNode.Struct.FirstMember = 0;
//
//		GetNextToken( tokens, token_cursor );
//
//		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::OPENED_CURLY_BRACE } ) )
//			return false;
//
//		ast_node_index* linkChain = &structNode.Struct.FirstMember;
//
//		GetNextToken( tokens, token_cursor );
//
//		while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_CURLY_BRACE } ) )
//		{
//			if( !Parse_Var( file_data, tokens, token_cursor, ast, *linkChain ) )
//				return false;
//
//			linkChain = &ast.Nodes[*linkChain].NextNode;
//		}
//
//		GetNextToken( tokens, token_cursor );	// On consomme l'accolade fermante
//
//		return true;
//	}
//
//	//----------------------------------------
//	static bool Parse_Enum( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		GetNextToken( tokens, token_cursor );	// On consomme le "enum"
//
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::LABEL )
//			return false;
//
//		ASTNode& enumNode = AddASTNode( ast, ASTNodeType::ENUM, out_created_node_index );
//		enumNode.Enum.Name = token_cursor;
//		enumNode.Enum.FirstValue = 0;
//
//		GetNextToken( tokens, token_cursor );
//
//		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::OPENED_CURLY_BRACE } ) )
//			return false;
//
//		ast_node_index* linkChain = &enumNode.Enum.FirstValue;
//
//		GetNextToken( tokens, token_cursor );
//
//		while( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_CURLY_BRACE } ) )
//		{
//			if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) )
//				return false;
//
//			ASTNode& enumValueNode = AddASTNode( ast, ASTNodeType::ENUM_VALUE, *linkChain );
//			enumValueNode.EnumValue.Value = token_cursor;
//			linkChain = &enumValueNode.NextNode;
//
//			GetNextToken( tokens, token_cursor );
//
//			if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COMMA } ) )
//				GetNextToken( tokens, token_cursor );
//		}
//
//		GetNextToken( tokens, token_cursor );	// On consomme l'accolade fermante
//
//		return true;
//	}
//
//	//----------------------------------------
//	static bool Parse_Func( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		GetNextToken( tokens, token_cursor );	// On consomme le "func" (ou le "branch" car on mutualise le code de parsing entre les deux)
//
//		TokenType tokenType = tokens.Tokens[token_cursor].Type;
//
//		if( tokenType != TokenType::LABEL )
//			return false;
//
//		ASTNode& functionNode = AddASTNode( ast, ASTNodeType::FUNCTION, out_created_node_index );
//		functionNode.Function.Name = token_cursor;
//		functionNode.Function.FirstArgument = 0;
//
//		GetNextToken( tokens, token_cursor );
//
//		if( !Parse_ArgumentDeclarationList(file_data, tokens, token_cursor, ast, functionNode.Function.FirstArgument ) )
//			return false;
//
//		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COLON } ) )
//			return false;
//
//		GetNextToken( tokens, token_cursor );
//
//		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL }) )
//			return false;
//
//		functionNode.Function.ReturnType = token_cursor;
//
//		GetNextToken( tokens, token_cursor );
//
//		return Parse_Scope( file_data, tokens, token_cursor, ast, functionNode.Function.Body );
//	}
//
//	//----------------------------------------
//	static bool Parse_Branch( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		ASTNode& branchNode = AddASTNode( ast, ASTNodeType::BRANCH, out_created_node_index );
//		
//		return Parse_Func( file_data, tokens, token_cursor, ast, branchNode.Branch.Function );
//	}
//
//	//----------------------------------------
//	static bool Parse_ConditionBlock( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) || !IsTokenContentEquals( file_data, tokens, token_cursor, "if" ) )
//			return false;
//
//		ASTNode& ifNode = AddASTNode( ast, ASTNodeType::IF_BLOCK, out_created_node_index );
//
//		GetNextToken( tokens, token_cursor );
//
//		if( !Parse_Expression( file_data, tokens, token_cursor, ast, ifNode.ConditionBlock.Condition, 0 ) )
//			return false;
//
//		if( !Parse_Scope( file_data, tokens, token_cursor, ast, ifNode.ConditionBlock.IfBlock ) )
//			return false;
//
//		if( IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) && IsTokenContentEquals( file_data, tokens, token_cursor, "else" ) )
//		{
//			TokenType tokenType;
//			if( !IsNextTokenOfType( tokens, token_cursor, tokenType, { TokenType::LABEL, TokenType::OPENED_CURLY_BRACE }, true ) )
//				return false;
//
//			if( tokenType == TokenType::LABEL && IsTokenContentEquals( file_data, tokens, token_cursor, "if" ) )
//				return Parse_ConditionBlock( file_data, tokens, token_cursor, ast, ifNode.ConditionBlock.ElseBlock );
//			else
//				return Parse_Scope( file_data, tokens, token_cursor, ast, ifNode.ConditionBlock.ElseBlock );
//		}
//		else
//		{
//			ifNode.ConditionBlock.ElseBlock = 0;
//
//			return true;
//		}
//	}
//
//	//----------------------------------------
//	static bool Parse_WhileLoop( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::LABEL } ) || !IsTokenContentEquals( file_data, tokens, token_cursor, "while" ) )
//			return false;
//
//		ASTNode& whileNode = AddASTNode( ast, ASTNodeType::WHILE_LOOP, out_created_node_index );
//
//		GetNextToken( tokens, token_cursor );
//
//		if( !Parse_Expression( file_data, tokens, token_cursor, ast, whileNode.WhileLoop.Condition, 0 ) )
//			return false;
//
//		if( !Parse_Scope( file_data, tokens, token_cursor, ast, whileNode.WhileLoop.Block ) )
//			return false;
//
//		return true;
//	}
//
//#pragma endregion

//#pragma region Expression
//
//	//----------------------------------------
//	// Pratt Parsing
//	//
//	// https://www.youtube.com/watch?v=Nlqv6NtBXcA&t=1171s
//	// https://crockford.com/javascript/tdop/tdop.html
//	// https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
//	//
//	// Règles :
//	//	* Le token courant quand on appelle les fonctions est déjà le token à tester/traiter
//	//  * Une opération (NUD, LED ou EXPR) consomme tous les tokens qu'elle traite : en sortie on est donc déjà sur le token suivant
//	//	* NUD ne sera appelé que pour le token de début d'expression, soit :
//	//		> Un opérateur unaire préfixe (comme !) qui crée son ASTNode, parse la suite comme une expression et retourne son node
//	//		> Un opérande qui crée son ASTNode et retourne son node
//	//		> Une sous-expression explicite (parenthèses) qui consomme la parenthèse, parse la suite comme une expression, consomme la parenthèse fermante et retourne le node de la sous expression
//	//	* LED est appelé de manière itérative sur tous les tokens suivants de l'expression (pas le premier)
//
//	//----------------------------------------
//	struct BindingPower
//	{
//		TokenType Token;
//		int LeftBindingPower;
//		int RightBindingPower;
//	};
//
//	static BindingPower l_bindingPowers[] =
//	{
//		{ TokenType::WHITESPACE, 0, 0 },
//
//		{ TokenType::OPENED_CURLY_BRACE, 0, 0 },
//		{ TokenType::CLOSED_CURLY_BRACE, 0, 0 },
//		{ TokenType::OPENED_PARENTHESIS, 0, 0 },
//		{ TokenType::CLOSED_PARENTHESIS, 0, 0 },
//		{ TokenType::OPENED_SQUARE_BRACKET, 0, 0 },
//		{ TokenType::CLOSED_SQUARE_BRACKET, 0, 0 },
//
//		{ TokenType::COLON, 0, 0 },
//		{ TokenType::SEMI_COLON, 0, 0 },
//		{ TokenType::COMMA, 0, 0 },
//		{ TokenType::EQUAL, 10, 11 },
//
//		{ TokenType::PLUS, 50, 51 },
//		{ TokenType::MINUS, 50, 51 },
//		{ TokenType::STAR, 60, 61 },
//		{ TokenType::SLASH, 60, 61 },
//
//		{ TokenType::DOT, 50, 51 },
//
//		{ TokenType::INFERIOR, 40, 41 },
//		{ TokenType::SUPERIOR, 40, 41 },
//
//		{ TokenType::INFERIOR_EQUAL, 40, 41 },
//		{ TokenType::SUPERIOR_EQUAL, 40, 41 },
//
//		{ TokenType::COMPARISON, 40, 41 },
//		{ TokenType::COMPARISON_NOT, 40, 41 },
//
//		{ TokenType::AND, 30, 31 },
//		{ TokenType::OR, 30, 31 },
//
//		{ TokenType::BIT_AND, 30, 31 },
//		{ TokenType::BIT_OR, 30, 31 },
//
//		{ TokenType::EXCLAMATION_MARK, 70, 71 },
//		{ TokenType::QUESTION_MARK, 20, 21 },
//
//		{ TokenType::NUMBER, 0, 0 },
//		{ TokenType::LABEL, 0, 0 },
//		{ TokenType::STRING, 0, 0 },
//		{ TokenType::COMMENT, 0, 0 },
//
//		{ TokenType::UNDEFINED, 0, 0 },
//
//		{ TokenType::END, 0, 0 }
//	};
//
//	//----------------------------------------
//	static bool Parse_NUD( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index )
//	{
//		TokenType currentTokenType = tokens.Tokens[token_cursor].Type;
//
//		switch( currentTokenType )
//		{
//			case TokenType::MINUS:
//			case TokenType::PLUS:
//			case TokenType::EXCLAMATION_MARK:
//			{
//				ASTNode& node = AddASTNode( ast, ASTNodeType::UNARY_OPERATOR, out_created_node_index );
//				node.UnaryOperator.Operator = token_cursor;
//				GetNextToken( tokens, token_cursor );
//				return Parse_Expression( file_data, tokens, token_cursor, ast, node.UnaryOperator.Operand, l_bindingPowers[(uint32_t) currentTokenType].RightBindingPower );
//			}
//			break;
//			case TokenType::OPENED_PARENTHESIS:
//			{
//				GetNextToken( tokens, token_cursor );
//				if( !Parse_Expression( file_data, tokens, token_cursor, ast, out_created_node_index, /*l_bindingPowers[(uint32_t) currentTokenType].RightBindingPower*/0 ) )
//					return false;
//				if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::CLOSED_PARENTHESIS } ) )
//					return false;
//				GetNextToken( tokens, token_cursor );
//				
//				return true;
//			}
//			break;
//			case TokenType::LABEL:
//			{
//				// 3 cas : FUNCTION_CALL ou VARIABLE_CALL ou CONSTANT_BOOL
//				if( PeekNextToken( tokens, token_cursor ).Type == TokenType::OPENED_PARENTHESIS )
//				{
//					ASTNode& node = AddASTNode( ast, ASTNodeType::FUNCTION_CALL, out_created_node_index );
//					node.FunctionCall.Name = token_cursor;
//					node.FunctionCall.FirstArgument = 0;	// Par défaut : sans argument
//
//					GetNextToken( tokens, token_cursor );	// On consomme le nom de la fonction
//
//					return Parse_ArgumentCallList( file_data, tokens, token_cursor, ast, node.FunctionCall.FirstArgument );
//				}
//				else if( IsTokenContentEquals( file_data, tokens, token_cursor, "true" ) )
//				{
//					ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_BOOL, out_created_node_index );
//					node.ConstantBool.StrValue = token_cursor;
//					node.ConstantBool.Value = true;
//					GetNextToken( tokens, token_cursor );
//					return true;
//				}
//				else if( IsTokenContentEquals( file_data, tokens, token_cursor, "false" ) )
//				{
//					ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_BOOL, out_created_node_index );
//					node.ConstantBool.StrValue = token_cursor;
//					node.ConstantBool.Value = false;
//					GetNextToken( tokens, token_cursor );
//					return true;
//				}
//				else
//				{
//					ASTNode& node = AddASTNode( ast, ASTNodeType::VARIABLE_CALL, out_created_node_index );
//					node.VariableCall.Name = token_cursor;
//					GetNextToken( tokens, token_cursor );
//					return true;
//				}
//			}
//			break;
//			case TokenType::STRING:
//			{
//				ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_STRING, out_created_node_index );
//				node.ConstantString.Value = token_cursor;
//				GetNextToken( tokens, token_cursor );
//				return true;
//			}
//			break;
//			case TokenType::NUMBER:
//			{
//				bool castableAsInt;
//				float numberValue;
//				if( !ParseFloat( file_data, tokens.Tokens[token_cursor].Position, numberValue, castableAsInt ) )
//					return false;	// Normalement il est impossible d'arriver là
//
//				if( castableAsInt )
//				{
//					ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_INT, out_created_node_index );
//					node.ConstantInt.StrValue = token_cursor;
//					node.ConstantInt.Value = (int32_t) numberValue;
//					GetNextToken( tokens, token_cursor );
//					return true;
//				}
//				else
//				{
//					ASTNode& node = AddASTNode( ast, ASTNodeType::CONSTANT_FLOAT, out_created_node_index );
//					node.ConstantFloat.StrValue = token_cursor;
//					node.ConstantFloat.Value = numberValue;
//					GetNextToken( tokens, token_cursor );
//					return true;
//				}
//			}
//			break;
//			default:
//				return false;
//				break;
//		}
//
//		return true;
//	}
//
//	//----------------------------------------
//	static bool Parse_LED( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index left_node, ast_node_index& out_created_node_index )
//	{
//		TokenType currentTokenType = tokens.Tokens[token_cursor].Type;
//
//		switch( currentTokenType )
//		{
//			case TokenType::PLUS:
//			case TokenType::MINUS:
//			case TokenType::STAR:
//			case TokenType::SLASH:
//			case TokenType::INFERIOR:
//			case TokenType::SUPERIOR:
//			case TokenType::INFERIOR_EQUAL:
//			case TokenType::SUPERIOR_EQUAL:
//			case TokenType::COMPARISON:
//			case TokenType::COMPARISON_NOT:
//			case TokenType::AND:
//			case TokenType::OR:
//			case TokenType::BIT_AND:
//			case TokenType::BIT_OR:
//			{
//				ASTNode& node = AddASTNode( ast, ASTNodeType::BINARY_OPERATOR, out_created_node_index );
//				node.BinaryOperator.Operator = token_cursor;
//				node.BinaryOperator.FirstOperand = left_node;
//				GetNextToken( tokens, token_cursor );
//				return Parse_Expression( file_data, tokens, token_cursor, ast, node.BinaryOperator.SecondOperand, l_bindingPowers[(uint32_t) currentTokenType].LeftBindingPower );
//			}
//			break;
//			case TokenType::QUESTION_MARK:
//			{
//				ASTNode& node = AddASTNode( ast, ASTNodeType::TERNARY_OPERATOR, out_created_node_index );
//				node.TernaryOperator.Operator = token_cursor;
//				node.TernaryOperator.FirstOperand = left_node;
//				GetNextToken( tokens, token_cursor );
//				if( !Parse_Expression( file_data, tokens, token_cursor, ast, node.TernaryOperator.SecondOperand, l_bindingPowers[(uint32_t) currentTokenType].LeftBindingPower ) )
//					return false;
//				
//				if( !IsCurrentTokenOfType( tokens, token_cursor, { TokenType::COLON } ) )
//					return false;
//
//				GetNextToken( tokens, token_cursor );
//				return Parse_Expression( file_data, tokens, token_cursor, ast, node.TernaryOperator.ThirdOperand, l_bindingPowers[(uint32_t)TokenType::COLON].LeftBindingPower );
//			}
//			break;
//			case TokenType::DOT:		// Très similaire à Binary Operator, à fusionner ? Ou à l'inverse être plus spécifique sur tous les opérateurs binaires dans l'AST et avoir AST_ADD, AST_SUBSTRACT, etc. ?
//			{
//				ASTNode& node = AddASTNode( ast, ASTNodeType::MEMBER_CALL, out_created_node_index );
//				node.MemberCall.Parent = left_node;
//				GetNextToken( tokens, token_cursor );
//				return Parse_Expression( file_data, tokens, token_cursor, ast, node.MemberCall.Member, 0 );
//			}
//			default:
//				return false;
//				break;
//		}
//	}
//
//	//----------------------------------------
//	static bool Parse_Expression( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast, ast_node_index& out_created_node_index, int right_binding_power )
//	{
//		if( !Parse_NUD( file_data, tokens, token_cursor, ast, out_created_node_index ) )
//			return false;
//
//		TokenType currentTokenType = tokens.Tokens[token_cursor].Type;
//
//		while( l_bindingPowers[(uint32_t)currentTokenType].LeftBindingPower > right_binding_power )
//		{
//			if( !Parse_LED( file_data, tokens, token_cursor, ast, out_created_node_index, out_created_node_index ) )	// 1er out_created_node_index : copié dans les arguments, pas référencé
//				return false;
//
//			currentTokenType = tokens.Tokens[token_cursor].Type;
//		}
//
//		return true;
//	}
//
//#pragma endregion

	////----------------------------------------
	//static bool Parse_Script( const FileData& file_data, const TokenCollection& tokens, uint32_t& token_cursor, ASTNodeCollection& ast )
	//{
	//	ast_node_index sentinelNode;
	//	ast_node_index* createdNode = &sentinelNode;

	//	while( token_cursor < tokens.TokenCount )
	//	{
	//		const Token& token = tokens.Tokens[token_cursor];

	//		switch( token.Type )
	//		{
	//			case TokenType::COMMENT:		++token_cursor; break;
	//			case TokenType::WHITESPACE:		++token_cursor; break;
	//			case TokenType::LABEL:
	//				if( IsTokenContentEquals( file_data, tokens, token_cursor, "branch" ) )
	//				{
	//					if( !Parse_Branch( file_data, tokens, token_cursor, ast, *createdNode ) )
	//						return false;
	//					createdNode = &ast.Nodes[*createdNode].NextNode;
	//				}
	//				else if( IsTokenContentEquals( file_data, tokens, token_cursor, "func" ) )
	//				{
	//					if( !Parse_Func( file_data, tokens, token_cursor, ast, *createdNode ) )
	//						return false;
	//					createdNode = &ast.Nodes[*createdNode].NextNode;
	//				}
	//				else if( IsTokenContentEquals( file_data, tokens, token_cursor, "struct" ) )
	//				{
	//					if( !Parse_Struct( file_data, tokens, token_cursor, ast, *createdNode ) )
	//						return false;
	//					createdNode = &ast.Nodes[*createdNode].NextNode;
	//				}
	//				else if( IsTokenContentEquals( file_data, tokens, token_cursor, "enum" ) )
	//				{
	//					if( !Parse_Enum( file_data, tokens, token_cursor, ast, *createdNode ) )
	//						return false;
	//					createdNode = &ast.Nodes[*createdNode].NextNode;
	//				}
	//				else
	//					return false;
	//				break;
	//			default:
	//				return false;
	//		}
	//	}

	//	return true;
	//}

	////----------------------------------------
	//bool ParseTokens( const FileData& file_data, const TokenCollection& tokens, ASTNodeCollection& out_ast, token_index& out_error_token )
	//{
	//	uint32_t token_cursor = 0;

	//	if( !Parse_Script( file_data, tokens, token_cursor, out_ast ) )
	//	{
	//		out_error_token = token_cursor;
	//		return false;
	//	}
	//	else
	//		return true;
	//}

#pragma region Printing

	////----------------------------------------
	//static void PrintIndent( uint32_t indent )
	//{
	//	while( indent > 0 )
	//		std::cout << "  ";
	//}

	////----------------------------------------
	//static void PrintWithIndent( const char* content, uint32_t indent )
	//{
	//	while( indent > 0 )
	//	{
	//		std::cout << "  ";
	//		--indent;
	//	}

	//	std::cout << content;
	//}

	////----------------------------------------
	//static void PrintTokenValue( const FileData& file_data, const TokenCollection& tokens, token_index token )
	//{
	//	const Token& current = tokens.Tokens[token];
	//	const Token& next = tokens.Tokens[token + 1];

	//	PrintSlice( file_data.Buffer + current.Position, next.Position - current.Position );
	//}

	////----------------------------------------
	//static void PrintASTNode( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, uint32_t ast_cursor, uint32_t indent )
	//{
	//	if( ast_cursor < ast.NodeCount )
	//	{
	//		const ASTNode& node = ast.Nodes[ast_cursor];

	//		switch( node.RuntimeType.Type )
	//		{
	//			case RuntimeTypeType::NONE:
	//				std::cout << "[NONE]  ";
	//				break;
	//			case RuntimeTypeType::VOID:
	//				std::cout << "[VOID]  ";
	//				break;
	//			case RuntimeTypeType::BOOL:
	//				std::cout << "[BOOL]  ";
	//				break;
	//			case RuntimeTypeType::INT:
	//				std::cout << "[INT]   ";
	//				break;
	//			case RuntimeTypeType::FLOAT:
	//				std::cout << "[FLOAT] ";
	//				break;
	//			case RuntimeTypeType::USER_DEFINED:
	//			{
	//				if( node.RuntimeType.UserDefinedType < analyzis_data.TypeDefinitionCount )
	//				{
	//					const ASTNode& structNode = ast.Nodes[analyzis_data.TypeDefinitions[node.RuntimeType.UserDefinedType].Node];
	//					std::cout << "[";
	//					PrintTokenValue( file_data, tokens, structNode.Struct.Name );
	//					std::cout << "] ";
	//				}
	//				else
	//					std::cout << "[USER]  ";
	//			}
	//			break;
	//		}

	//		switch( node.Type )
	//		{
	//			case ASTNodeType::BRANCH:
	//				PrintWithIndent( "BRANCH:\n", indent );
	//				PrintWithIndent( "Function:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.Branch.Function, indent + 2 );
	//				break;
	//			case ASTNodeType::FUNCTION:
	//				PrintWithIndent( "FUNCTION:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.Function.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Return Type: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.Function.ReturnType );
	//				std::cout << "\n";
	//				PrintWithIndent( "Arguments:\n", indent + 1 );
	//				if( node.Function.FirstArgument != 0 )
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.Function.FirstArgument, indent + 2 );
	//				PrintWithIndent( "Body:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.Function.Body, indent + 2 );
	//				break;
	//			case ASTNodeType::ARGUMENT:
	//				PrintWithIndent( "ARGUMENT:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.Argument.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Type: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.Argument.Type );
	//				std::cout << "\n";
	//				break;
	//			case ASTNodeType::SCOPE:
	//				PrintWithIndent( "SCOPE:\n", indent );
	//				if( node.Scope.FirstInstruction != 0 )
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.Scope.FirstInstruction, indent + 1 );
	//				break;
	//			case ASTNodeType::VARIABLE_DECLARATION_ASSIGNATION:
	//				PrintWithIndent( "VARIABLE DECLARATION ASSIGNATION:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.VariableDeclarationAssignation.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Type: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.VariableDeclarationAssignation.Type );
	//				std::cout << "\n";
	//				PrintWithIndent( "Value:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.VariableDeclarationAssignation.Value, indent + 2 );
	//				break;
	//			case ASTNodeType::VARIABLE_DECLARATION:
	//				PrintWithIndent( "VARIABLE DECLARATION:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.VariableDeclaration.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Type: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.VariableDeclaration.Type );
	//				std::cout << "\n";
	//				break;
	//			case ASTNodeType::VARIABLE_ASSIGNATION:
	//				PrintWithIndent( "VARIABLE ASSIGNATION:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.VariableAssignation.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Value:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.VariableAssignation.Value, indent + 2 );
	//				break;
	//			case ASTNodeType::FUNCTION_CALL:
	//				PrintWithIndent( "FUNCTION CALL:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.FunctionCall.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Arguments:\n", indent + 1 );
	//				if( node.FunctionCall.FirstArgument != 0 )
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.FunctionCall.FirstArgument, indent + 2 );
	//				break;
	//			case ASTNodeType::CONSTANT_STRING:
	//				PrintWithIndent( "CONSTANT STRING:\n", indent );
	//				PrintWithIndent( "Value: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.ConstantString.Value );
	//				std::cout << "\n";
	//				break;
	//			case ASTNodeType::CONSTANT_FLOAT:
	//				PrintWithIndent( "CONSTANT FLOAT:\n", indent );
	//				PrintWithIndent( "String Value: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.ConstantFloat.StrValue );
	//				std::cout << "\n";
	//				PrintWithIndent( "Float Value: ", indent + 1 );
	//				std::cout << node.ConstantFloat.Value << "\n";
	//				break;
	//			case ASTNodeType::CONSTANT_INT:
	//				PrintWithIndent( "CONSTANT INT:\n", indent );
	//				PrintWithIndent( "String Value: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.ConstantInt.StrValue );
	//				std::cout << "\n";
	//				PrintWithIndent( "Int Value: ", indent + 1 );
	//				std::cout << node.ConstantInt.Value << "\n";
	//				break;
	//			case ASTNodeType::CONSTANT_BOOL:
	//				PrintWithIndent( "CONSTANT BOOL:\n", indent );
	//				PrintWithIndent( "String Value: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.ConstantBool.StrValue );
	//				std::cout << "\n";
	//				PrintWithIndent( "Bool Value: ", indent + 1 );
	//				std::cout << (node.ConstantBool.Value ? "true" : "false") << "\n";
	//				break;
	//			case ASTNodeType::VARIABLE_CALL:
	//				PrintWithIndent( "VARIABLE CALL:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.VariableCall.Name );
	//				std::cout << "\n";
	//				break;
	//			case ASTNodeType::UNARY_OPERATOR:
	//				PrintWithIndent( "UNARY OPERATOR:\n", indent );
	//				PrintWithIndent( "Operator: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.UnaryOperator.Operator );
	//				std::cout << "\n";
	//				PrintWithIndent( "Operand:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.UnaryOperator.Operand, indent + 2 );
	//				break;
	//			case ASTNodeType::BINARY_OPERATOR:
	//				PrintWithIndent( "BINARY OPERATOR:\n", indent );
	//				PrintWithIndent( "Operator: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.BinaryOperator.Operator );
	//				std::cout << "\n";
	//				PrintWithIndent( "First Operand:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.BinaryOperator.FirstOperand, indent + 2 );
	//				PrintWithIndent( "Second Operand:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.BinaryOperator.SecondOperand, indent + 2 );
	//				break;
	//			case ASTNodeType::TERNARY_OPERATOR:
	//				PrintWithIndent( "TERNARY OPERATOR:\n", indent );
	//				PrintWithIndent( "Operator: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.TernaryOperator.Operator );
	//				std::cout << "\n";
	//				PrintWithIndent( "First Operand:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.TernaryOperator.FirstOperand, indent + 2 );
	//				PrintWithIndent( "Second Operand:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.TernaryOperator.SecondOperand, indent + 2 );
	//				PrintWithIndent( "Third Operand:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.TernaryOperator.ThirdOperand, indent + 2 );
	//				break;
	//			case ASTNodeType::MEMBER_CALL:
	//				PrintWithIndent( "MEMBER CALL:\n", indent );
	//				PrintWithIndent( "Parent:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.MemberCall.Parent, indent + 2 );
	//				PrintWithIndent( "Member:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.MemberCall.Member, indent + 2 );
	//				break;
	//			case ASTNodeType::RETURN_CALL:
	//				PrintWithIndent( "RETURN CALL:\n", indent );
	//				PrintWithIndent( "Value:\n", indent + 1 );
	//				if( node.ReturnCall.Value != 0 )
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.ReturnCall.Value, indent + 2 );
	//				break;
	//			case ASTNodeType::IF_BLOCK:
	//				PrintWithIndent( "IF BLOCK:\n", indent );
	//				PrintWithIndent( "Condition:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.ConditionBlock.Condition, indent + 2 );
	//				PrintWithIndent( "If Block:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.ConditionBlock.IfBlock, indent + 2 );
	//				if( node.ConditionBlock.ElseBlock != 0 )
	//				{
	//					PrintWithIndent( "Else Block:\n", indent + 1 );
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.ConditionBlock.ElseBlock, indent + 2 );
	//				}
	//				break;
	//			case ASTNodeType::WHILE_LOOP:
	//				PrintWithIndent( "WHILE LOOP:\n", indent );
	//				PrintWithIndent( "Condition:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.WhileLoop.Condition, indent + 2 );
	//				PrintWithIndent( "Block:\n", indent + 1 );
	//				PrintASTNode( file_data, tokens, ast, analyzis_data, node.WhileLoop.Block, indent + 2 );
	//				break;
	//			case ASTNodeType::STRUCT:
	//				PrintWithIndent( "STRUCT:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.Struct.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Members:\n", indent + 1 );
	//				if( node.Struct.FirstMember != 0 )
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.Struct.FirstMember, indent + 2 );
	//				break;
	//			case ASTNodeType::ENUM:
	//				PrintWithIndent( "ENUM:\n", indent );
	//				PrintWithIndent( "Name: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.Enum.Name );
	//				std::cout << "\n";
	//				PrintWithIndent( "Values:\n", indent + 1 );
	//				if( node.Enum.FirstValue != 0 )
	//					PrintASTNode( file_data, tokens, ast, analyzis_data, node.Enum.FirstValue, indent + 2 );
	//				break;
	//			case ASTNodeType::ENUM_VALUE:
	//				PrintWithIndent( "ENUM VALUE:\n", indent );
	//				PrintWithIndent( "Value: ", indent + 1 );
	//				PrintTokenValue( file_data, tokens, node.EnumValue.Value );
	//				std::cout << "\n";
	//				break;
	//			default:
	//				PrintWithIndent( l_astNames[(uint32_t)node.Type], indent );
	//				std::cout << "\n";
	//				break;
	//		}

	//		// Chaînage générique, utilisé pour la liste d'arguments, d'instructions, etc.
	//		if( node.NextNode != 0 )
	//			PrintASTNode( file_data, tokens, ast, analyzis_data, node.NextNode, indent );
	//	}
	//}

	////----------------------------------------
	//void PrintToken( const FileData& file_data, const TokenCollection& tokens, token_index token_cursor )
	//{
	//	// Note : ce PrintToken est pensé pour avoir une correspondance avec les métriques de Notepad++, d'où le fait de démarrer le compte à 1, de rajouter les carriage return et de compter 4 pour les tabs
	//
	//	const Token& token = tokens.Tokens[token_cursor];
	//
	//	uint32_t linePos = 1;
	//	uint32_t charPos = 1;
	//	uint32_t i = 0;
	//
	//	while( i < token.Position )
	//	{
	//		if( file_data.Buffer[i] == '\n' )
	//		{
	//			++linePos;
	//			charPos = 1;
	//		}
	//		else if( file_data.Buffer[i] == '\t' )
	//			charPos += 4;							// Pour visualiser correctement dans Notepad++, il faut comptabiliser les colonnes et pas les caractères
	//		else if( file_data.Buffer[i] != '\r' )
	//			++charPos;
	//
	//		++i;
	//	}
	//
	//	// fopen_s supprime les carriage return (et ça ne semble pas paramétrable)
	//	// En conséquence, la position du token est correcte vis à vis du buffer de travail, mais pas vis à vis du texte brut
	//	// Si on veut correctement identifier la position dans le fichier texte pour diagnostiquer, il faut rajouter le nombre de carriage return supprimés
	//	//	> Sachant que cette approche suppose que le fichier texte initial a bien été produit avec des carriages return, ce qui est très probable mais non garanti
	//	uint32_t carriageReturnAwarePosition = token.Position + linePos;
	//
	//	std::cout << "[" << l_tokenNames[(uint32_t) token.Type] << "] @" << carriageReturnAwarePosition << " (line " << linePos << " char " << charPos << ")";
	//}

	////----------------------------------------
	//void PrintAST( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data)
	//{
	//	PrintASTNode( file_data, tokens, ast, analyzis_data, 0, 0 );
	//}

#pragma endregion

#pragma endregion

#pragma region Semantic Analyzis

	//----------------------------------------
	const char* ReservedKeywords[] =
	{
		"void",

		"bool",
		"int",
		"float",
		"string",

		"var",
		"func",
		"branch"

		"if",
		"else",
		"while"
	};

	//----------------------------------------
	uint32_t ReservedKeywordCount = sizeof( ReservedKeywords ) / sizeof( ReservedKeywords[0] );

	////----------------------------------------
	//static bool IsTokenReservedKeyWord( const FileData& file_data, const TokenCollection& tokens, uint32_t token )
	//{
	//	for( uint32_t i = 0; i < ReservedKeywordCount; ++i )
	//	{
	//		if( IsTokenContentEquals( file_data, tokens, token, ReservedKeywords[i] ) )
	//			return true;
	//	}

	//	return false;
	//}

	////----------------------------------------
	//static bool AreTokenContentsEqual( const FileData& file_data, const TokenCollection& tokens, uint32_t first_token, uint32_t second_token )
	//{
	//	if( first_token >= tokens.TokenCount - 1 )
	//		return false;
	//	if( second_token >= tokens.TokenCount - 1 )
	//		return false;

	//	if( first_token == second_token )
	//		return true;

	//	const Token& firstToken = tokens.Tokens[first_token];
	//	const Token& nextFirstToken = tokens.Tokens[first_token + 1];

	//	const Token& secondToken = tokens.Tokens[second_token];
	//	const Token& nextSecondToken = tokens.Tokens[second_token + 1];

	//	uint32_t tokenFirstLength = nextFirstToken.Position - firstToken.Position;
	//	uint32_t tokenSecondLength = nextSecondToken.Position - secondToken.Position;

	//	if( tokenFirstLength != tokenSecondLength )
	//		return false;

	//	uint32_t readCursor = 0;
	//	while( readCursor < tokenFirstLength )
	//	{
	//		if( file_data.Buffer[firstToken.Position + readCursor] != file_data.Buffer[secondToken.Position + readCursor] )
	//			return false;

	//		++readCursor;
	//	}

	//	return true;
	//}

	////----------------------------------------
	//static bool IsNameAlreadyUsed( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, token_index name )
	//{
	//	// On teste les noms réservés
	//	if( IsTokenReservedKeyWord( file_data, tokens, name ) )
	//		return true;

	//	// Définition de types
	//	for( uint32_t i = 0; i < analyzis_data.TypeDefinitionCount; ++i )
	//	{
	//		const UserTypeDefinition& typeDef = analyzis_data.TypeDefinitions[i];

	//		if( AreTokenContentsEqual( file_data, tokens, ast.Nodes[typeDef.Node].Struct.Name, name ) )
	//			return true;
	//	}

	//	// Définition de fonctions
	//	for( uint32_t i = 0; i < analyzis_data.FunctionDefinitionCount; ++i )
	//	{
	//		ast_node_index func = analyzis_data.FunctionDefinitions[i];

	//		if( AreTokenContentsEqual( file_data, tokens, ast.Nodes[func].Function.Name, name ) )
	//			return true;
	//	}

	//	// #TODO : définition de variables (dans le scope courant et les scopes supérieurs)

	//	return false;
	//}

	////----------------------------------------
	//static bool GetUserDefinedTypesAndFunctionDefinitions( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data )
	//{
	//	for( ast_node_index i = 0; i < ast.NodeCount; ++i )
	//	{
	//		const ASTNode& node = ast.Nodes[i];

	//		if( node.Type == ASTNodeType::STRUCT )
	//		{
	//			if( IsNameAlreadyUsed( file_data, tokens, ast, analyzis_data, node.Struct.Name ) )
	//				return false;
	//			else
	//			{
	//				UserTypeDefinition& typeDef = analyzis_data.TypeDefinitions[analyzis_data.TypeDefinitionCount];
	//				typeDef.TypeId = analyzis_data.TypeDefinitionCount;
	//				typeDef.Node = i;
	//				++analyzis_data.TypeDefinitionCount;
	//			}
	//		}
	//		if( node.Type == ASTNodeType::FUNCTION )
	//		{
	//			if( IsNameAlreadyUsed( file_data, tokens, ast, analyzis_data, node.Function.Name ) )
	//				return false;
	//			else
	//			{
	//				analyzis_data.FunctionDefinitions[analyzis_data.FunctionDefinitionCount] = i;
	//				++analyzis_data.FunctionDefinitionCount;
	//			}
	//		}
	//	}

	//	return true;
	//}

	////----------------------------------------
	//static bool IsFunctionNameBuiltIn( const FileData& file_data, const TokenCollection& tokens, token_index function_name, RuntimeType& out_runtime_type )
	//{
	//	if( IsTokenContentEquals( file_data, tokens, function_name, "print" ) )
	//	{
	//		out_runtime_type.Type = RuntimeTypeType::VOID;
	//		return true;
	//	}

	//	return false;
	//}

	////----------------------------------------
	//static bool GetFunctionDefinitionNode( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, token_index function_name, ast_node_index& out_function_node_index )
	//{
	//	for( uint32_t i = 0; i < analyzis_data.FunctionDefinitionCount; ++i )
	//	{
	//		ast_node_index functionNodeIndexCandidate = analyzis_data.FunctionDefinitions[i];
	//		const ASTNode& functionNodeCandidate = ast.Nodes[functionNodeIndexCandidate];
	//		if( AreTokenContentsEqual( file_data, tokens, function_name, functionNodeCandidate.Function.Name ) )
	//		{
	//			out_function_node_index = functionNodeIndexCandidate;
	//			return true;
	//		}
	//	}

	//	return false;
	//}

	////----------------------------------------
	//static bool GetRuntimeTypeFromTypeName( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, token_index type_name, RuntimeType& out_runtime_type )
	//{
	//	if( IsTokenContentEquals( file_data, tokens, type_name, "void" ) )
	//	{
	//		out_runtime_type.Type = RuntimeTypeType::VOID;
	//		return true;
	//	}
	//	else if( IsTokenContentEquals( file_data, tokens, type_name, "bool" ) )
	//	{
	//		out_runtime_type.Type = RuntimeTypeType::BOOL;
	//		return true;
	//	}
	//	else if( IsTokenContentEquals( file_data, tokens, type_name, "int" ) )
	//	{
	//		out_runtime_type.Type = RuntimeTypeType::INT;
	//		return true;
	//	}
	//	else if( IsTokenContentEquals( file_data, tokens, type_name, "float" ) )
	//	{
	//		out_runtime_type.Type = RuntimeTypeType::FLOAT;
	//		return true;
	//	}
	//	else
	//	{
	//		for( uint32_t i = 0; i < analyzis_data.TypeDefinitionCount; ++i )
	//		{
	//			const UserTypeDefinition& typeDef = analyzis_data.TypeDefinitions[i];
	//			const ASTNode& typeNode = ast.Nodes[typeDef.Node];
	//			if( AreTokenContentsEqual( file_data, tokens, typeNode.Struct.Name, type_name ) )
	//			{
	//				out_runtime_type.Type = RuntimeTypeType::USER_DEFINED;
	//				out_runtime_type.UserDefinedType = typeDef.TypeId;
	//				return true;
	//			}
	//		}
	//	}

	//	return false;
	//}

	////----------------------------------------
	//static bool GetRuntimeTypeFromFunctionName( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, token_index function_name, RuntimeType& out_runtime_type )
	//{
	//	if( IsFunctionNameBuiltIn( file_data, tokens, function_name, out_runtime_type ) )
	//		return true;

	//	ast_node_index functionNodeIndex;

	//	if( !GetFunctionDefinitionNode( file_data, tokens, ast, analyzis_data, function_name, functionNodeIndex ) )
	//		return false;

	//	return GetRuntimeTypeFromTypeName( file_data, tokens, ast, analyzis_data, ast.Nodes[functionNodeIndex].Function.ReturnType, out_runtime_type );
	//}

	////----------------------------------------
	//static bool GetRuntimeTypeFromFunctionNode( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, ast_node_index function_node, RuntimeType& out_runtime_type )
	//{
	//	return GetRuntimeTypeFromTypeName( file_data, tokens, ast, analyzis_data, ast.Nodes[function_node].Function.ReturnType, out_runtime_type );
	//}

	////----------------------------------------
	//// Sémantique :
	//// var test:type;
	//// var test2:casting_type = (casting_type)test;
	//static bool IsCastableTo( const RuntimeType& type, const RuntimeType& casting_type )
	//{
	//	if( type.Type == casting_type.Type )
	//	{
	//		if( type.Type == RuntimeTypeType::USER_DEFINED )
	//			return type.UserDefinedType == casting_type.UserDefinedType;
	//		else
	//			return true;
	//	}

	//	if( type.Type == RuntimeTypeType::INT || casting_type.Type == RuntimeTypeType::FLOAT )
	//		return true;

	//	if( type.Type == RuntimeTypeType::FLOAT || casting_type.Type == RuntimeTypeType::INT )
	//		return true;

	//	return false;
	//}

	////----------------------------------------
	//static bool GetVariableDeclaration( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, ast_node_index from_node, ast_node_index& out_declaration_node )
	//{
	//	// 0 : #NOTE : on cherche la première déclaration. S'il y en a plusieurs, ce sera détecté par l'utilisation de cette fonction lorsqu'on rencontre une déclaration pour identifier s'il y en a eu une avant

	//	// 1 : Scopes locaux à la fonction, on remonte l'arbre ast depuis le node courant jusqu'au node de définition de la fonction courante (ou le début du fichier)

	//	// 2 : Scope global, on descend l'arbre ast depuis le début sans rentrer dans les déclarations de fonction
	//}

	////----------------------------------------
	//static bool AnalyzeNode( const FileData& file_data, const TokenCollection& tokens, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data, ast_node_index cursor )
	//{
	//	ASTNode& node = ast.Nodes[cursor];

	//	switch( node.Type )
	//	{
	//		case ASTNodeType::CONSTANT_BOOL:
	//		{
	//			node.RuntimeType.Type = RuntimeTypeType::BOOL;
	//			return true;
	//		}
	//		case ASTNodeType::CONSTANT_INT:
	//		{
	//			node.RuntimeType.Type = RuntimeTypeType::INT;
	//			return true;
	//		}
	//		case ASTNodeType::CONSTANT_FLOAT:
	//		{
	//			node.RuntimeType.Type = RuntimeTypeType::FLOAT;
	//			return true;
	//		}
	//		case ASTNodeType::UNARY_OPERATOR:
	//		{
	//			if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, node.UnaryOperator.Operand ) )
	//				return false;
	//			node.RuntimeType.Type = ast.Nodes[node.UnaryOperator.Operand].RuntimeType.Type;
	//			// #TODO : si on promeut les opérateurs comme noeuds explicites (on aurait un ASTNodeType::NEGATE_NUMBER et ASTNodeType::NEGATE_BOOL par exemple) on pourra être plus strict sur la correspondance de type
	//			return
	//				node.RuntimeType.Type == RuntimeTypeType::BOOL ||
	//				node.RuntimeType.Type == RuntimeTypeType::FLOAT ||
	//				node.RuntimeType.Type == RuntimeTypeType::INT;
	//		}
	//		case ASTNodeType::BINARY_OPERATOR:
	//		{
	//			if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, node.BinaryOperator.FirstOperand ) )
	//				return false;
	//			if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, node.BinaryOperator.SecondOperand ) )
	//				return false;

	//			RuntimeTypeType firstType = ast.Nodes[node.BinaryOperator.FirstOperand].RuntimeType.Type;
	//			RuntimeTypeType secondType = ast.Nodes[node.BinaryOperator.SecondOperand].RuntimeType.Type;

	//			if( firstType == secondType )
	//			{
	//				node.RuntimeType.Type = firstType;
	//				return true;
	//			}
	//			else if( firstType == RuntimeTypeType::INT && secondType == RuntimeTypeType::FLOAT )
	//			{
	//				node.RuntimeType.Type = RuntimeTypeType::FLOAT;
	//				return true;
	//			}
	//			else if( firstType == RuntimeTypeType::FLOAT && secondType == RuntimeTypeType::INT )
	//			{
	//				node.RuntimeType.Type = RuntimeTypeType::FLOAT;
	//				return true;
	//			}
	//			else
	//				return false;	// En l'état, aucune des combinaisons qui reste n'est valide
	//		}
	//		case ASTNodeType::SCOPE:
	//		{
	//			// Un scope n'a pas de type
	//			node.RuntimeType.Type = RuntimeTypeType::NONE;

	//			ast_node_index instructionIndex = node.Scope.FirstInstruction;
	//			while( instructionIndex != 0 )
	//			{
	//				if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, instructionIndex ) )
	//					return false;

	//				instructionIndex = ast.Nodes[instructionIndex].NextNode;
	//			}

	//			return true;
	//		}
	//		case ASTNodeType::FUNCTION:
	//		{
	//			// Pas forcément besoin de ça, mais ça consolide les infos et ça facilite le debug
	//			if( !GetRuntimeTypeFromTypeName( file_data, tokens, ast, analyzis_data, node.Function.ReturnType, node.RuntimeType ) )
	//				return false;

	//			analyzis_data.CurrentFunctionDefinition = cursor;

	//			ast_node_index argumentNodeIndex = node.Function.FirstArgument;
	//			while( argumentNodeIndex != 0 )
	//			{
	//				if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, argumentNodeIndex ) )
	//					return false;

	//				argumentNodeIndex = ast.Nodes[argumentNodeIndex].NextNode;
	//			}

	//			if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, node.Function.Body ) )
	//				return false;

	//			analyzis_data.CurrentFunctionDefinition = -1;

	//			return true;
	//		}
	//		case ASTNodeType::FUNCTION_CALL:
	//		{
	//			if( !GetRuntimeTypeFromFunctionName( file_data, tokens, ast, analyzis_data, node.FunctionCall.Name, node.RuntimeType ) )
	//				return false;

	//			ast_node_index argumentNodeIndex = node.FunctionCall.FirstArgument;
	//			while( argumentNodeIndex != 0 )
	//			{
	//				if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, argumentNodeIndex ) )
	//					return false;

	//				argumentNodeIndex = ast.Nodes[argumentNodeIndex].NextNode;
	//			}

	//			return true;
	//		}
	//		case ASTNodeType::STRUCT:
	//		{
	//			ast_node_index memberIndex = node.Struct.FirstMember;

	//			while( memberIndex != 0 )
	//			{
	//				if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, memberIndex ) )
	//					return false;

	//				memberIndex = ast.Nodes[memberIndex].NextNode;
	//			}
	//			
	//			return true;
	//		}
	//		case ASTNodeType::RETURN_CALL:
	//		{
	//			if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, node.ReturnCall.Value ) )
	//				return false;

	//			if( !GetRuntimeTypeFromFunctionNode( file_data, tokens, ast, analyzis_data, analyzis_data.CurrentFunctionDefinition, node.RuntimeType ) )
	//				return false;

	//			// On vérifie que le type de la fonction correspond au type de l'expression à retourner
	//			if( !IsCastableTo( ast.Nodes[node.ReturnCall.Value].RuntimeType, node.RuntimeType ) )
	//				return false;

	//			return true;
	//		}
	//		default:
	//			return false;
	//	}
	//}

	////----------------------------------------
	//bool AnalyzeAST( const FileData& file_data, const TokenCollection& tokens, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data )
	//{
	//	// #TODO :
	//	//	* Répertorier les types utilisateurs
	//	//	* Répertorier les définitions de fonctions (pour connaître le type de retour qui est nécessaire à la résolution du type FUNCTION_CALL)
	//	//	* Identifier les types pour chaque noeud
	//	//	* Résoudre les incohérences de types

	//	if( !GetUserDefinedTypesAndFunctionDefinitions( file_data, tokens, ast, analyzis_data ) )
	//		return false;

	//	ast_node_index cursor = 0;

	//	do
	//	{
	//		if( !AnalyzeNode( file_data, tokens, ast, analyzis_data, cursor ) )
	//			return false;

	//		cursor = ast.Nodes[cursor].NextNode;

	//	} while( cursor != 0 );

	//	return true;
	//}

	////----------------------------------------
	//void PrintAnalyzis( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data )
	//{
	//	std::cout << "User defined types:\n";

	//	for( uint32_t i = 0; i < analyzis_data.TypeDefinitionCount; ++i )
	//	{
	//		const UserTypeDefinition& typeDef = analyzis_data.TypeDefinitions[i];

	//		std::cout << "Type " << typeDef.TypeId << ": ";
	//		PrintTokenValue( file_data, tokens, ast.Nodes[typeDef.Node].Function.Name );
	//		std::cout << "\n";
	//	}
	//}

#pragma endregion

#pragma region Virtual Machine

	// #TODO :
	//	* Passer en "stack based" et plus "register based" : on va chercher les variables sur la stack
	//		> Certaines opérations utiliseront des paramètres mais ils seront modélisés dans un ou plusieurs opérateurs suivants (exemple : PUSH_CONSTANT_TO_STACK)
	//		> Attention : nos opérateurs vont être sur 8 bit, on devra donc interpréter plusieurs opérateurs d'un coup pour par exemple obtenir un int32_t. Est-ce qu'on ne va pas avoir des problèmes d'alignement mémoire ?
	//	* Créer un buffer de constantes qui stockera les valeurs plus grande qu'une adresse mémoire (comme une string)
	//		> Ca supposera problablement d'avoir des opérateurs plus spécifiques comme PUSH_CONSTANT_UINT_TO_STACK qui récupère directement à la suite la valeur de l'entier et PUSH_CONSTANT_STRING_TO_STACK qui récupère l'adresse
	//		  dans les opérateurs suivants
	//		> Retirer le store block qui est un cul de sac d'analyse : les instructions n'ont pas à "stocker" des données décrites dans les instructions dans un espace mémoire pour les utiliser,
	//		  mais ces données sont directement stockées dans le buffer des constantes et adressées directement là bas
	//		> En conséquence on aura un PUSH_XXX_TO_STACK qui va aller chercher dans la mémoire de travail dynamique
	//			> Mais là, est-ce qu'on en a 2 (la stack au sens de la call-stack et la heap) ou 1 (la heap, et la stack est celle utilisée pour l'exécution des opérations)
	//	* Enjeux d'optimisation :
	//		> Pour les constantes de string, si 2 string on la même valeur dans le code source, il est important de ne le stocker qu'une seule fois dans le buffer de constantes
	//		> Et ça permettra d'optimiser certaines opérations de comparaison ... mais je m'avance, j'en suis loin

	// #TODO :
	//	* Implémenter le mécanisme de retour de fonction

	////----------------------------------------
	//static void EmitOperation( CompiledProgram& program, OpCode op_code )
	//{
	//	program.Ops[program.OpCount] = op_code;
	//	program.OpCount++;
	//}

	////----------------------------------------
	//// Sémantique :
	//// var test:type;
	//// var test2:casting_type = (casting_type)test;
	//static bool EmitCast( CompiledProgram& program, const RuntimeType& type, const RuntimeType& casting_type )
	//{
	//	if( type.Type == casting_type.Type )
	//	{
	//		if( type.Type == RuntimeTypeType::USER_DEFINED )
	//			return type.UserDefinedType == casting_type.UserDefinedType;
	//		else
	//			return true;
	//	}

	//	if( type.Type == RuntimeTypeType::INT || casting_type.Type == RuntimeTypeType::FLOAT )
	//	{
	//		EmitOperation( program, OpCode::CAST_INT_TO_FLOAT );
	//		return true;
	//	}

	//	if( type.Type == RuntimeTypeType::FLOAT || casting_type.Type == RuntimeTypeType::INT )
	//	{
	//		EmitOperation( program, OpCode::CAST_FLOAT_TO_INT );
	//		return true;
	//	}

	//	return false;
	//}

	////----------------------------------------
	//static void EmitPushConstant( CompiledProgram& program, int32_t constant_value )
	//{
	//	MemoryBlock mb;
	//	mb.i32 = constant_value;

	//	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_INT;
	//	program.Ops[program.OpCount + 1] = (OpCode)mb.store[0];
	//	program.Ops[program.OpCount + 2] = (OpCode)mb.store[1];
	//	program.Ops[program.OpCount + 3] = (OpCode)mb.store[2];
	//	program.Ops[program.OpCount + 4] = (OpCode)mb.store[3];

	//	program.OpCount += 5;
	//}

	////----------------------------------------
	//static void EmitPushConstant( CompiledProgram& program, float constant_value )
	//{
	//	MemoryBlock mb;
	//	mb.f32 = constant_value;

	//	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_FLOAT;
	//	program.Ops[program.OpCount + 1] = (OpCode) mb.store[0];
	//	program.Ops[program.OpCount + 2] = (OpCode) mb.store[1];
	//	program.Ops[program.OpCount + 3] = (OpCode) mb.store[2];
	//	program.Ops[program.OpCount + 4] = (OpCode) mb.store[3];

	//	program.OpCount += 5;
	//}

	////----------------------------------------
	//static void EmitPushConstant( CompiledProgram& program, bool constant_value )
	//{
	//	program.Ops[program.OpCount] = OpCode::PUSH_CONSTANT_BOOL;
	//	program.Ops[program.OpCount + 1] = (OpCode)(constant_value ? (uint8_t)1 : (uint8_t)0);

	//	program.OpCount += 2;
	//}

	////----------------------------------------
	//static void EmitCallFunction( CompiledProgram& program, uint32_t function_first_operation )
	//{
	//	MemoryBlock mb;
	//	mb.i32 = function_first_operation;

	//	program.Ops[program.OpCount] = OpCode::CALL_FUNCTION;
	//	program.Ops[program.OpCount + 1] = (OpCode) mb.store[0];
	//	program.Ops[program.OpCount + 2] = (OpCode) mb.store[1];
	//	program.Ops[program.OpCount + 3] = (OpCode) mb.store[2];
	//	program.Ops[program.OpCount + 4] = (OpCode) mb.store[3];

	//	program.OpCount += 5;
	//}

	////----------------------------------------
	//static void ReplaceCallFunction( CompiledProgram& program, uint32_t function_first_operation, uint32_t cursor )
	//{
	//	MemoryBlock mb;
	//	mb.i32 = function_first_operation;

	//	if( program.Ops[cursor] != OpCode::CALL_FUNCTION )
	//		return;

	//	program.Ops[cursor + 1] = (OpCode) mb.store[0];
	//	program.Ops[cursor + 2] = (OpCode) mb.store[1];
	//	program.Ops[cursor + 3] = (OpCode) mb.store[2];
	//	program.Ops[cursor + 4] = (OpCode) mb.store[3];
	//}

	//// #NOTE : c'est un test pour commencer à avoir des informations de type pour faire des choix dans l'exécution
	////	* Une idée alternative consisterait à typer les éléments de la stack et à se servir de ces types pour inférer le contexte
	////----------------------------------------
	//enum class TypeContext
	//{
	//	NONE,

	//	FLOAT,
	//	INT,
	//	BOOL
	//};

	////----------------------------------------
	//static bool AddFunctionReference( CompilationData& compilation_data, token_index name, uint32_t first_operation )
	//{
	//	// #TODO : soit on teste l'unicité de nom ici, soit on le fait dans une passe d'analyse précédente. Je pencherais pour la seconde approche
	//	if( compilation_data.FunctionRefCount >= 127 )
	//	{
	//		std::cout << "[ERROR] Too many functions!\n";
	//		return false;
	//	}

	//	FunctionDefRef& funcRef = compilation_data.FunctionRefs[compilation_data.FunctionRefCount];
	//	funcRef.Name = name;
	//	funcRef.FirstOperation = first_operation;
	//	++compilation_data.FunctionRefCount;
	//	
	//	return true;
	//}

	////----------------------------------------
	//static bool GetFunctionReference( const CompilationData& compilation_data, const FileData& file_data, const TokenCollection& tokens, token_index name, uint32_t& out_first_operation )
	//{
	//	for( uint32_t i = 0; i < compilation_data.FunctionRefCount; ++i )
	//	{
	//		const FunctionDefRef& funcRef = compilation_data.FunctionRefs[i];

	//		if( AreTokenContentsEqual(file_data, tokens, funcRef.Name, name ) )
	//		{
	//			out_first_operation = funcRef.FirstOperation;
	//			return true;
	//		}
	//	}

	//	std::cout << "[ERROR] Function ";
	//	PrintTokenValue( file_data, tokens, name );
	//	std::cout << " not found\n";

	//	return false;
	//}

	////----------------------------------------
	//static void PrintFunctionNameFromFirstOperation( const CompilationData& compilation_data, const FileData& file_data, const TokenCollection& tokens, uint32_t first_operation )
	//{
	//	for( uint32_t i = 0; i < compilation_data.FunctionRefCount; ++i )
	//	{
	//		const FunctionDefRef& funcRef = compilation_data.FunctionRefs[i];

	//		if( funcRef.FirstOperation == first_operation )
	//			PrintTokenValue( file_data, tokens, funcRef.Name );
	//	}
	//}

	////----------------------------------------
	//static bool CompileNode( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, ast_node_index cursor, CompilationData& compilation_data, CompiledProgram& program, TypeContext& out_type_context )
	//{
	//	out_type_context = TypeContext::NONE;

	//	const ASTNode& node = ast.Nodes[cursor];

	//	switch( node.Type )
	//	{
	//		case ASTNodeType::FUNCTION:
	//		{
	//			if( IsTokenContentEquals( file_data, tokens, node.Function.Name, "main" ) )
	//				compilation_data.MainNameToken = node.Function.Name;

	//			AddFunctionReference( compilation_data, node.Function.Name, program.OpCount );

	//			if( !CompileNode( file_data, tokens, ast, node.Function.Body, compilation_data, program, out_type_context ) )
	//				return false;

	//			EmitOperation( program, OpCode::EXIT_FUNCTION );

	//			out_type_context = TypeContext::NONE;
	//			return true;
	//		}
	//		case ASTNodeType::FUNCTION_CALL:
	//		{
	//			// Function built-in
	//			// #TODO : généraliser avec un catalogue de fonctions externes
	//			if( IsTokenContentEquals( file_data, tokens, node.FunctionCall.Name, "print" ) )
	//			{
	//				if( node.FunctionCall.FirstArgument == 0 )
	//					return false;

	//				const ASTNode& functionFirstArgNode = ast.Nodes[node.FunctionCall.FirstArgument];

	//				if( functionFirstArgNode.NextNode != 0 )	// Pas le droit à plus d'1 argument
	//					return false;

	//				if( !CompileNode( file_data, tokens, ast, node.FunctionCall.FirstArgument, compilation_data, program, out_type_context ) )
	//					return false;

	//				// #NOTE : dans cette version, c'est la fonction appelée (comme l'opérateur d'ailleurs) qui a pour responsabilité de gérer la stack. En l'occurence ici on consomme la variable donnée à afficher
	//				switch( out_type_context )
	//				{
	//					case TypeContext::FLOAT:	EmitOperation( program, OpCode::DEBUG_PRINT_FLOAT );	EmitOperation( program, OpCode::POP ); break;
	//					case TypeContext::INT:		EmitOperation( program, OpCode::DEBUG_PRINT_INT );		EmitOperation( program, OpCode::POP ); break;
	//					case TypeContext::BOOL:		EmitOperation( program, OpCode::DEBUG_PRINT_BOOL );		EmitOperation( program, OpCode::POP ); break;
	//					default: return false;
	//				}

	//				out_type_context = TypeContext::NONE;
	//				return true;
	//			}
	//			else
	//			{
	//				EmitCallFunction( program, node.FunctionCall.Name );

	//				// On récupère le type de la fonction
	//				out_type_context = 
	//					node.RuntimeType.Type == RuntimeTypeType::BOOL ? TypeContext::BOOL :
	//					node.RuntimeType.Type == RuntimeTypeType::INT ? TypeContext::INT :
	//					node.RuntimeType.Type == RuntimeTypeType::FLOAT ? TypeContext::FLOAT :
	//					TypeContext::NONE;	

	//				// #TODO : ATTENTION, si cet appel n'est pas "utilisé" et que la fonction renvoie une valeur, il faut pop cette valeur !

	//				return true;
	//			}
	//		}
	//		case ASTNodeType::SCOPE:
	//		{
	//			ast_node_index currentInstruction = node.Scope.FirstInstruction;

	//			while( currentInstruction != 0 )
	//			{
	//				if( !CompileNode( file_data, tokens, ast, currentInstruction, compilation_data, program, out_type_context ) )
	//					return false;

	//				currentInstruction = ast.Nodes[currentInstruction].NextNode;
	//			}

	//			out_type_context = TypeContext::NONE;
	//			return true;
	//		}
	//		case ASTNodeType::CONSTANT_FLOAT:
	//		{
	//			EmitPushConstant( program, node.ConstantFloat.Value );
	//			out_type_context = TypeContext::FLOAT;
	//			return true;
	//		}
	//		case ASTNodeType::CONSTANT_INT:
	//		{
	//			EmitPushConstant( program, node.ConstantInt.Value );
	//			out_type_context = TypeContext::INT;
	//			return true;
	//		}
	//		case ASTNodeType::CONSTANT_BOOL:
	//		{
	//			EmitPushConstant( program, node.ConstantBool.Value );
	//			out_type_context = TypeContext::BOOL;
	//			return true;
	//		}
	//		case ASTNodeType::UNARY_OPERATOR:
	//		{
	//			if(!CompileNode(file_data, tokens, ast, node.UnaryOperator.Operand, compilation_data, program, out_type_context) )
	//				return false;

	//			if( IsTokenContentEquals( file_data, tokens, node.UnaryOperator.Operator, "-" ) )
	//			{
	//				if( out_type_context == TypeContext::FLOAT )
	//				{
	//					EmitOperation( program, OpCode::NEGATE_FLOAT );
	//					return true;	// Le contexte reste le même
	//				}
	//				else if( out_type_context == TypeContext::INT )
	//				{
	//					EmitOperation( program, OpCode::NEGATE_INT );
	//					return true;	// Le contexte reste le même
	//				}
	//				else
	//					return false;
	//			}
	//			else if( IsTokenContentEquals( file_data, tokens, node.UnaryOperator.Operator, "!" ) )
	//			{
	//				if( out_type_context == TypeContext::BOOL )
	//				{
	//					EmitOperation( program, OpCode::NEGATE_BOOL );
	//					return true;	// Le contexte reste le même
	//				}
	//				else
	//					return false;
	//			}
	//			else
	//			{
	//				return false;
	//			}
	//		}
	//		case ASTNodeType::BINARY_OPERATOR:
	//		{
	//			TypeContext firstOperandType;
	//			if( !CompileNode( file_data, tokens, ast, node.BinaryOperator.FirstOperand, compilation_data, program, firstOperandType ) )
	//				return false;

	//			if(ast.Nodes[node.BinaryOperator.FirstOperand].RuntimeType.Type == RuntimeTypeType::INT && node.RuntimeType.Type == RuntimeTypeType::FLOAT )
	//				EmitOperation( program, OpCode::CAST_INT_TO_FLOAT );

	//			TypeContext secondOperandType;
	//			if( !CompileNode( file_data, tokens, ast, node.BinaryOperator.SecondOperand, compilation_data, program, secondOperandType ) )
	//				return false;

	//			if( ast.Nodes[node.BinaryOperator.SecondOperand].RuntimeType.Type == RuntimeTypeType::INT && node.RuntimeType.Type == RuntimeTypeType::FLOAT )
	//				EmitOperation( program, OpCode::CAST_INT_TO_FLOAT );

	//			bool addOp = IsTokenContentEquals( file_data, tokens, node.BinaryOperator.Operator, "+" );
	//			bool substractOp = IsTokenContentEquals( file_data, tokens, node.BinaryOperator.Operator, "-" );
	//			bool multiplyOp = IsTokenContentEquals( file_data, tokens, node.BinaryOperator.Operator, "*" );
	//			bool divideOp = IsTokenContentEquals( file_data, tokens, node.BinaryOperator.Operator, "/" );

	//			if( addOp || substractOp || multiplyOp || divideOp )
	//			{
	//				if( node.RuntimeType.Type == RuntimeTypeType::INT )
	//				{
	//					if(addOp)
	//						EmitOperation( program, OpCode::ADD_INT );
	//					else if(substractOp )
	//						EmitOperation( program, OpCode::SUBSTRACT_INT );
	//					else if( multiplyOp )
	//						EmitOperation( program, OpCode::MULTIPLY_INT );
	//					else if( divideOp )
	//						EmitOperation( program, OpCode::DIVIDE_INT );

	//					out_type_context = TypeContext::INT;
	//				}
	//				else
	//				{
	//					if( addOp )
	//						EmitOperation( program, OpCode::ADD_FLOAT );
	//					else if( substractOp )
	//						EmitOperation( program, OpCode::SUBSTRACT_FLOAT );
	//					else if( multiplyOp )
	//						EmitOperation( program, OpCode::MULTIPLY_FLOAT );
	//					else if( divideOp )
	//						EmitOperation( program, OpCode::DIVIDE_FLOAT );

	//					out_type_context = TypeContext::FLOAT;
	//				}

	//				return true;
	//			}
	//			else
	//				return false;
	//		}
	//		case ASTNodeType::STRUCT:
	//		{
	//			return true;	// Une structure n'a pas besoin d'être compilée
	//		}
	//		case ASTNodeType::RETURN_CALL:
	//		{
	//			if( !CompileNode( file_data, tokens, ast, node.ReturnCall.Value, compilation_data, program, out_type_context ) )
	//				return false;

	//			// Gestion du cast si possible
	//			if( IsCastableTo( ast.Nodes[node.ReturnCall.Value].RuntimeType, node.RuntimeType ) )
	//				EmitCast( program, ast.Nodes[node.ReturnCall.Value].RuntimeType, node.RuntimeType);
	//			else
	//				return false;

	//			EmitOperation( program, OpCode::EXIT_FUNCTION );

	//			return true;
	//		}
	//		default:
	//			return false;
	//	}
	//}

	////----------------------------------------
	//static bool ResolveFunctionReferences( const FileData& file_data, const TokenCollection& tokens, const CompilationData& compilation_data, CompiledProgram& program )
	//{
	//	uint32_t i = 0;

	//	while( i < program.OpCount )
	//	{
	//		OpCode op = program.Ops[i];

	//		if( op == OpCode::CALL_FUNCTION )
	//		{
	//			MemoryBlock mb;
	//			mb.store[0] = (uint8_t)program.Ops[i + 1];
	//			mb.store[1] = (uint8_t)program.Ops[i + 2];
	//			mb.store[2] = (uint8_t)program.Ops[i + 3];
	//			mb.store[3] = (uint8_t)program.Ops[i + 4];

	//			uint32_t opIndexUsedAsTokenIndex = mb.index;

	//			if( !GetFunctionReference( compilation_data, file_data, tokens, opIndexUsedAsTokenIndex, mb.index ) )
	//				return false;

	//			program.Ops[i + 1] = (OpCode)mb.store[0];
	//			program.Ops[i + 2] = (OpCode)mb.store[1];
	//			program.Ops[i + 3] = (OpCode)mb.store[2];
	//			program.Ops[i + 4] = (OpCode)mb.store[3];

	//			i += 5;
	//		}
	//		else if( op == OpCode::PUSH_CONSTANT_INT || op == OpCode::PUSH_CONSTANT_FLOAT )
	//		{
	//			i += 5;
	//		}
	//		else if( op == OpCode::PUSH_CONSTANT_BOOL )
	//		{
	//			i += 2;
	//		}
	//		else
	//		{
	//			++i;
	//		}
	//	}

	//	return true;
	//}

	////----------------------------------------
	//bool CompileAST( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, CompiledProgram& out_program, CompilationData& out_compilation_data )
	//{
	//	TypeContext typeContext;
	//	uint32_t cursor = 0;
	//
	//	// Reserving Entry Point
	//	EmitCallFunction( out_program, 0 );
	//	EmitOperation( out_program, OpCode::EXIT );
	//
	//	do
	//	{
	//		if( !CompileNode( file_data, tokens, ast, cursor, out_compilation_data, out_program, typeContext ) )
	//			return false;
	//
	//		cursor = ast.Nodes[cursor].NextNode;
	//	} while( cursor != 0 );
	//
	//	// Setting Entry Point
	//	ReplaceCallFunction( out_program, out_compilation_data.MainNameToken, 0 );
	//
	//	return ResolveFunctionReferences( file_data, tokens, out_compilation_data, out_program );
	//}

	////----------------------------------------
	//void PrintProgram( const CompiledProgram& program )
	//{
	//	uint32_t opCountToDisplayRaw = 0;

	//	for( uint32_t i = 0; i < program.OpCount; ++i )
	//	{
	//		std::cout << std::setw( 4 ) << i << std::setw( 0 ) << " ";

	//		if( opCountToDisplayRaw > 0 )
	//		{
	//			--opCountToDisplayRaw;
	//			std::cout << (uint32_t)program.Ops[i];
	//		}
	//		else
	//		{
	//			switch( program.Ops[i] )
	//			{
	//				case OpCode::CREATE_MEMORY:			std::cout << "CREATE_MEMORY";			opCountToDisplayRaw = 4;	break;
	//				case OpCode::PUSH_FROM_MEMORY:		std::cout << "PUSH_FROM_MEMORY";		opCountToDisplayRaw = 4;	break;
	//				case OpCode::POP_TO_MEMORY:			std::cout << "POP_TO_MEMORY";			opCountToDisplayRaw = 4;	break;
	//				case OpCode::PUSH_CONSTANT_INT:		std::cout << "PUSH_CONSTANT_INT";		opCountToDisplayRaw = 4;	break;
	//				case OpCode::PUSH_CONSTANT_FLOAT:	std::cout << "PUSH_CONSTANT_FLOAT";		opCountToDisplayRaw = 4;	break;
	//				case OpCode::PUSH_CONSTANT_BOOL:	std::cout << "PUSH_CONSTANT_BOOL";		opCountToDisplayRaw = 1;	break;
	//				case OpCode::POP:					std::cout << "POP";													break;
	//				case OpCode::NEGATE_FLOAT:			std::cout << "NEGATE_FLOAT";										break;
	//				case OpCode::NEGATE_INT:			std::cout << "NEGATE_INT";											break;
	//				case OpCode::NEGATE_BOOL:			std::cout << "NEGATE_BOOL";											break;
	//				case OpCode::CAST_INT_TO_FLOAT:		std::cout << "CAST_INT_TO_FLOAT";									break;
	//				case OpCode::CAST_FLOAT_TO_INT:		std::cout << "CAST_FLOAT_TO_INT";									break;
	//				case OpCode::ADD_INT:				std::cout << "ADD_INT";												break;
	//				case OpCode::ADD_FLOAT:				std::cout << "ADD_FLOAT";											break;
	//				case OpCode::SUBSTRACT_INT:			std::cout << "SUBSTRACT_INT";										break;
	//				case OpCode::SUBSTRACT_FLOAT:		std::cout << "SUBSTRACT_FLOAT";										break;
	//				case OpCode::MULTIPLY_INT:			std::cout << "MULTIPLY_INT";										break;
	//				case OpCode::MULTIPLY_FLOAT:		std::cout << "MULTIPLY_FLOAT";										break;
	//				case OpCode::DIVIDE_INT:			std::cout << "DIVIDE_INT";											break;
	//				case OpCode::DIVIDE_FLOAT:			std::cout << "DIVIDE_FLOAT";										break;
	//				case OpCode::CALL_FUNCTION:			std::cout << "CALL_FUNCTION";			opCountToDisplayRaw = 4;	break;
	//				case OpCode::EXIT_FUNCTION:			std::cout << "EXIT_FUNCTION";										break;
	//				case OpCode::DEBUG_PRINT_INT:		std::cout << "DEBUG_PRINT_INT";										break;
	//				case OpCode::DEBUG_PRINT_FLOAT:		std::cout << "DEBUG_PRINT_FLOAT";									break;
	//				case OpCode::DEBUG_PRINT_BOOL:		std::cout << "DEBUG_PRINT_BOOL";									break;
	//				case OpCode::EXIT:					std::cout << "EXIT";												break;
	//			}
	//		}

	//		std::cout << "\n";
	//	}
	//}

	////----------------------------------------
	//void PrintExecutionStack( const ExecutionContext& context )
	//{
	//	std::cout << "Execution Stack (" << context.ExecutionStackCursor << " memory blocks):\n\n";
	//
	//	std::cout << std::setw( 7 ) << "Pos |" << std::setw(23) << "Store |" << std::setw( 16 ) << "Index |" << std::setw(16) << "i32 |" << std::setw(16) << " f32 |" << std::setw(7) << "b\n";
	//	std::cout << "-------------------------------------------------------------------------------------\n";
	//
	//	for( uint32_t i = context.ExecutionStackCursor; i > 0; --i )
	//	{
	//		const MemoryBlock& mb = context.ExecutionStack[i - 1];
	//
	//		std::cout << std::setw( 5 ) << ( i - 1 ) << std::setw( 0 ) << " |";
	//		std::cout <<
	//			std::setw( 0 ) << " [" << std::setw( 3 ) << (uint32_t)mb.store[0] <<
	//			std::setw( 0 ) << "][" << std::setw( 3 ) << (uint32_t)mb.store[1] <<
	//			std::setw( 0 ) << "][" << std::setw( 3 ) << (uint32_t)mb.store[2] <<
	//			std::setw( 0 ) << "][" << std::setw( 3 ) << (uint32_t)mb.store[3] <<
	//			std::setw( 0 ) << "] |";
	//		std::cout << std::setw( 14 ) << mb.index << std::setw( 0 ) << " |";
	//		std::cout << std::setw( 14 ) << mb.i32 << std::setw( 0 ) << " |";
	//		std::cout << std::setw( 14 ) << mb.f32 << std::setw( 0 ) << " |";
	//		std::cout << std::setw( 6 ) << ( mb.b ? "true" : "false" ) << "\n";
	//	}
	//
	//	std::cout << std::setw( 0 ) << "\n";
	//}

	////----------------------------------------
	//void PrintCallStack( const FileData& file_data, const TokenCollection& tokens, const CompiledProgram& program, const ExecutionContext& context, const CompilationData& compilation_data )
	//{
	//	std::cout << "Call Stack (" << context.CallStackCursor << " frames):\n\n";

	//	for( uint32_t i = context.CallStackCursor; i > 0; --i )
	//	{
	//		const CallFrame& callFrame = context.CallStack[i - 1];

	//		std::cout << std::setw( 5 ) << ( i - 1 ) << std::setw( 5 ) << callFrame.CallSite << std::setw( 0 ) << " ";

	//		MemoryBlock mb;
	//		mb.store[0] = (uint8_t) program.Ops[callFrame.CallSite + 1];
	//		mb.store[1] = (uint8_t) program.Ops[callFrame.CallSite + 2];
	//		mb.store[2] = (uint8_t) program.Ops[callFrame.CallSite + 3];
	//		mb.store[3] = (uint8_t) program.Ops[callFrame.CallSite + 4];

	//		uint32_t firstFunctionOp = mb.index;

	//		std::cout << " ";
	//		PrintFunctionNameFromFirstOperation( compilation_data, file_data, tokens, firstFunctionOp );
	//		std::cout << "\n";

	//		std::cout << std::setw( 10 ) << callFrame.MemoryStackCursor << std::setw( 0 ) << "\n";
	//	}
	//}

	////----------------------------------------
	//bool ExecuteProgram( const CompiledProgram& program, ExecutionContext& context )
	//{
	//	uint32_t cursor = 0;

	//	// CallFrame par défaut
	//	CallFrame& callFrame = context.CallStack[context.CallStackCursor];
	//	callFrame.CallSite = cursor;
	//	callFrame.MemoryStackCursor = context.MemoryStackCursor;
	//	callFrame.NamedMemoryBaseIndex = context.CurrentMaxName;
	//	++context.CallStackCursor;

	//	while( cursor < program.OpCount )
	//	{
	//		switch( program.Ops[cursor] )
	//		{
	//			case OpCode::CREATE_MEMORY:
	//			{
	//				MemoryBlock varLocalAddress;
	//				varLocalAddress.store[0] = (uint8_t) program.Ops[cursor + 1];
	//				varLocalAddress.store[1] = (uint8_t) program.Ops[cursor + 2];
	//				varLocalAddress.store[2] = (uint8_t) program.Ops[cursor + 3];
	//				varLocalAddress.store[3] = (uint8_t) program.Ops[cursor + 4];

	//				uint32_t varLocalAddressIndex = context.CallStack[context.CallStackCursor - 1].NamedMemoryBaseIndex + varLocalAddress.index;

	//				context.MemoryStack[context.MemoryStackCursor].i32 = 0;	// init universel
	//				context.NamedMemoryMap[varLocalAddressIndex] = context.MemoryStackCursor;
	//				context.CurrentMaxName = std::max( context.CurrentMaxName, varLocalAddressIndex );
	//				context.MemoryStackCursor++;

	//				cursor += 5;
	//			}
	//			break;
	//			case OpCode::PUSH_FROM_MEMORY:
	//			{
	//				MemoryBlock varLocalAddress;
	//				varLocalAddress.store[0] = (uint8_t) program.Ops[cursor + 1];
	//				varLocalAddress.store[1] = (uint8_t) program.Ops[cursor + 2];
	//				varLocalAddress.store[2] = (uint8_t) program.Ops[cursor + 3];
	//				varLocalAddress.store[3] = (uint8_t) program.Ops[cursor + 4];

	//				uint32_t varLocalAddressIndex = context.CallStack[context.CallStackCursor - 1].NamedMemoryBaseIndex + varLocalAddress.index;

	//				MemoryBlock& memoryVar = context.MemoryStack[context.MemoryStackCursor];
	//				MemoryBlock& exeStackVar = context.ExecutionStack[context.ExecutionStackCursor];

	//				exeStackVar = memoryVar;

	//				context.ExecutionStackCursor++;
	//				cursor += 5;
	//			}
	//			break;
	//			case OpCode::POP_TO_MEMORY:
	//			{
	//				MemoryBlock varLocalAddress;
	//				varLocalAddress.store[0] = (uint8_t) program.Ops[cursor + 1];
	//				varLocalAddress.store[1] = (uint8_t) program.Ops[cursor + 2];
	//				varLocalAddress.store[2] = (uint8_t) program.Ops[cursor + 3];
	//				varLocalAddress.store[3] = (uint8_t) program.Ops[cursor + 4];

	//				uint32_t varLocalAddressIndex = context.CallStack[context.CallStackCursor - 1].NamedMemoryBaseIndex + varLocalAddress.index;

	//				MemoryBlock& exeStackVar = context.ExecutionStack[context.ExecutionStackCursor - 1];
	//				MemoryBlock& memoryVar = context.MemoryStack[context.MemoryStackCursor];

	//				memoryVar = exeStackVar;

	//				context.ExecutionStackCursor--;
	//				cursor += 5;
	//			}
	//			break;
	//			case OpCode::PUSH_CONSTANT_INT:
	//			case OpCode::PUSH_CONSTANT_FLOAT:
	//			{
	//				MemoryBlock& mb = context.ExecutionStack[context.ExecutionStackCursor];
	//				mb.store[0] = (uint8_t)program.Ops[cursor + 1];
	//				mb.store[1] = (uint8_t)program.Ops[cursor + 2];
	//				mb.store[2] = (uint8_t)program.Ops[cursor + 3];
	//				mb.store[3] = (uint8_t)program.Ops[cursor + 4];

	//				context.ExecutionStackCursor++;
	//				cursor += 5;
	//			}
	//			break;
	//			case OpCode::PUSH_CONSTANT_BOOL:
	//			{
	//				MemoryBlock& mb = context.ExecutionStack[context.ExecutionStackCursor];
	//				mb.b = (uint8_t)program.Ops[cursor + 1] != 0;

	//				context.ExecutionStackCursor++;
	//				cursor += 2;
	//			}
	//			break;
	//			case OpCode::POP:
	//			{
	//				context.ExecutionStackCursor--;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::NEGATE_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 1].f32 *= -1.0f;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::NEGATE_INT:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 1].i32 *= -1;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::NEGATE_BOOL:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 1].b ^= true;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::CAST_INT_TO_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 1].f32 = (float)context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::CAST_FLOAT_TO_INT:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 1].i32 = (int32_t)context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::ADD_INT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 += context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::ADD_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 += context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::SUBSTRACT_INT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 -= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::SUBSTRACT_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 -= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::MULTIPLY_INT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 *= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::MULTIPLY_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 *= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::DIVIDE_INT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 /= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::DIVIDE_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor < 2 )
	//				{
	//					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 /= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
	//				--context.ExecutionStackCursor;
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::CALL_FUNCTION:
	//			{
	//				if( context.CallStackCursor > 15 )
	//				{
	//					std::cout << "[ERROR] Stack overflow for operation " << cursor << "\n";
	//					return false;
	//				}

	//				CallFrame& callFrame = context.CallStack[context.CallStackCursor];
	//				callFrame.CallSite = cursor;
	//				callFrame.MemoryStackCursor = context.MemoryStackCursor;
	//				callFrame.NamedMemoryBaseIndex = context.CurrentMaxName;
	//				++context.CallStackCursor;

	//				MemoryBlock mb;
	//				mb.store[0] = (uint8_t) program.Ops[cursor + 1];
	//				mb.store[1] = (uint8_t) program.Ops[cursor + 2];
	//				mb.store[2] = (uint8_t) program.Ops[cursor + 3];
	//				mb.store[3] = (uint8_t) program.Ops[cursor + 4];

	//				cursor = mb.index;
	//			}
	//			break;
	//			case OpCode::EXIT_FUNCTION:
	//			{
	//				if( context.CallStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Stack underflow for operation " << cursor << "\n";
	//					return false;
	//				}

	//				--context.CallStackCursor;
	//				CallFrame& callFrame = context.CallStack[context.CallStackCursor];

	//				cursor = callFrame.CallSite + 5;	// +1 pour consommer l'opérateur d'appel de fonction, +4 pour les données d'index de curseur
	//				context.MemoryStackCursor = callFrame.MemoryStackCursor;
	//				context.CurrentMaxName = callFrame.NamedMemoryBaseIndex;
	//			}
	//			break;
	//			case OpCode::DEBUG_PRINT_INT:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//					return false;
	//				}
	//				std::cout << "DEBUG INT: " << context.ExecutionStack[context.ExecutionStackCursor - 1].i32 << "\n";
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::DEBUG_PRINT_FLOAT:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//						return false;
	//				}
	//				std::cout << "DEBUG FLOAT: " << context.ExecutionStack[context.ExecutionStackCursor - 1].f32 << "\n";
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::DEBUG_PRINT_BOOL:
	//			{
	//				if( context.ExecutionStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
	//						return false;
	//				}
	//				std::cout << "DEBUG BOOL: " << (context.ExecutionStack[context.ExecutionStackCursor - 1].b ? "true" : "false") << "\n";
	//				++cursor;
	//			}
	//			break;
	//			case OpCode::EXIT:
	//			{
	//				if( context.CallStackCursor == 0 )
	//				{
	//					std::cout << "[ERROR] Stack underflow for operation " << cursor << "\n";
	//					return false;
	//				}

	//				--context.CallStackCursor;

	//				return true;
	//			}
	//			default:
	//				std::cout << "[ERROR] Unexpected op code " << (uint32_t) program.Ops[cursor] << " for operation " << cursor << "\n";
	//				return false;
	//		}
	//	}

	//	return true;
	//}

#pragma endregion
}