#pragma once

#include "Tokenizer.h"
#include "Errors.h"

//----------------------------------------
enum class ASTNodeType : uint32_t
{
	STRUCT,
	ENUM,
	ENUM_VALUE,
	BRANCH,
	FUNCTION,
	ARGUMENT,
	SCOPE,
	VARIABLE_DECLARATION_WO_ASSIGNATION,	// WO : With Or Without
	VARIABLE_ASSIGNATION,
	FUNCTION_CALL,
	IF_BLOCK,
	WHILE_LOOP,
	CONSTANT_FLOAT,
	CONSTANT_INT,
	CONSTANT_BOOL,
	CONSTANT_CHARACTER,
	CONSTANT_STRING,
	SYMBOL_CALL,
	UNARY_OPERATOR,
	BINARY_OPERATOR,
	TERNARY_OPERATOR,
	MEMBER_CALL,
	RETURN_CALL,

	UNKNOWN
};

typedef uint32_t RuntimeTypeId;
typedef uint32_t ast_node_index;

//----------------------------------------
enum class SymbolContext : uint8_t
{
	GLOBAL,
	SCOPE,
	ENUM_TYPE_NAME,
	ENUM_MEMBER,
	STRUCT_MEMBER
};

//----------------------------------------
enum class UsageContext
{
	VALUE,
	ADDRESS,
	ADDRESS_OF_ADDRESS,

	NONE
};

//----------------------------------------
struct ASTNode
{
	ASTNodeType Type{ ASTNodeType::UNKNOWN };
	token_index ProvokingToken{ 0 };
	RuntimeTypeId NativeTypeId{ 0 };
	RuntimeTypeId ContextTypeId{ 0 };		// Utilisé pour indiquer au compilateur s'il y a besoin d'un cast et vers quel type
	UsageContext Usage{ UsageContext::NONE };

	union
	{
		struct
		{
			token_index Name;
			ast_node_index FirstMember;		// VariableDeclarationWOAssignation
		} Struct;

		struct
		{
			token_index Name;
			ast_node_index FirstValue;		// EnumValue
		} Enum;

		struct
		{
			token_index Value;
			int32_t CompiledValue;
		} EnumValue;

		struct
		{
			ast_node_index Function;		// Function
		} Branch;

		struct
		{
			token_index Name;
			ast_node_index FirstArgument;	// Argument
			token_index ReturnType;
			ast_node_index Body;			// Scope
			uint32_t ReservedMemorySize;
		} Function;

		struct
		{
			token_index Name;
			uint32_t LocalAddress;
			token_index Type;
		} Argument;

		struct
		{
			ast_node_index FirstInstruction;	// Scope OU IfBlock OU WhileLoop OU ReturnCall OU VariableDeclarationWOAssignation OU VariableAssignation OU FunctionCall
		} Scope;

		// Instructions --------
		//struct
		//{
		//	token_index Name;
		//	uint32_t LocalAddress;				// Utilisé en analyse et compilation
		//	token_index Type;
		//} VariableDeclaration;

		struct
		{
			//token_index Name;
			ast_node_index NameX;				// Expression de résolution de l'adresse à affecter
			bool Global;						// Indique que c'est une variable du scope global
			//uint32_t LocalAddress;				// Utiisé en analyse et compilation
			ast_node_index Value;				// Expression
		} VariableAssignation;

		//struct
		//{
		//	token_index Name;
		//	uint32_t LocalAddress;				// Utilisé en analyse et compilation
		//	token_index Type;
		//	ast_node_index Value;				// Expression
		//} VariableDeclarationAssignation;

		struct
		{
			token_index Name;
			bool Global;						// Indique que c'est une variable du scope global
			uint32_t LocalAddress;				// Utilisé en analyse et compilation
			token_index Type;
			ast_node_index Value;				// Expression. Si le node est à 0, c'est une simple déclaration sans assignation
		} VariableDeclarationWOAssignation;

		struct
		{
			token_index Name;
			ast_node_index FirstArgument;		// Expression
			bool ConsumeResult;					// Utilisé pour indiquer à la compilation que cette méthode renvoie une valeur (donc a une valeur sur la stack) mais que rien ne la récupère. Il faut donc explicitement la popper
		} FunctionCall;							// Aussi utilisé en Expression

		struct
		{
			ast_node_index Condition;			// Expression
			ast_node_index IfBlock;				// Scope
			ast_node_index ElseBlock;			// Scope
		} IfBlock;

		struct
		{
			ast_node_index Condition;			// Expression
			ast_node_index Block;				// Scope
		} WhileLoop;

		struct
		{
			ast_node_index Value;				// Expression
		} ReturnCall;
		// ---------------------

		// Expression ----------
		struct
		{
			token_index StrValue;
			float Value;
		} ConstantFloat;

		struct
		{
			token_index StrValue;
			int32_t Value;
		} ConstantInt;

		struct
		{
			token_index StrValue;
			bool Value;
		} ConstantBool;

		struct
		{
			token_index StrValue;
			char Value;
		} ConstantChar;

		struct
		{
			token_index StrValue;
		} ConstantString;

		struct
		{
			token_index Name;
			SymbolContext Context;
			union
			{
				uint32_t LocalAddress;				// Utilisé en analyse et compilation
				int32_t CompiledValue;
				uint32_t CompiledOffset;
			};
		} SymbolCall;

		struct
		{
			token_index Operator;
			ast_node_index Operand;				// Expression
		} UnaryOperator;

		struct
		{
			token_index Operator;
			ast_node_index FirstOperand;		// Expression
			ast_node_index SecondOperand;		// Expression
		} BinaryOperator;

		struct
		{
			token_index Operator;
			ast_node_index FirstOperand;		// Expression
			ast_node_index SecondOperand;		// Expression
			ast_node_index ThirdOperand;		// Expression
		} TernaryOperator;

		struct
		{
			ast_node_index Parent;				
			ast_node_index Member;
		} MemberCall;
		// ---------------------
	};

	ast_node_index NextNode;		// Utilisé pour les nodes qui peuvent être en nombre variable
};

//----------------------------------------
struct ASTNodeCollection
{
	ASTNode Nodes[1024];
	uint32_t NodeCount{ 0 };
};

Error ParseTokens( const TextData& text_data, const TokenCollection& tokens, ASTNodeCollection& out_ast );

const char* GetASTNodeName( ASTNodeType ast_node_type );

// #NOTE : pas de PrintAST ici, il est dans la partie analyse sémantique pour pouvoir afficher correctement les infos sémantiques qui sont mutualisées dans l'AST