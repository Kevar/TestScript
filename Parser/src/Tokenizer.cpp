#include "Tokenizer.h"

#include <iostream>

//----------------------------------------
static const char* l_tokenNames[TokenType::COUNT] =
{
	"WHITESPACE",

	"OPENED_CURLY_BRACE",
	"CLOSED_CURLY_BRACE",
	"OPENED_PARENTHESIS",
	"CLOSED_PARENTHESIS",
	"OPENED_SQUARE_BRACKET",
	"CLOSED_SQUARE_BRACKET",

	"COLON",
	"SEMI_COLON",
	"COMMA",
	"EQUAL",

	"PLUS",
	"MINUS",
	"STAR",
	"SLASH",

	"DOT",

	"INFERIOR",
	"SUPERIOR",

	"INFERIOR_EQUAL",
	"SUPERIOR_EQUAL",

	"COMPARISON",
	"COMPARISON_NOT",

	"AND",
	"OR",

	"BIT_AND",
	"BIT_OR",

	"EXCLAMATION_MARK",
	"QUESTION_MARK",

	"NUMBER",
	"LABEL",
	"CHARACTER",
	"STRING",
	"COMMENT",

	"UNDEFINED",

	"END"
};

#pragma region Helpers

//----------------------------------------
static bool IsNextCharacterIs( const TextData& text_data, uint32_t cursor, char c )
{
	return cursor + 1 < text_data.Length && text_data.Buffer[cursor + 1] == c;
}

//----------------------------------------
static bool IsWhitespace( char c )
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

