#pragma once

#include <numeric>
#include <functional>

#include "Parser.h"		// Pour l'analyse de la signature des fonctions externes. C'est craspec mais je ne suis plus sûr de comment proprement isoler VM et compilation

//----------------------------------------
union MemoryBlock
{
	uint8_t		store[4];		// Utilisé pour stocker une valeur dans le buffer d'opérations
	uint32_t	index;			// Utilisé pour indexer une opération ou une adresse mémoire

	int32_t		i32;
	float		f32;
	bool		b;
	char		c;
	char		c8_4[4];		// pas de c32 pour l'instant : l'output console ne le gère pas et j'ai besoin de texte pour mes tests. Je gèrerai l'unicode plus tard (et peut être autrement que via des char32_t)
};

//----------------------------------------
enum class OpCode : uint8_t
{
	UNUSED,

	CREATE_MEMORY,

	RESERVE_MEMORY_STACK,		// Utilisé pour indiquer à l'exécution combien de variables nommées peuvent être utilisées dans la fonction. Ca permet d'avoir un curseur de memory stack à un emplacement libre pour faire des allocations dynamiques sur la stack si besoin

	COPY_MEMORY,

	PUSH_FROM_ADDRESS,
	POP_TO_ADDRESS,
	PUSH_LOCAL_ADDRESS,
	PUSH_GLOBAL_ADDRESS,
	OFFSET_ADDRESS,

	PUSH_CONSTANT_INDEX,
	PUSH_CONSTANT_INT,
	PUSH_CONSTANT_FLOAT,
	PUSH_CONSTANT_BOOL,
	PUSH_CONSTANT_STRING,

	POP,

	NEGATE_INT,
	NEGATE_FLOAT,
	NEGATE_BOOL,

	CAST_INT_TO_FLOAT,
	CAST_FLOAT_TO_INT,

	ADD_INT,
	ADD_FLOAT,
	SUBSTRACT_INT,
	SUBSTRACT_FLOAT,
	MULTIPLY_INT,
	MULTIPLY_FLOAT,
	DIVIDE_INT,
	DIVIDE_FLOAT,

	LESS_INT,
	LESS_FLOAT,
	LESS_EQUAL_INT,
	LESS_EQUAL_FLOAT,
	MORE_INT,
	MORE_FLOAT,
	MORE_EQUAL_INT,
	MORE_EQUAL_FLOAT,

	AND,
	OR,

	EQUALS_BOOL,
	EQUALS_INT,
	EQUALS_FLOAT,
	NOT_EQUALS_BOOL,
	NOT_EQUALS_INT,
	NOT_EQUALS_FLOAT,

	START_STRING_BUFFER,
	APPEND_BOOL_TO_STRING_BUFFER,
	APPEND_INT_TO_STRING_BUFFER,
	APPEND_FLOAT_TO_STRING_BUFFER,
	APPEND_CHAR_TO_STRING_BUFFER,
	APPEND_STRING_TO_STRING_BUFFER,
	END_STRING_BUFFER,

	JUMP,				// Utilise le paramètre suivant l'opérateur pour savoir où sauter
	JUMP_IF,			// Utilise la valeur sur la stack pour savoir si il faut sauter (ou plutôt savoir s'il ne faut PAS sauter, ça permet d'avoir les instructions du block if en premier) et le paramètre suivant l'opérateur pour savoir où sauter

	CALL_FUNCTION,		// Utilise le paramètre suivant l'opérateur pour savoir où sauter. La différence avec JUMP est que CALL_FUNCTION empile une frame sur la callstack
	EXIT_FUNCTION,		// Utilise la valeur stockée dans la structure de donnée spécifique à la callstack runtime

	CALL_EXTERNAL_FUNCTION,

	DEBUG_PRINT_INT,
	DEBUG_PRINT_FLOAT,
	DEBUG_PRINT_BOOL,
	DEBUG_PRINT_CHAR,
	DEBUG_PRINT_STRING,

	EXIT		// Fin de programme
};

const char* GetOpCodeName(OpCode op_code);

//----------------------------------------
struct CompiledProgram
{
	MemoryBlock Constants[1024];
	uint32_t ConstantTop{ 0 };

	OpCode Ops[1024];
	uint32_t OpCount{ 0 };
};

//----------------------------------------
struct IExternalFunction
{
	virtual ~IExternalFunction() {};

	virtual const char* GetName() const = 0;

	virtual uint32_t GetArgumentCount() const = 0;
	virtual RuntimeTypeId GetArgumentType(uint32_t argument_index) const = 0;

	virtual RuntimeTypeId GetReturnType() const = 0;

	virtual void Execute(MemoryBlock* exe_stack, uint32_t& exe_stack_top) const = 0;
};

//----------------------------------------
struct ExternalFunctionCatalog
{
	IExternalFunction* Catalog[64];
	uint32_t Count{ 0 };
};

#pragma region Test template helpers for external function

//----------------------------------------
template<typename T>
RuntimeTypeId GetTypeCategory()
{
	static_assert(false, "invalid type");
}

