#pragma once

#include "Parser.h"
#include "VirtualMachine.h"	// Pour les external functions

#pragma region Nouvelle analyse sémantique

//----------------------------------------
enum class RuntimeTypeCategory : uint8_t
{
	UNKNOWN,

	// Built-in
	VOID,
	BOOL,
	INT,
	FLOAT,
	CHAR,
	STRING,

	// User defined
	ENUM,
	STRUCT
};

//----------------------------------------
struct RuntimeType
{
	RuntimeTypeCategory Category;
	uint32_t Index;			// Utilisé pour les catégories ENUM et STRUCT
	uint32_t Size{ 0 };		// 0 indique que la taille n'a pas été calculée car on interdit les types vides
};

//----------------------------------------
struct EnumTypeDefinition
{
	RuntimeTypeId TypeId;
	ast_node_index Node;
};

//----------------------------------------
struct StructTypeDefinition
{
	RuntimeTypeId TypeId;
	ast_node_index Node;
};

//----------------------------------------
struct FunctionDefinition
{
	ast_node_index Node;
};

//----------------------------------------
struct SemanticAnalyzisData
{
	// Liste des enums définis par l'utilisateur
	EnumTypeDefinition Enums[64];
	uint32_t EnumCount{ 0 };
	
	// Liste des types struct définis par l'utilisateur
	StructTypeDefinition Structs[64];
	uint32_t StructCount{ 0 };

	// Liste des types runtime (built-in, enum, struct)
	RuntimeType Types[140];
	uint32_t TypeCount{ 0 };

	// #TODO : à revoir : les mettre enfants du semantic analyzis et les initialiser en même temps que le reste des données ?
	static RuntimeTypeId UNKNOWN_TypeId;
	static RuntimeTypeId VOID_TypeId;
	static RuntimeTypeId BOOL_TypeId;
	static RuntimeTypeId INT_TypeId;
	static RuntimeTypeId FLOAT_TypeId;
	static RuntimeTypeId CHAR_TypeId;
	static RuntimeTypeId STRING_TypeId;


	// Liste des définitions de fonctions
	FunctionDefinition Functions[64];
	uint32_t FunctionCount{ 0 };
};

#pragma endregion



bool IsCastableTo( const SemanticAnalyzisData& analyzis_data, RuntimeTypeId type, RuntimeTypeId casting_type );

Error AnalyzeAST( const TextData& text_data, const TokenCollection& tokens, const ExternalFunctionCatalog& ex_func_catalog, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data );

void PrintAST( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data );
void PrintAnalyzis( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data );