#pragma once

#include <numeric>

//#pragma region Utilities
//
//void PrintSlice( const char* buffer, uint32_t from, uint32_t to );
//void PrintSlice( const char* buffer, uint32_t count );
//
//#pragma endregion

//----------------------------------------
namespace PARSER
{
//#pragma region FileData
//
//	//----------------------------------------
//	struct FileData
//	{
//		const char* Path{ nullptr };
//		char* Buffer{ nullptr };
//		uint32_t Length{ 0 };
//	};
//	
//	bool LoadFile( const char* path, FileData& out_file_data );
//	void CloseFile( FileData& file_data );
//	
//	void PrintFile( const FileData& file_data );
//
//#pragma endregion

//#pragma region Tokenization
//
//	//----------------------------------------
//	enum class TokenType : uint32_t
//	{
//		WHITESPACE,
//
//		OPENED_CURLY_BRACE,			// {
//		CLOSED_CURLY_BRACE,			// }
//		OPENED_PARENTHESIS,			// (
//		CLOSED_PARENTHESIS,			// )
//		OPENED_SQUARE_BRACKET,		// [
//		CLOSED_SQUARE_BRACKET,		// ]
//
//		COLON,						// :
//		SEMI_COLON,					// ;
//		COMMA,						// ,
//		EQUAL,						// =
//
//		PLUS,						// +
//		MINUS,						// -
//		STAR,						// *
//		SLASH,						// /
//
//		DOT,						// .
//
//		INFERIOR,					// <
//		SUPERIOR,					// >
//
//		INFERIOR_EQUAL,				// <=
//		SUPERIOR_EQUAL,				// >=
//
//		COMPARISON,					// ==
//		COMPARISON_NOT,				// !=
//
//		AND,						// &&
//		OR,							// ||
//
//		BIT_AND,					// &
//		BIT_OR,						// |
//
//		EXCLAMATION_MARK,			// !
//		QUESTION_MARK,				// ?
//
//		NUMBER,
//		LABEL,						// Suite de lettres, chiffres et underscore sans espace
//		STRING,						// Toute séquence de caractères commençant et terminant par un " (avec gestion de l'échappement avec \)
//		COMMENT,					// Toute séquence de caractères commençant par //
//
//		UNDEFINED,					// Ce qu'on n'a pas pu catégoriser. Par exemple : une chaîne de caractère qui ne se termine pas
//
//
//		END,						// Token spécial indiquant la fin de la collection de token
//
//		COUNT
//	};
//
//	//----------------------------------------
//	struct Token
//	{
//		TokenType Type;
//		uint32_t Position;
//	};
//
//	//----------------------------------------
//	struct TokenCollection
//	{
//		Token Tokens[1024];
//		uint32_t TokenCount{ 0 };
//	};
//
//	void ParseText( const FileData& file_data, TokenCollection& out_tokens );
//	void PrintTokens( const FileData& file_data, const TokenCollection& tokens, bool display_whitespaces = false );
//
//#pragma endregion

//#pragma region AST
//
//	//----------------------------------------
//	enum class ASTNodeType : uint32_t
//	{
//		STRUCT,
//		ENUM,
//		ENUM_VALUE,
//		BRANCH,
//		FUNCTION,
//		ARGUMENT,
//		SCOPE,
//		VARIABLE_DECLARATION,
//		VARIABLE_ASSIGNATION,
//		VARIABLE_DECLARATION_ASSIGNATION,
//		FUNCTION_CALL,
//		IF_BLOCK,
//		WHILE_LOOP,
//		CONSTANT_FLOAT,
//		CONSTANT_INT,
//		CONSTANT_BOOL,
//		CONSTANT_STRING,
//		VARIABLE_CALL,
//		UNARY_OPERATOR,
//		BINARY_OPERATOR,
//		TERNARY_OPERATOR,
//		MEMBER_CALL,
//		RETURN_CALL,
//
//		UNKNOWN,
//
//		COUNT
//	};
//
//	typedef uint32_t token_index;
//	typedef uint32_t ast_node_index;
//
//	// #NOTE : utilisé pour l'analyse sémantique mais défini ici car on réutilise l'AST pour l'analyse sémantique
//	//----------------------------------------
//	enum class RuntimeTypeType : uint16_t
//	{
//		NONE,
//
//		VOID,				// VOID est différent de NONE : NONE représente une absence d'information, VOID représente explicitement "pas de type"
//		BOOL,
//		INT,
//		FLOAT,
//
//		USER_DEFINED
//	};
//
//	//----------------------------------------
//	struct RuntimeType
//	{
//		RuntimeTypeType Type;
//		uint16_t UserDefinedType;
//	};
//
//	//----------------------------------------
//	struct ASTNode
//	{
//		ASTNodeType Type;
//		RuntimeType RuntimeType{ RuntimeTypeType::NONE, 0 };
//
//		union
//		{
//			struct
//			{
//				token_index Name;
//				ast_node_index FirstMember;
//			} Struct;
//
//			struct
//			{
//				token_index Name;
//				ast_node_index FirstValue;
//			} Enum;
//
//			struct
//			{
//				token_index Value;
//			} EnumValue;
//
//			struct
//			{
//				ast_node_index Function;
//			} Branch;
//
//			struct
//			{
//				token_index Name;
//				ast_node_index FirstArgument;
//				token_index ReturnType;
//				ast_node_index Body;
//			} Function;
//
//			struct
//			{
//				token_index Name;
//				token_index Type;
//			} Argument;
//
//			struct
//			{
//				ast_node_index FirstInstruction;
//			} Scope;
//
//			// Instructions --------
//			struct
//			{
//				token_index Name;
//				uint32_t LocalAddress;	//Utiisé en analyse et compilation
//				token_index Type;
//			} VariableDeclaration;
//
//			struct
//			{
//				token_index Name;
//				uint32_t LocalAddress;	//Utiisé en analyse et compilation
//				ast_node_index Value;
//			} VariableAssignation;
//
//			struct
//			{
//				token_index Name;
//				uint32_t LocalAddress;	//Utiisé en analyse et compilation
//				token_index Type;
//				ast_node_index Value;
//			} VariableDeclarationAssignation;
//
//			struct
//			{
//				token_index Name;
//				ast_node_index FirstArgument;
//			} FunctionCall;							// Aussi utilisé en Expression
//
//			struct
//			{
//				ast_node_index Condition;
//				ast_node_index IfBlock;
//				ast_node_index ElseBlock;
//			} ConditionBlock;
//
//			struct
//			{
//				ast_node_index Condition;
//				ast_node_index Block;
//			} WhileLoop;
//
//			struct
//			{
//				ast_node_index Value;
//			} ReturnCall;
//			// ---------------------
//
//			// Expression ----------
//			struct
//			{
//				token_index StrValue;
//				float Value;
//			} ConstantFloat;
//
//			struct
//			{
//				token_index StrValue;
//				int32_t Value;
//			} ConstantInt;
//
//			struct
//			{
//				token_index StrValue;
//				bool Value;
//			} ConstantBool;
//
//			struct
//			{
//				token_index Value;
//			} ConstantString;
//
//			struct
//			{
//				token_index Name;
//				uint32_t LocalAddress;	//Utiisé en analyse et compilation
//			} VariableCall;
//
//			struct
//			{
//				token_index Operator;
//				ast_node_index Operand;
//			} UnaryOperator;
//
//			struct
//			{
//				token_index Operator;
//				ast_node_index FirstOperand;
//				ast_node_index SecondOperand;
//			} BinaryOperator;
//
//			struct
//			{
//				token_index Operator;
//				ast_node_index FirstOperand;
//				ast_node_index SecondOperand;
//				ast_node_index ThirdOperand;
//			} TernaryOperator;
//
//			struct
//			{
//				ast_node_index Parent;
//				ast_node_index Member;
//			} MemberCall;
//			// ---------------------
//		};
//
//		ast_node_index NextNode;		// Utilisé pour les nodes qui peuvent être en nombre variable
//	};
//
//	//----------------------------------------
//	struct ASTNodeCollection
//	{
//		ASTNode Nodes[1024];
//		uint32_t NodeCount{ 0 };
//	};
//
//	bool ParseTokens( const FileData& file_data, const TokenCollection& tokens, ASTNodeCollection& out_ast, token_index& out_error_token );
//
//	void PrintToken( const FileData& file_data, const TokenCollection& tokens, token_index token_cursor );
//
//#pragma endregion

//#pragma region Semantic Analyzis
//
//	//----------------------------------------
//	struct UserTypeDefinition
//	{
//		uint16_t TypeId;
//		ast_node_index Node;
//	};
//
//	//----------------------------------------
//	struct SemanticAnalyzisData
//	{
//		UserTypeDefinition TypeDefinitions[256];
//		uint32_t TypeDefinitionCount{ 0 };
//
//		ast_node_index FunctionDefinitions[256];
//		uint32_t FunctionDefinitionCount{ 0 };
//
//		ast_node_index CurrentFunctionDefinition{ (uint32_t)-1 };
//
//		uint32_t LocalAddressSeed{ 0 };
//	};
//
//	bool AnalyzeAST( const FileData& file_data, const TokenCollection& tokens, ASTNodeCollection& ast, SemanticAnalyzisData& analyzis_data );
//
//	void PrintAST( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data );
//	void PrintAnalyzis( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data );
//
//#pragma endregion

#pragma region Virtual Machine

