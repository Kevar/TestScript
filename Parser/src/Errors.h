#pragma once

#include <numeric>

struct TextData;
struct TokenCollection;

//----------------------------------------
enum class ErrorType : uint32_t
{
	NONE,

	//-------------------
	// Text tokenization
	INVALID_ESCAPED_CHARACTER,
	INVALID_NUMBER_FORMAT_MULTIPLE_POINTS,
	INVALID_NUMBER_FORMAT_NO_DIGIT_AFTER_POINT,
	//-------------------

	//-------------------
	// Token parsing
	UNEXPECTED_TOKEN,

	// Var
	MISSING_VAR_KEYWORD,
	MISSING_VAR_NAME,
	MISSING_VAR_COLON_SEPARATOR,
	MISSING_VAR_TYPE,
	//-------------------

	//-------------------
	// AST Analyzis
	NAME_DUPLICATE,
	INCOMPATIBLE_OPERAND_TYPES,
	INCOMPATIBLE_RETURN_TYPE,
	UNKNOWN_OPERATOR,
	UNKNOWN_FUNCTION,
	UNKNOWN_TYPE,
	INVALID_TYPE,
	NOT_A_COMPOSED_TYPE,
	CYCLIC_TYPE_REFERENCE,

	INVALID_CAST,
	INVALID_INSTRUCTION,

	RESERVED_KEYWORD,

	EMPTY_ENUM,
	DUPLICATE_ENUM_VALUE,
	UNKNOWN_ENUM_VALUE,
	NOT_AN_ENUM_MEMBER_EXPRESSION,

	EMPTY_STRUCT,
	DUPLICATE_STRUCT_MEMBER,
	UNKNOWN_STRUCT_MEMBER,
	NOT_A_STRUCT_MEMBER_EXPRESSION,

	VAR_USE_BEFORE_DECLARATION,			// #TODO : attention, on a aussi le cas où on utilise un nom de variable qui n'existe tout simplement pas. Il faudrait discriminer ces cas

	ARGUMENT_COUNT_MISMATCH,
	ARGUMENT_TYPE_MISMATCH,

	MISSING_STRING_DATA,
	UNREFERENCED_STRING,
	//-------------------

	// Placeholder
	ERROR
};

//----------------------------------------
struct Error
{
	static Error OK;

	Error( ErrorType type, uint32_t character_or_token );

	ErrorType Type{ ErrorType::NONE };
	uint32_t CharacterOrToken;

	operator bool();
};

//----------------------------------------
void PrintError( const TextData& text_data, const TokenCollection& tokens, const Error& error );