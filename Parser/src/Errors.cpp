#include "Errors.h"

#include <iostream>

#include "Tokenizer.h"

//----------------------------------------
static const char* l_errorNames[] =
{
	"NONE",

	"INVALID_NUMBER_FORMAT_MULTIPLE_POINTS",
	"INVALID_NUMBER_FORMAT_NO_DIGIT_AFTER_POINT",
	"INVALID_ESCAPED_CHARACTER",

	"UNEXPECTED_TOKEN",

	"MISSING_VAR_KEYWORD",
	"MISSING_VAR_NAME",
	"MISSING_VAR_COLON_SEPARATOR",
	"MISSING_VAR_TYPE",

	"NAME_DUPLICATE",
	"INCOMPATIBLE_OPERAND_TYPES",
	"INCOMPATIBLE_RETURN_TYPE",
	"UNKNOWN_OPERATOR",
	"UNKNOWN_FUNCTION",
	"UNKNOWN_TYPE",
	"INVALID_TYPE",
	"NOT_A_COMPOSED_TYPE",
	"CYCLIC_TYPE_REFERENCE",

	"INVALID_CAST",
	"INVALID_INSTRUCTION",

	"RESERVED_KEYWORD",

	"EMPTY_ENUM",
	"DUPLICATE_ENUM_VALUE",
	"UNKNOWN_ENUM_VALUE",
	"NOT_AN_ENUM_MEMBER_EXPRESSION",

	"EMPTY_STRUCT",
	"DUPLICATE_STRUCT_MEMBER",
	"UNKNOWN_STRUCT_MEMBER",
	"NOT_A_STRUCT_MEMBER_EXPRESSION",

	"VAR_USE_BEFORE_DECLARATION",

	"ARGUMENT_COUNT_MISMATCH",
	"ARGUMENT_TYPE_MISMATCH",

	"MISSING_STRING_DATA",
	"UNREFERENCED_STRING",

	"ERROR"
};

static uint32_t l_errorNamesCount = sizeof( l_errorNames ) / sizeof( l_errorNames[0] );

//----------------------------------------
static ErrorType l_textErrors[] =
{
	ErrorType::INVALID_ESCAPED_CHARACTER,
	ErrorType::INVALID_NUMBER_FORMAT_MULTIPLE_POINTS,
	ErrorType::INVALID_NUMBER_FORMAT_NO_DIGIT_AFTER_POINT
};

//----------------------------------------
static uint32_t l_textErrorsCount = sizeof( l_textErrors ) / sizeof( l_textErrors[0] );

//----------------------------------------
static bool IsTextError( ErrorType error_type )
{
	for( uint32_t i = 0; i < l_textErrorsCount; ++i )
	{
		if( l_textErrors[i] == error_type )
			return true;
	}

	return false;
}

//----------------------------------------
Error Error::OK( ErrorType::NONE, 0 );

//----------------------------------------
Error::Error( ErrorType type, uint32_t character_or_token ) :
	Type( type ),
	CharacterOrToken( character_or_token )
{
	if( Type == ErrorType::ERROR )
	{
		std::cout << "Breakpoint\n";
	}

	if (Type == ErrorType::UNEXPECTED_TOKEN)
	{
		std::cout << "Breakpoint\n";
	}
}

//----------------------------------------
Error::operator bool()
{
	return Type != ErrorType::NONE;
}

//----------------------------------------
void PrintError( const TextData& text_data, const TokenCollection& tokens, const Error& error )
{
	uint32_t errorTypeIndex = (uint32_t) error.Type;

	if( errorTypeIndex >= l_errorNamesCount )
		throw "Oups, j'ai dû oublier de rajouter un nom";

	std::cout << "[ERROR] " << l_errorNames[(uint32_t) error.Type] << " ";

	if( IsTextError( error.Type ) )
	{
		uint32_t line;
		uint32_t column;

		GetLineAndColumn( text_data, error.CharacterOrToken, line, column );

		uint32_t carriageReturnAwarePosition = error.CharacterOrToken + line;

		std::cout << "Character \'" << text_data.Buffer[error.CharacterOrToken] << "\' @" << carriageReturnAwarePosition << " (line " << line << " column " << column << ")\n";
	}
	else
	{
		std::cout << "Token ";
		PrintTokenMetrics( text_data, tokens, error.CharacterOrToken );
		std::cout << "\n";
	}
}