	//----------------------------------------
	// Traitement des appels de fonction :
	//	1 Préparer les paramètres :
	//		> Empiler sur la stack l'index de l'instruction suivant l'appel de la fonction
	//		> Empiler sur la stack les MemoryBlock des valeurs de retour
	//		> Empiler sur la stack les MemoryBlock des arguments
	//	2 Appel de la fonction :
	//		> Faire sauter le curseur à la première instruction de la fonction
	//		> Par construction, la fonction "sait" que les derniers blocks de la stack représentent les arguments
	//	

	//----------------------------------------
	// #NOTES sur la gestion de la mémoire et des variables :
	//	* A la compilation
	//		> Objectifs
	//			- détecter les variables déclarées 2+ fois (erreur) ou utilisées sans avoir été déclarées (erreur)
	//			- gérer les scopes imbriqués (permettant de réutiliser le même nom dans 2 scopes différents)
	//			- gérer le scope global qui sera partagé entre toutes les branches
	//		> Données de compilation :
	//			- Map associant le nom d'une variable à son type et un index (aka son nom compilé)
	//			  L'index est reconstruit par parcours de la map (single source of truth)
	//			- Dans cette map est injecté (via l'utilisation d'un union) des "frontières" de scope : lorsqu'en compilant on tombe sur la fin d'un scope, il suffit de remonter jusqu'à la frontière de scope précédente
	//			- Le scope implicite de plus haut niveau (aka début de la map) correspond au scope global, aka les variables déclarées en dehors d'une fonction ou d'une struct
	//			  Note : ce scope global doit être construit en entier dans une première passe avant de traiter les fonctions pour éviter la contrainte de déclarer des variables globales au début
	//		> Utilisation à la compilation :
	//			- Lorsqu'on tombe sur la déclaration d'une variable, on vérifie si elle n'a pas déjà été déclarée (par nom via la map) et on émet une opération de création d'une variable (de la taille définie par le type avec l'index de la variable)
	//			- Lorsqu'on tombe sur l'assignation d'une variable, on vérifie si elle a été déclarée (par nom via la map) et on utilise l'index comme paramètre de l'opération de pop to memory
	//			- Lorsqu'on tombe sur l'utilisation d'une variable, on vérifie si elle a été déclarée (par nom via la map) et on utilise l'index comme paramètre de l'opération push from memory
	//			  Note sur les variables membres d'une instance de struct : l'index de déclaration des variables ne s'incrémente pas par variable, mais par membre de variable :
	//				On n'a PAS ça :
	//				0	> toto:int						@x12
	//				1	> tutu:my_struct (4 membres)	@x13
	//				2	> tata:bool						@x17
	// 
	//				Mais on a ça :
	//				0	> toto:int						@x12
	//				1	> tutu.member1:int				@x13
	//				2	> tutu.member2:float			@x14
	//				3	> tutu.member3:float			@x15
	//				4	> tutu.member4:bool				@x16
	//				5	> tata:bool						@x17
	//	* A l'exécution
	//		> Contexte :
	//			- On a une map associant l'index de la variable à une adresse mémoire explicite de la memory stack
	//		> Utilisation avec les opérations :
	//			- Lorsqu'on a une opération déclarant une variable, on ajoute une entrée dans la mémoire qu'on associée à l'index de la variable stockée dans les paramètres d'opération
	//			  Note : à ce stade, une struct n'existe plus, elle est remplacée par une suite de variables
	//			- Lorsqu'on a une opération utilisant une variable, on utilise la map dans le contexte pour aller chercher la valeur en mémoire adhoc
	//			- Lorsqu'on retourne une valeur, si cette dernière est une valeur composite (struct ou string), on copie cette valeur sur le dessus de la memory stack du contexte d'appel et on décale le curseur de contexte avant de passer dans l'execution stack l'adresse mémoire (index) de la valeur

