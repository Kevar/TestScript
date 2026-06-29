#pragma once

#include "TextData.h"
#include "Errors.h"

//----------------------------------------
enum class TokenType : uint32_t
{
	WHITESPACE,

	OPENED_CURLY_BRACE,			// {
	CLOSED_CURLY_BRACE,			// }
	OPENED_PARENTHESIS,			// (
	CLOSED_PARENTHESIS,			// )
	OPENED_SQUARE_BRACKET,		// [
	CLOSED_SQUARE_BRACKET,		// ]

	COLON,						// :
	SEMI_COLON,					// ;
	COMMA,						// ,
	EQUAL,						// =

	PLUS,						// +
	MINUS,						// -
	STAR,						// *
	SLASH,						// /

	DOT,						// .

	INFERIOR,					// <
	SUPERIOR,					// >

	INFERIOR_EQUAL,				// <=
	SUPERIOR_EQUAL,				// >=

	COMPARISON,					// ==
	COMPARISON_NOT,				// !=

	AND,						// &&
	OR,							// ||

	BIT_AND,					// &
	BIT_OR,						// |

	EXCLAMATION_MARK,			// !
	QUESTION_MARK,				// ?

	NUMBER,						// Suite de chiffres et lettres
	LABEL,						// Suite de lettres, chiffres et underscore sans espace
	CHARACTER,					// Un caractère entouré par des '
	STRING,						// Toute séquence de caractères commençant et terminant par un " (avec gestion de l'échappement avec \)
	COMMENT,					// Toute séquence de caractères commençant par //

	UNDEFINED,					// Ce qu'on n'a pas pu catégoriser. Par exemple : une chaîne de caractère qui ne se termine pas


	END,						// Token spécial indiquant la fin de la collection de token

	COUNT
};

//----------------------------------------
struct Token
{
	TokenType Type;
	uint32_t TextPosition;
};

//----------------------------------------
struct TokenCollection
{
	Token Tokens[1024];
	uint32_t TokenCount{ 0 };
};

typedef uint32_t token_index;

void TokenizeText( const TextData& text_data, TokenCollection& out_tokens );

const char* GetTokenName( TokenType token_type );

bool GetTokenValue( const TextData& text_data, const TokenCollection& tokens, token_index token_id, TextData& out_value );
bool IsTokenContentEquals( const TextData& text_data, const TokenCollection& tokens, token_index token_id, const char* comparison );
bool IsTokenContentEquals( const TextData& text_data, const TokenCollection& tokens, token_index token_id, const TextData& comparison );
bool AreTokenContentsEqual( const TextData& text_data, const TokenCollection& tokens, token_index first_token_id, token_index second_token_id );

void PrintTokenMetrics( const TextData& text_data, const TokenCollection& tokens, token_index token_id );
void PrintTokenValue( const TextData& text_data, const TokenCollection& tokens, token_index token_id, bool display_id );
void PrintTokens( const TextData& text_data, const TokenCollection& tokens, bool display_whitespaces = false );