//----------------------------------------
template<typename R, typename A1>
bool CheckFunctionArguments(const ASTNodeCollection& ast, ast_node_index first_argument_index)
{
	if (first_argument_index == 0)
		return false;

	const ASTNode& firstArgumentNode = ast.Nodes[first_argument_index];

	return firstArgumentNode.NativeTypeId != GetTypeCategory<A1>();
}

//----------------------------------------
template<typename T>
void ReadFromMemoryBlock(const MemoryBlock& mb, T& out_value)
{
	static_assert(false, "invalid type");
}

//----------------------------------------
template<typename T>
void WriteToMemoryBlock(MemoryBlock& mb, const T& value)
{
	static_assert(false, "invalid type");
}

//----------------------------------------
template<typename R>
void RunFunction(const std::function<R()>& func, MemoryBlock* exe_stack, uint32_t& exe_stack_cursor)
{
	R result = func();

	MemoryBlock& resultMB = exe_stack[exe_stack_cursor];

	WriteToMemoryBlock(mb, resultMB);

	++exe_stack_cursor;
}

//----------------------------------------
template<typename R, typename A1>
void RunFunction(const std::function<R(const A1&)>& func, MemoryBlock* exe_stack, uint32_t& exe_stack_cursor)
{
	MemoryBlock& a1MB = exe_stack[exe_stack_cursor - 1];
	--exe_stack_cursor;

	A1 a1;
	ReadFromMemoryBlock(a1MB, a1);

	R result = func(a1);

	MemoryBlock& resultMB = exe_stack[exe_stack_cursor];
	WriteToMemoryBlock(mb, resultMB);

	++exe_stack_cursor;
}

//----------------------------------------
template<typename R, typename A1, typename A2>
void RunFunction(const std::function<R(const A1&, const A2&)>& func, MemoryBlock* exe_stack, uint32_t& exe_stack_cursor)
{
	MemoryBlock& a1MB = exe_stack[exe_stack_cursor - 1];
	--exe_stack_cursor;

	MemoryBlock& a2MB = exe_stack[exe_stack_cursor - 1];
	--exe_stack_cursor;

	A1 a1;
	ReadFromMemoryBlock(a1MB, a1);
	A1 a2;
	ReadFromMemoryBlock(a2MB, a2);

	R result = func(a1, a2);

	MemoryBlock& resultMB = exe_stack[exe_stack_cursor];
	WriteToMemoryBlock(mb, resultMB);

	++exe_stack_cursor;
}

//----------------------------------------
template<typename A1>
void RunFunction(const std::function<void(const A1&)>& func, MemoryBlock* exe_stack, uint32_t& exe_stack_cursor)
{
	MemoryBlock& a1MB = exe_stack[exe_stack_cursor - 1];
	--exe_stack_cursor;

	A1 a1;
	ReadFromMemoryBlock(a1MB, a1);

	func(a1);
}

//----------------------------------------
template<typename A1, typename A2>
void RunFunction(const std::function<void(const A1&, const A2&)>& func, MemoryBlock* exe_stack, uint32_t& exe_stack_cursor)
{
	MemoryBlock& a1MB = exe_stack[exe_stack_cursor - 1];
	--exe_stack_cursor;

	MemoryBlock& a2MB = exe_stack[exe_stack_cursor - 1];
	--exe_stack_cursor;

	A1 a1;
	ReadFromMemoryBlock(a1MB, a1);
	A1 a2;
	ReadFromMemoryBlock(a2MB, a2);

	result = func(a1, a2);
}


#pragma endregion

#pragma region Standard Library

void SetupStandardLibrary(ExternalFunctionCatalog& catalog);
void ClearFunctionCatalog(ExternalFunctionCatalog& catalog);

#pragma endregion

//----------------------------------------
struct CallFrame
{
	uint32_t CallSite;
	uint32_t MemoryBaseAddress;
	uint32_t CurrentBufferIndex;
};

//----------------------------------------
struct ExecutionContext
{
	ExternalFunctionCatalog ExternalFunctions;

	MemoryBlock ExecutionStack[1024];
	uint32_t ExecutionStackCursor{ 0 };

	CallFrame CallStack[64];
	uint32_t CallStackCursor{ 0 };

	MemoryBlock MemoryStack[2048];
	// #NOTE: la valeur est correctement gérée mais elle n'est pas utilisée
	// elle servirait dans l'optique d'une allocation dynamique sur la stack (par exemple pour faire de la concaténation de string)
	// et représente l'adresse mémoire disponible minimale au delà de la plage d'adresse réservée pour la callframe courante et parentes
	// Actuellement on s'en sert juste pour sauvegarder et restaurer l'espace mémoire effectivement utilisé par une callframe (à savoir la taille cumulée des variables de la frame)
	uint32_t MemoryStackDynamicAllocCursor{ 0 };
};

bool ExecuteProgram( const CompiledProgram& program, ExecutionContext& context);

void PrintMemory(uint32_t count, const MemoryBlock* memory, bool reverse_order = true);
//void PrintStringConstant(const CompiledProgram& program, uint32_t constant_address, bool full_debug = false);

uint32_t PrintOperation(const CompiledProgram& program, uint32_t op_index);
void PrintProgram(const CompiledProgram& program);

void PrintExecutionStack( const ExecutionContext& context );
void PrintMemoryStack( const ExecutionContext& context );