	// #NOTES sur la gestion des structs :
	//	* Une struct n'existe plus lors de l'exécution : ce n'est plus qu'une liste de variables fondamentales (bool, int, float et string)
	//	* Passer une struct en paramètres revient à empiler autant de variables que nécessaire sur l'execution stack
	//	* Renvoyer une struct en retour de fonction revient à empiler autant de variables que nécessaire sur l'execution stack
	//	* Les sites d'appel/réception de fonctions ont à leur charge de gérer toute cette liste de variables
	//	* Le nommage des variables issues du "dépliage" des structs utilise simplement la syntaxe pointée comme clé (my_struct.my_member)

	// #TODO :
	//	* [OK] Ajouter l'opération de création d'une variable (init à 0)
	//	* [OK] Ajouter la map index de variable -> adresse mémoire dans le contexte
	//		> Attention, à terme avec les branches, la map "globale" sera commune à toutes les branches, donc on ne pourra plus avoir une "simple" map
	//	* [OK] Modifier les opérations push from memory et pop to memory pour utiliser cette map
	//	* [OFF] Créer le type pour stocker les infos de type et d'index d'une variable en union avec un délimiteur de scope
	//		> PAS BESOIN, je peux construire cette info en remontant l'AST
	//		> Petite opti : on stocke dans les noeuds liées à des variables le compiled name qui correspond au numéro de la variable dans l'ordre du fichier
	//	* [OFF] Créer la map de construction des variables
	//	  [OFF] Cette map va être modifiée par écrasement lorsqu'on ferme un scope. Comment conserver toutes ces informations pour en faire des données "pdb" ?
	//		> PAS BESOIN, je peux construire cette info en remontant l'AST
	// 
	//	* Développer l'analyse des déclarations, assignations et appels de variables
	//		> Mis en pause, ça devient trop confus