//----------------------------------------
static bool IsLetter( char c )
{
	return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

//----------------------------------------
static bool IsNumeric( char c )
{
	return c >= '0' && c <= '9';
}

//----------------------------------------
static bool IsNumericOperator( char c )
{
	return c == '+' || c == '-' || c == '*' || c == '/';
}

//----------------------------------------
static bool IsLabelCharacter( char c )
{
	return IsLetter( c ) || IsNumeric( c ) || c == '_';
}

//----------------------------------------
static void TokenizeUndefined( const TextData& text_data, Token& token, uint32_t& cursor )
{
	++cursor;

	token.Type = TokenType::UNDEFINED;

	while( cursor < text_data.Length && !IsWhitespace( text_data.Buffer[cursor] ) )
	{
		// #TODO : il faudrait détecter le début d'un commentaire et switcher dessus si besoin
		++cursor;
	}
}

//----------------------------------------
static void TokenizeWhitespace( const TextData& text_data, Token& token, uint32_t& cursor )
{
	++cursor;

	token.Type = TokenType::WHITESPACE;

	while( cursor < text_data.Length && IsWhitespace( text_data.Buffer[cursor] ) )
		++cursor;
}

//----------------------------------------
static void TokenizeNumeric( const TextData& text_data, Token& token, uint32_t& cursor )
{
	++cursor;

	token.Type = TokenType::NUMBER;

	bool pointFound = false;

	while( cursor < text_data.Length )
	{
		char c = text_data.Buffer[cursor];

		if( c == '.' )
		{
			if( pointFound )
			{
				TokenizeUndefined( text_data, token, cursor );		// Double point : erreur
				return;
			}

			pointFound = true;
		}
		else if( !IsNumeric( c ) )
		{
			if( pointFound && text_data.Buffer[cursor - 1] == '.' )
			{
				TokenizeUndefined( text_data, token, cursor );		// On n'a pas de chiffre juste après le point : erreur
				return;
			}
			else
			{
				return;
			}
		}

		++cursor;
	}
}

//----------------------------------------
static void TokenizeLabel( const TextData& text_data, Token& token, uint32_t& cursor )
{
	++cursor;

	token.Type = TokenType::LABEL;

	while( cursor < text_data.Length && IsLabelCharacter( text_data.Buffer[cursor] ) )
		++cursor;
}

//----------------------------------------
static void TokenizeCharacter(const TextData& text_data, Token& token, uint32_t& cursor)
{
	++cursor;

	token.Type = TokenType::UNDEFINED;
	
	// On a 2 cas à gérer :
	//	* Cas simple ['] + [character] + [']
	//	* Cas échappé ['] + [\] + [character] + [']

	if (cursor >= text_data.Length)
		return;

	if (text_data.Buffer[cursor] == '\\')
	{
		// Echappement
		++cursor;

		if (cursor >= text_data.Length)
			return;
	}

	// Caractère
	++cursor;

	if (cursor >= text_data.Length)
		return;

	// Quote de fermeture
	if(text_data.Buffer[cursor] == '\'')
		token.Type = TokenType::CHARACTER;
	
	++cursor;
}

//----------------------------------------
static void TokenizeString( const TextData& text_data, Token& token, uint32_t& cursor )
{
	++cursor;

	token.Type = TokenType::UNDEFINED;		// On ne va le basculer en STRING que si on trouve un double quote de fermeture propre
	bool escaping = false;

	while( cursor < text_data.Length )
	{
		char c = text_data.Buffer[cursor];

		if( c == '"' )
		{
			if( !escaping )
			{
				token.Type = TokenType::STRING;
				++cursor;
				return;
			}
			else
				escaping = false;
		}
		else if( c == '\\' )
		{
			escaping = !escaping;
		}
		else if( escaping )
		{
			// #TODO : Gérer les autres caractères valides à échapper (comme \t)
			// Et pour les caractères invalides, on a donc une string malformée, donc on doit remonter l'erreur mais on continue de parser quand même
			escaping = false;
		}

		++cursor;
	}
}

//----------------------------------------
static void TokenizeCommentOrSlash( const TextData& text_data, Token& token, uint32_t& cursor )
{
	++cursor;

	if( cursor >= text_data.Length || text_data.Buffer[cursor] != '/' )
	{
		token.Type = TokenType::SLASH;
	}
	else
	{
		token.Type = TokenType::COMMENT;
		while( cursor < text_data.Length && text_data.Buffer[cursor] != '\n' )
			++cursor;
	}
}

#pragma endregion

//----------------------------------------
void TokenizeText( const TextData& text_data, TokenCollection& out_tokens )
{
	out_tokens.TokenCount = 0;
	uint32_t cursor = 0;

	while( cursor < text_data.Length )
	{
		char c = text_data.Buffer[cursor];

		Token& token = out_tokens.Tokens[out_tokens.TokenCount];
		token.TextPosition = cursor;
		out_tokens.TokenCount++;

		switch( c )
		{
			case '{':	token.Type = TokenType::OPENED_CURLY_BRACE;		++cursor;	break;
			case '}':	token.Type = TokenType::CLOSED_CURLY_BRACE;		++cursor;	break;
			case '[':	token.Type = TokenType::OPENED_SQUARE_BRACKET;	++cursor;	break;
			case ']':	token.Type = TokenType::CLOSED_SQUARE_BRACKET;	++cursor;	break;
			case '(':	token.Type = TokenType::OPENED_PARENTHESIS;		++cursor;	break;
			case ')':	token.Type = TokenType::CLOSED_PARENTHESIS;		++cursor;	break;
			case ':':	token.Type = TokenType::COLON;					++cursor;	break;
			case ';':	token.Type = TokenType::SEMI_COLON;				++cursor;	break;
			case ',':	token.Type = TokenType::COMMA;					++cursor;	break;
			case '!':
			{
				if( IsNextCharacterIs( text_data, cursor, '=' ) )
				{
					token.Type = TokenType::COMPARISON_NOT;
					cursor += 2;
				}
				else
				{
					token.Type = TokenType::EXCLAMATION_MARK;
					++cursor;
				}
			}
			break;
			case '?':	token.Type = TokenType::QUESTION_MARK;			++cursor;	break;
			case '=':
			{
				if( IsNextCharacterIs( text_data, cursor, '=' ) )
				{
					token.Type = TokenType::COMPARISON;
					cursor += 2;
				}
				else
				{
					token.Type = TokenType::EQUAL;
					++cursor;
				}
			}
			break;
			case '+':	token.Type = TokenType::PLUS;					++cursor;	break;
			case '-':	token.Type = TokenType::MINUS;					++cursor;	break;
			case '*':	token.Type = TokenType::STAR;					++cursor;	break;
			case '.':	token.Type = TokenType::DOT;					++cursor;	break;
			case '<':
			{
				if( IsNextCharacterIs( text_data, cursor, '=' ) )
				{
					token.Type = TokenType::INFERIOR_EQUAL;
					cursor += 2;
				}
				else
				{
					token.Type = TokenType::INFERIOR;
					++cursor;
				}
			}
			break;
			case '>':
			{
				if( IsNextCharacterIs( text_data, cursor, '=' ) )
				{
					token.Type = TokenType::SUPERIOR_EQUAL;
					cursor += 2;
				}
				else
				{
					token.Type = TokenType::SUPERIOR;
					++cursor;
				}
			}
			break;
			case '&':
			{
				if( IsNextCharacterIs( text_data, cursor, '&' ) )
				{
					token.Type = TokenType::AND;
					cursor += 2;
				}
				else
				{
					token.Type = TokenType::BIT_AND;
					++cursor;
				}
			}
			break;
			case '|':
			{
				if( IsNextCharacterIs( text_data, cursor, '|' ) )
				{
					token.Type = TokenType::OR;
					cursor += 2;
				}
				else
				{
					token.Type = TokenType::BIT_OR;
					++cursor;
				}
			}
			break;
			case '/':	TokenizeCommentOrSlash( text_data, token, cursor );			break;
			case '\'':	TokenizeCharacter( text_data, token, cursor );				break;
			case '"':	TokenizeString( text_data, token, cursor );					break;
			default:
				if( IsLetter( c ) )
					TokenizeLabel( text_data, token, cursor );
				else if( IsWhitespace( c ) )
					TokenizeWhitespace( text_data, token, cursor );
				else if( IsNumeric( c ) )
					TokenizeNumeric( text_data, token, cursor );
				else
					TokenizeUndefined( text_data, token, cursor );
				break;
		}
	}

	Token& token = out_tokens.Tokens[out_tokens.TokenCount];
	token.Type = TokenType::END;
	token.TextPosition = cursor;
	// out_tokens.TokenCount++;		// Le END est une sentinelle au cas où on dépasse le nombre de token
}

//----------------------------------------
const char* GetTokenName( TokenType token_type )
{
	if( token_type < TokenType::COUNT )
		return l_tokenNames[(uint32_t)token_type];
	else
		return "#ERROR INVALID TokenType";
}

//----------------------------------------
bool GetTokenValue( const TextData& text_data, const TokenCollection& tokens, token_index token_id, TextData& out_value )
{
	if( token_id >= tokens.TokenCount - 1 )
		return false;

	const Token& token = tokens.Tokens[token_id];
	const Token& nextToken = tokens.Tokens[token_id + 1];

	out_value.Buffer = text_data.Buffer + token.TextPosition;
	out_value.Length = nextToken.TextPosition - token.TextPosition;

	return true;
}

//----------------------------------------
bool IsTokenContentEquals( const TextData& text_data, const TokenCollection& tokens, token_index token_id, const char* comparison )
{
	if( token_id >= tokens.TokenCount - 1 )
		return false;

	const Token& token = tokens.Tokens[token_id];
	const Token& nextToken = tokens.Tokens[token_id + 1];

	uint32_t readCursor = 0;
	uint32_t tokenLength = nextToken.TextPosition - token.TextPosition;

	while( readCursor < tokenLength && comparison[readCursor] != '\0' )
	{
		if( text_data.Buffer[token.TextPosition + readCursor] != comparison[readCursor] )
			return false;

		++readCursor;
	}

	if( readCursor < tokenLength )			// Il reste des caractères dans le token
		return false;

	if( comparison[readCursor] != '\0' )	// Il reste des caractères dans la chaîne de comparaison
		return false;

	return true;
}

//----------------------------------------
bool IsTokenContentEquals( const TextData& text_data, const TokenCollection& tokens, token_index token_id, const TextData& comparison )
{
	if( token_id >= tokens.TokenCount - 1 )
		return false;

	const Token& token = tokens.Tokens[token_id];
	const Token& nextToken = tokens.Tokens[token_id + 1];

	uint32_t readCursor = 0;
	uint32_t tokenLength = nextToken.TextPosition - token.TextPosition;

	if( tokenLength != comparison.Length )
		return false;

	while( readCursor < tokenLength )
	{
		if( text_data.Buffer[token.TextPosition + readCursor] != comparison.Buffer[readCursor] )
			return false;

		++readCursor;
	}

	return true;
}

//----------------------------------------
bool AreTokenContentsEqual( const TextData& text_data, const TokenCollection& tokens, token_index first_token_id, token_index second_token_id )
{
	if( first_token_id >= tokens.TokenCount - 1 )
		return false;
	if( second_token_id >= tokens.TokenCount - 1 )
		return false;

	if( first_token_id == second_token_id )
		return true;

	const Token& firstToken = tokens.Tokens[first_token_id];
	const Token& nextFirstToken = tokens.Tokens[first_token_id + 1];

	const Token& secondToken = tokens.Tokens[second_token_id];
	const Token& nextSecondToken = tokens.Tokens[second_token_id + 1];

	uint32_t tokenFirstLength = nextFirstToken.TextPosition - firstToken.TextPosition;
	uint32_t tokenSecondLength = nextSecondToken.TextPosition - secondToken.TextPosition;

	if( tokenFirstLength != tokenSecondLength )
		return false;

	uint32_t readCursor = 0;
	while( readCursor < tokenFirstLength )
	{
		if( text_data.Buffer[firstToken.TextPosition + readCursor] != text_data.Buffer[secondToken.TextPosition + readCursor] )
			return false;

		++readCursor;
	}

	return true;
}


//----------------------------------------
void PrintTokenMetrics( const TextData& text_data, const TokenCollection& tokens, token_index token_id )
{
	// #NOTE : ce PrintToken est pensé pour avoir une correspondance avec les métriques de Notepad++, d'où le fait de démarrer le compte à 1, de rajouter les carriage return et de compter 4 pour les tabs

	const Token& token = tokens.Tokens[token_id];

	uint32_t line = 1;
	uint32_t column = 1;

	GetLineAndColumn( text_data, token.TextPosition, line, column );

	// fopen_s supprime les carriage return (et ça ne semble pas paramétrable)
	// En conséquence, la position du token est correcte vis à vis du buffer de travail, mais pas vis à vis du texte brut
	// Si on veut correctement identifier la position dans le fichier texte pour diagnostiquer, il faut rajouter le nombre de carriage return supprimés
	//	> Sachant que cette approche suppose que le fichier texte initial a bien été produit avec des carriages return, ce qui est très probable mais non garanti
	// #NOTE : si on passe en lecture de fichier avec "rb", les \r vont probablement revenir, il faudra vérifier que ça marche toujours correctement
	uint32_t carriageReturnAwarePosition = token.TextPosition + line;

	std::cout << "[" << l_tokenNames[(uint32_t) token.Type] << "]";
	if( token.Type == TokenType::LABEL )
	{
		std::cout << " \"";
		PrintTokenValue( text_data, tokens, token_id, true );
		std::cout << "\"";
	}

	std::cout << " @" << carriageReturnAwarePosition << " (line " << line << " column " << column << ")";
}

//----------------------------------------
void PrintTokenValue( const TextData& text_data, const TokenCollection& tokens, token_index token_id, bool display_id)
{
	const Token& current = tokens.Tokens[token_id];
	const Token& next = tokens.Tokens[token_id + 1];

	if (display_id)
		std::cout << token_id << ": ";

	PrintTextSlice( text_data.Buffer + current.TextPosition, next.TextPosition - current.TextPosition );
}

//----------------------------------------
void PrintTokens( const TextData& text_data, const TokenCollection& tokens, bool display_whitespaces )
{
	for( uint32_t i = 0; i < tokens.TokenCount - 1; ++i )		// -1 pour retirer le token END qui est là uniquement pour des raisons techniques
	{
		const Token& token = tokens.Tokens[i];

		if( !display_whitespaces && token.Type == TokenType::WHITESPACE )
			continue;

		const Token& nextToken = tokens.Tokens[i + 1];

		std::cout << "ID "<< i << " @[" << token.TextPosition << ":" << nextToken.TextPosition << "] " << l_tokenNames[(uint32_t) token.Type] << "\n";

		if( token.Type == TokenType::LABEL || token.Type == TokenType::STRING || token.Type == TokenType::COMMENT || token.Type == TokenType::UNDEFINED )
		{
			std::cout << "\t";
			PrintTextSlice( text_data.Buffer, token.TextPosition, nextToken.TextPosition );
			std::cout << "\n";
		}
	}
}