	// #NOTES :
	//	* Pour clarifier la situation, je pense qu'il faut une phase d'analyse sémantique plus robuste :
	//		> Unicité des noms entre variable, fonction, struct et enum en fonction des scopes
	//		> Référencement des déclaration de variable et définition de fonction pour les sites d'appel
	//		> Référencement des définitions de types pour les utilisations de types
	//		> Génération de noms compilés (aka un uint unique) et référencement dans les nodes
	//			- Pour les variables membres d'une struct, le nom compilé correspond au nom compilé de l'instance de la struct + l'offset du membre
	//	* L'enjeu est d'avoir des metadata techniques qui permettent de déterminer les opérations à émettre à la compilation sans devoir explorer l'arbre AST ou faire des lookup dans des tables intermédiaires (ou le minimum)
	//		> Donc avoir les types complets là où c'est nécessaire
	//		> Pouvoir énumérer trivialement les membres d'une struct
	//		> Utiliser tel quel le nom compilé
	//	* Pour minimiser au maximum le recours à des structures de données supplémentaires, les algos d'analyse vont naviguer dans l'ast pour obtenir les informations nécessaires
	//	* Pour la même raison, on va ajouter une logique d'index chaîné entre les définitions de types pour parcourir plus rapidement
	//
	// #TODO :
	//	* Fonction générant la liste des types
	//		> Taille
	//		> "RuntimeType" : plutôt qu'une struct, c'est un index dans la map de struct
	//		> Algo récursif pour extraire la taille, il faudra détecter les références cycliques
	//		> Intègre les types natifs
	//	* Fonction affectant là où c'est pertinent le runtime type associé (aka l'index vers la liste des types)
	//	* Fonction générant le nom compilé des variables
	//		> Simple itération linéaire sur l'arbre ast et dès qu'on tombe sur une déclaration de variable (VARIABLE_DECLARATION et VARIABLE_DECLARATION_ASSIGNATION) on lui donne une index issu d'une seed qu'on incrémente
	//		> AH NON : le nom compilé va être utilisé pour définir un index local, il faut être plus fin :
	//			- Suivre l'imbrication des scopes
	//			- Prendre en compte la taille d'un type struct
	//	* Fonction détectant les doublons de noms
	//		> Entre struct, enum et fonction
	//		> Entre les noms de variables et les noms de struct/enum/fonction et déclarations de variables dans les scopes courant et supérieurs
	//	* Fonction associant le nom compilé d'une variable aux noeuds d'utilisation de cette variable


	////----------------------------------------
	//union MemoryBlock
	//{
	//	uint8_t		store[4];		// Utilisé pour stocker une valeur dans le buffer d'opérations
	//	uint32_t	index;			// Utilisé pour indexer une opération ou une adresse mémoire

	//	int32_t		i32;
	//	float		f32;
	//	bool		b;
	//	char		c8_4[4];		// pas de c32 pour l'instant : l'output console ne le gère pas et j'ai besoin de texte pour mes tests. Je gèrerai l'unicode plus tard (et peut être autrement que via des char32_t)
	//};

	////----------------------------------------
	//enum class OpCode : uint8_t
	//{
	//	CREATE_MEMORY,

	//	PUSH_FROM_MEMORY,
	//	POP_TO_MEMORY,

	//	PUSH_CONSTANT_INT,
	//	PUSH_CONSTANT_FLOAT,
	//	PUSH_CONSTANT_BOOL,

	//	POP,

	//	NEGATE_INT,
	//	NEGATE_FLOAT,
	//	NEGATE_BOOL,

	//	CAST_INT_TO_FLOAT,
	//	CAST_FLOAT_TO_INT,

	//	ADD_INT,
	//	ADD_FLOAT,
	//	SUBSTRACT_INT,
	//	SUBSTRACT_FLOAT,
	//	MULTIPLY_INT,
	//	MULTIPLY_FLOAT,
	//	DIVIDE_INT,
	//	DIVIDE_FLOAT,

	//	CALL_FUNCTION,		// Utilise la valeur sur la stack pour savoir où sauter
	//	EXIT_FUNCTION,		// Utilise la valeur stockée dans la structure de donnée spécifique à la callstack runtime

	//	DEBUG_PRINT_INT,
	//	DEBUG_PRINT_FLOAT,
	//	DEBUG_PRINT_BOOL,

	//	EXIT		// Fin de programme
	//};

	////----------------------------------------
	//struct FunctionDefRef
	//{
	//	token_index Name;
	//	uint32_t FirstOperation;
	//};

	////----------------------------------------
	//struct CompilationData
	//{
	//	FunctionDefRef FunctionRefs[128];
	//	uint32_t FunctionRefCount{ 0 };

	//	uint32_t MainNameToken{ 0 };
	//};

	////----------------------------------------
	//struct CompiledProgram
	//{
	//	MemoryBlock Constants[1024];
	//	OpCode Ops[1024];
	//	uint32_t OpCount{ 0 };
	//};

	////----------------------------------------
	//struct CallFrame
	//{
	//	uint32_t CallSite;
	//	uint32_t MemoryStackCursor;
	//	uint32_t NamedMemoryBaseIndex;		// Sert de décalage d'index pour éviter d'écraser des noms locaux entre 2 appels récursifs d'une même méthode par exemple
	//};

	////----------------------------------------
	//struct ExecutionContext
	//{
	//	MemoryBlock ExecutionStack[1024];
	//	uint32_t ExecutionStackCursor{ 0 };

	//	CallFrame CallStack[16];
	//	uint32_t CallStackCursor{ 0 };

	//	MemoryBlock MemoryStack[1024];
	//	uint32_t MemoryStackCursor{ 0 };

	//	uint32_t NamedMemoryMap[1024];
	//	uint32_t CurrentMaxName{ 0 };		// Utilisé pour identifier le nom le plus élevé utilisé. Sert à définir la base de noms d'une sous-fonction
	//};

	//bool CompileAST( const FileData& file_data, const TokenCollection& tokens, const ASTNodeCollection& ast, CompiledProgram& out_program, CompilationData& out_compilation_data );
	//
	//void PrintProgram( const CompiledProgram& program );
	//void PrintExecutionStack( const ExecutionContext& context );
	//void PrintCallStack( const FileData& file_data, const TokenCollection& tokens, const CompiledProgram& program, const ExecutionContext& context, const CompilationData& compilation_data );
	//
	//bool ExecuteProgram( const CompiledProgram& program, ExecutionContext& context );

#pragma endregion
}