#include "VirtualMachine.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "Compiler.h"		// Ajouté ici pour résoudre des problèmes de définition de types. Si on veut faire les choses proprement, il faudrait construire une structure de donnée ad-hoc stockant les informations nécessaires au debug et s'en servir ici et le générer dans le Compiler



template<>
RuntimeTypeId GetTypeCategory<bool>() { return SemanticAnalyzisData::BOOL_TypeId; }
template<>
RuntimeTypeId GetTypeCategory<int32_t>() { return SemanticAnalyzisData::INT_TypeId; }
template<>
RuntimeTypeId GetTypeCategory<float>() { return SemanticAnalyzisData::FLOAT_TypeId; }


template<>
void ReadFromMemoryBlock<bool>(const MemoryBlock& mb, bool& out_value) { out_value = mb.b; }
template<>
void ReadFromMemoryBlock<int32_t>(const MemoryBlock& mb, int32_t& out_value) { out_value = mb.i32; }
template<>
void ReadFromMemoryBlock<float>(const MemoryBlock& mb, float& out_value) { out_value = mb.f32; }


template<>
void WriteToMemoryBlock<bool>(MemoryBlock& mb, const bool& value) { mb.b = value; }
template<>
void WriteToMemoryBlock<int32_t>(MemoryBlock& mb, const int32_t& value) { mb.i32 = value; }
template<>
void WriteToMemoryBlock<float>(MemoryBlock& mb, const float& value) { mb.f32 = value; }


#pragma region Standard Library

//----------------------------------------
struct STD_PrintNewLine : public IExternalFunction
{
	~STD_PrintNewLine() {}

	const char* GetName() const override { return "println"; }

	uint32_t GetArgumentCount() const override { return 0; }
	RuntimeTypeId GetArgumentType(uint32_t argument_index) const override { return SemanticAnalyzisData::VOID_TypeId; }

	RuntimeTypeId GetReturnType() const override { return SemanticAnalyzisData::VOID_TypeId; }

	void Execute(MemoryBlock* exe_stack, uint32_t& exe_stack_top) const override { std::cout << "\n"; }
};

//----------------------------------------
struct STD_GetInt : public IExternalFunction
{
	~STD_GetInt() {}

	const char* GetName() const override { return "get_int"; }

	uint32_t GetArgumentCount() const override { return 0; }
	RuntimeTypeId GetArgumentType(uint32_t argument_index) const override { return SemanticAnalyzisData::VOID_TypeId; }

	RuntimeTypeId GetReturnType() const override { return SemanticAnalyzisData::INT_TypeId; }

	void Execute(MemoryBlock* exe_stack, uint32_t& exe_stack_top) const override
	{
		MemoryBlock& mb = exe_stack[exe_stack_top];

		std::cin >> mb.i32;

		++exe_stack_top;
	}
};

//----------------------------------------
struct STD_GetFloat : public IExternalFunction
{
	~STD_GetFloat() {}

	const char* GetName() const override { return "get_float"; }

	uint32_t GetArgumentCount() const override { return 0; }
	RuntimeTypeId GetArgumentType(uint32_t argument_index) const override { return SemanticAnalyzisData::VOID_TypeId; }

	RuntimeTypeId GetReturnType() const override { return SemanticAnalyzisData::FLOAT_TypeId; }

	void Execute(MemoryBlock* exe_stack, uint32_t& exe_stack_top) const override
	{
		MemoryBlock& mb = exe_stack[exe_stack_top];

		std::cin >> mb.f32;

		++exe_stack_top;
	}
};

//----------------------------------------
struct STD_Wait : public IExternalFunction
{
	~STD_Wait() {}

	const char* GetName() const override { return "wait"; }

	uint32_t GetArgumentCount() const override { return 1; }
	RuntimeTypeId GetArgumentType(uint32_t argument_index) const override
	{
		if (argument_index == 0)
			return SemanticAnalyzisData::FLOAT_TypeId;

		return SemanticAnalyzisData::VOID_TypeId;
	}

	RuntimeTypeId GetReturnType() const override { return SemanticAnalyzisData::VOID_TypeId; }

	void Execute(MemoryBlock* exe_stack, uint32_t& exe_stack_top) const override
	{
		float duration = exe_stack[exe_stack_top - 1].f32;
		--exe_stack_top;

		if (duration > 0.0f)
			std::this_thread::sleep_for(std::chrono::milliseconds((int32_t)(duration * 1000)));
	}
};


//----------------------------------------
struct ISTD_MonoFloat : public IExternalFunction
{
	~ISTD_MonoFloat() {}

	uint32_t GetArgumentCount() const override { return 1; }
	RuntimeTypeId GetArgumentType(uint32_t argument_index) const override
	{
		if (argument_index == 0)
			return SemanticAnalyzisData::FLOAT_TypeId;

		return SemanticAnalyzisData::VOID_TypeId;
	}

	RuntimeTypeId GetReturnType() const override { return SemanticAnalyzisData::FLOAT_TypeId; }

	void Execute(MemoryBlock* exe_stack, uint32_t& exe_stack_top) const override { exe_stack[exe_stack_top - 1].f32 = ExecuteImpl(exe_stack[exe_stack_top - 1].f32); }
	virtual float ExecuteImpl(float value) const = 0;
};


//----------------------------------------
struct STD_Square : public ISTD_MonoFloat
{
	const char* GetName() const override { return "sqr"; }
	float ExecuteImpl(float value) const { return value * value; }
};

//----------------------------------------
struct STD_SquareRoot : public ISTD_MonoFloat
{
	const char* GetName() const override { return "sqrt"; }
	float ExecuteImpl(float value) const { return sqrtf(value); }
};

//----------------------------------------
struct STD_Cos : public ISTD_MonoFloat
{
	const char* GetName() const override { return "cos"; }
	float ExecuteImpl(float value) const { return cosf(value); }
};

//----------------------------------------
struct STD_Sin : public ISTD_MonoFloat
{
	const char* GetName() const override { return "sin"; }
	float ExecuteImpl(float value) const { return sinf(value); }
};

//----------------------------------------
struct STD_Tan : public ISTD_MonoFloat
{
	const char* GetName() const override { return "tan"; }
	float ExecuteImpl(float value) const { return tanf(value); }
};





//----------------------------------------
template<typename T>
void AddStandardLibraryFunction(ExternalFunctionCatalog& catalog)
{
	if (catalog.Count >= sizeof(catalog.Catalog) / sizeof(catalog.Catalog[0]))
		throw "Out of bounds!";

	catalog.Catalog[catalog.Count] = new T();
	++catalog.Count;
}

//----------------------------------------
void SetupStandardLibrary(ExternalFunctionCatalog& catalog)
{
	AddStandardLibraryFunction<STD_PrintNewLine>(catalog);
	AddStandardLibraryFunction<STD_Wait>(catalog);
	AddStandardLibraryFunction<STD_GetInt>(catalog);
	AddStandardLibraryFunction<STD_GetFloat>(catalog);

	AddStandardLibraryFunction<STD_Square>(catalog);
	AddStandardLibraryFunction<STD_SquareRoot>(catalog);
	AddStandardLibraryFunction<STD_Cos>(catalog);
	AddStandardLibraryFunction<STD_Sin>(catalog);
	AddStandardLibraryFunction<STD_Tan>(catalog);
}

//----------------------------------------
void ClearFunctionCatalog(ExternalFunctionCatalog& catalog)
{
	for (uint32_t i = 0; i < catalog.Count; ++i)
	{
		delete catalog.Catalog[i];
		catalog.Catalog[i] = nullptr;
	}

	catalog.Count = 0;
}

#pragma endregion




static const char* l_opCodeNames[] =
{
	"UNUSED",

	"CREATE_MEMORY",

	"RESERVE_MEMORY_STACK",

	"COPY_MEMORY",

	"PUSH_FROM_ADDRESS",
	"POP_TO_ADDRESS",
	"PUSH_LOCAL_ADDRESS_2",
	"PUSH_GLOBAL_ADDRESS_2",
	"OFFSET_ADDRESS",

	"PUSH_CONSTANT_INDEX",
	"PUSH_CONSTANT_INT",
	"PUSH_CONSTANT_FLOAT",
	"PUSH_CONSTANT_BOOL",
	"PUSH_CONSTANT_STRING",

	"POP",

	"NEGATE_INT",
	"NEGATE_FLOAT",
	"NEGATE_BOOL",

	"CAST_INT_TO_FLOAT",
	"CAST_FLOAT_TO_INT",

	"ADD_INT",
	"ADD_FLOAT",
	"SUBSTRACT_INT",
	"SUBSTRACT_FLOAT",
	"MULTIPLY_INT",
	"MULTIPLY_FLOAT",
	"DIVIDE_INT",
	"DIVIDE_FLOAT",

	"LESS_INT",
	"LESS_FLOAT",
	"LESS_EQUAL_INT",
	"LESS_EQUAL_FLOAT",
	"MORE_INT",
	"MORE_FLOAT",
	"MORE_EQUAL_INT",
	"MORE_EQUAL_FLOAT",

	"AND",
	"OR",

	"EQUALS_BOOL",
	"EQUALS_INT",
	"EQUALS_FLOAT",
	"NOT_EQUALS_BOOL",
	"NOT_EQUALS_INT",
	"NOT_EQUALS_FLOAT",

	"START_STRING_BUFFER",
	"APPEND_BOOL_TO_STRING_BUFFER",
	"APPEND_INT_TO_STRING_BUFFER",
	"APPEND_FLOAT_TO_STRING_BUFFER",
	"APPEND_CHAR_TO_STRING_BUFFER",
	"APPEND_STRING_TO_STRING_BUFFER",
	"END_STRING_BUFFER",

	"JUMP",
	"JUMP_IF",

	"CALL_FUNCTION",
	"EXIT_FUNCTION",

	"CALL_EXTERNAL_FUNCTION",

	"DEBUG_PRINT_INT",
	"DEBUG_PRINT_FLOAT",
	"DEBUG_PRINT_BOOL",
	"DEBUG_PRINT_CHAR",
	"DEBUG_PRINT_STRING",

	"EXIT"
};

static uint32_t l_opCodeNamesCount = sizeof(l_opCodeNames) / sizeof(l_opCodeNames[0]);

//----------------------------------------
const char* GetOpCodeName(OpCode op_code)
{
	if ((uint32_t)op_code < l_opCodeNamesCount)
		return l_opCodeNames[(uint32_t)op_code];
	else
		return "#ERROR INVALID OpCode";
}
//----------------------------------------
static MemoryBlock GetParameter(const CompiledProgram& program, uint32_t cursor, uint32_t parameter_index)
{
	MemoryBlock mb;

	cursor += parameter_index * 4;

	mb.store[0] = (uint8_t)program.Ops[cursor + 1];
	mb.store[1] = (uint8_t)program.Ops[cursor + 2];
	mb.store[2] = (uint8_t)program.Ops[cursor + 3];
	mb.store[3] = (uint8_t)program.Ops[cursor + 4];

	return mb;
}

//----------------------------------------
static void CopyCharacter(MemoryBlock* memory, uint32_t from_buffer_index, uint32_t from_buffer_char_index, uint32_t to_buffer_index, uint32_t to_buffer_char_index)
{
	uint32_t fromMemoryBlockIndex = from_buffer_index + 1 + from_buffer_char_index / 4;
	uint32_t fromMemoryCharIndex = from_buffer_char_index % 4;

	uint32_t toMemoryBlockIndex = to_buffer_index + 1 + to_buffer_char_index / 4;
	uint32_t toMemoryCharIndex = to_buffer_char_index % 4;

	memory[toMemoryBlockIndex].c8_4[toMemoryCharIndex] = memory[fromMemoryBlockIndex].c8_4[fromMemoryCharIndex];
}

//----------------------------------------
static void CopyCharacter(MemoryBlock* memory, char c, uint32_t to_buffer_index, uint32_t to_buffer_char_index)
{
	uint32_t toMemoryBlockIndex = to_buffer_index + 1 + to_buffer_char_index / 4;
	uint32_t toMemoryCharIndex = to_buffer_char_index % 4;

	memory[toMemoryBlockIndex].c8_4[toMemoryCharIndex] = c;
}

//----------------------------------------
static void PrintMemoryString(const ExecutionContext& context, uint32_t constant_address)
{
	const MemoryBlock& header = context.MemoryStack[constant_address];

	uint32_t read = 0;
	uint32_t memoryAddress = constant_address + 1;
	while (read < header.index)
	{
		const MemoryBlock& str = context.MemoryStack[memoryAddress];

		uint32_t subRead = 0;
		while (subRead < 4 && read + subRead < header.index)
		{
			std::cout << str.c8_4[subRead];
			++subRead;
		}

		read += subRead;
		++memoryAddress;
	}
}

//----------------------------------------
static void PrintStringConstant(const CompiledProgram& program, uint32_t constant_address)
{
	const MemoryBlock& header = program.Constants[constant_address];

	uint32_t read = 0;
	uint32_t memoryAddress = constant_address + 1;
	while (read < header.index)
	{
		const MemoryBlock& str = program.Constants[memoryAddress];

		uint32_t subRead = 0;
		while (subRead < 4 && read + subRead < header.index)
		{
			std::cout << str.c8_4[subRead];
			++subRead;
		}

		read += subRead;
		++memoryAddress;
	}
}


//----------------------------------------
bool ExecuteProgram( const CompiledProgram& program, ExecutionContext& context)
{
	uint32_t cursor = 0;

	uint32_t stackSize = sizeof(context.CallStack) / sizeof(context.CallStack[0]);

	// On copie dans la mémoire du contexte le contenu des constantes. Ca permet d'avoir un espace d'adressage homogène entre constantes et variables dynamiques (notamment pour l'adressage des strings)
	for (uint32_t i = 0; i < program.ConstantTop; ++i)
	{
		context.MemoryStack[i] = program.Constants[i];
	}
	context.MemoryStackDynamicAllocCursor = program.ConstantTop;

	// CallFrame par défaut
	CallFrame& callFrame = context.CallStack[context.CallStackCursor];
	callFrame.CallSite = cursor;
	callFrame.MemoryBaseAddress = context.MemoryStackDynamicAllocCursor;
	++context.CallStackCursor;

	while( cursor < program.OpCount )
	{
		switch( program.Ops[cursor] )
		{
			case OpCode::UNUSED:
			{
				std::cout << "Unexpected UNUSED operation.\n";
				return false;
			}
			break;
			case OpCode::CREATE_MEMORY:
			{
				MemoryBlock mb = GetParameter(program, cursor, 0);

				uint32_t varLocalAddressIndex = context.CallStack[context.CallStackCursor - 1].MemoryBaseAddress + mb.index;

				mb = GetParameter(program, cursor, 1);

				uint32_t size = mb.index;

				for(uint32_t i = 0; i < size; ++i)
					context.MemoryStack[varLocalAddressIndex + i].i32 = 0;	// init universel

				cursor += 9;
			}
			break;
			case OpCode::RESERVE_MEMORY_STACK:
			{
				MemoryBlock reservedSize = GetParameter(program, cursor, 0);

				context.MemoryStackDynamicAllocCursor += reservedSize.index;

				cursor += 5;
			}
			break;
			case OpCode::COPY_MEMORY:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}

				MemoryBlock size = GetParameter(program, cursor, 0);

				uint32_t addressTo = context.ExecutionStack[context.ExecutionStackCursor - 1].index;
				uint32_t addressFrom = context.ExecutionStack[context.ExecutionStackCursor - 2].index;

				for (uint32_t i = 0; i < size.index; ++i)
				{
					context.MemoryStack[addressTo + i] = context.MemoryStack[addressFrom + i];
				}

				context.ExecutionStackCursor -= 2;
				cursor += 5;
			}
			break;

			case OpCode::PUSH_FROM_ADDRESS:
			{
				uint32_t globalAddress = context.ExecutionStack[context.ExecutionStackCursor - 1].index;
				MemoryBlock& memory = context.MemoryStack[globalAddress];

				context.ExecutionStack[context.ExecutionStackCursor - 1] = memory;

				++cursor;
			}
			break;
			case OpCode::POP_TO_ADDRESS:
			{
				uint32_t globalAddress = context.ExecutionStack[context.ExecutionStackCursor - 1].index;
				MemoryBlock& value = context.ExecutionStack[context.ExecutionStackCursor - 2];

				context.MemoryStack[globalAddress] = value;

				context.ExecutionStackCursor -= 2;
				++cursor;
			}
			break;
			case OpCode::PUSH_LOCAL_ADDRESS:
			{
				uint32_t localAddress = GetParameter(program, cursor, 0).index;
				uint32_t globalAddress = context.CallStack[context.CallStackCursor - 1].MemoryBaseAddress + localAddress;

				MemoryBlock& globalAddressMB = context.ExecutionStack[context.ExecutionStackCursor];
				globalAddressMB.index = globalAddress;

				++context.ExecutionStackCursor;
				cursor += 5;
			}
			break;
			case OpCode::PUSH_GLOBAL_ADDRESS:
			{
				uint32_t globalAddress = GetParameter(program, cursor, 0).index;

				MemoryBlock& globalAddressMB = context.ExecutionStack[context.ExecutionStackCursor];
				globalAddressMB.index = globalAddress;

				++context.ExecutionStackCursor;
				cursor += 5;
			}
			break;
			case OpCode::OFFSET_ADDRESS:
			{
				MemoryBlock& offsetMB = context.ExecutionStack[context.ExecutionStackCursor - 1];
				MemoryBlock& globalAddressMB = context.ExecutionStack[context.ExecutionStackCursor - 2];
				//uint32_t offset = GetParameter(program, cursor, 0).index;

				globalAddressMB.index += offsetMB.index;

				--context.ExecutionStackCursor;
				++cursor;
			}
			break;

			case OpCode::PUSH_CONSTANT_INDEX:
			case OpCode::PUSH_CONSTANT_INT:
			case OpCode::PUSH_CONSTANT_FLOAT:
			case OpCode::PUSH_CONSTANT_STRING:
			{
				MemoryBlock& mb = context.ExecutionStack[context.ExecutionStackCursor];

				mb = GetParameter(program, cursor, 0);

				++context.ExecutionStackCursor;
				cursor += 5;
			}
			break;
			case OpCode::PUSH_CONSTANT_BOOL:
			{
				MemoryBlock& mb = context.ExecutionStack[context.ExecutionStackCursor];
				mb.b = (uint8_t) program.Ops[cursor + 1] != 0;

				context.ExecutionStackCursor++;
				cursor += 2;
			}
			break;

			case OpCode::POP:
			{
				context.ExecutionStackCursor--;
				++cursor;
			}
			break;
			case OpCode::NEGATE_FLOAT:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 1].f32 *= -1.0f;
				++cursor;
			}
			break;
			case OpCode::NEGATE_INT:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 1].i32 *= -1;
				++cursor;
			}
			break;
			case OpCode::NEGATE_BOOL:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 1].b ^= true;
				++cursor;
			}
			break;
			case OpCode::CAST_INT_TO_FLOAT:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 1].f32 = (float) context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				++cursor;
			}
			break;
			case OpCode::CAST_FLOAT_TO_INT:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 1].i32 = (int32_t) context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				++cursor;
			}
			break;
			case OpCode::ADD_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 += context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::ADD_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 += context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::SUBSTRACT_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 -= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::SUBSTRACT_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 -= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::MULTIPLY_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 *= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::MULTIPLY_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 *= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::DIVIDE_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].i32 /= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::DIVIDE_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].f32 /= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;

			case OpCode::LESS_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].i32 < context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::LESS_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].f32 < context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::LESS_EQUAL_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].i32 <= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::LESS_EQUAL_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].f32 <= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::MORE_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].i32 > context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::MORE_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].f32 > context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::MORE_EQUAL_INT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].i32 >= context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::MORE_EQUAL_FLOAT:
			{
				if( context.ExecutionStackCursor < 2 )
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].f32 >= context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;

			case OpCode::AND:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].b && context.ExecutionStack[context.ExecutionStackCursor - 1].b;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::OR:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].b || context.ExecutionStack[context.ExecutionStackCursor - 1].b;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;

			case OpCode::EQUALS_BOOL:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].b == context.ExecutionStack[context.ExecutionStackCursor - 1].b;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::EQUALS_INT:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].i32 == context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::EQUALS_FLOAT:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].f32 == context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;

			case OpCode::NOT_EQUALS_BOOL:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].b != context.ExecutionStack[context.ExecutionStackCursor - 1].b;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::NOT_EQUALS_INT:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].i32 != context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::NOT_EQUALS_FLOAT:
			{
				if (context.ExecutionStackCursor < 2)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				context.ExecutionStack[context.ExecutionStackCursor - 2].b = context.ExecutionStack[context.ExecutionStackCursor - 2].f32 != context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				--context.ExecutionStackCursor;
				++cursor;
			}
			break;

			case OpCode::START_STRING_BUFFER:
			{
				MemoryBlock& mb = context.ExecutionStack[context.ExecutionStackCursor];
				mb.index = context.MemoryStackDynamicAllocCursor;
				MemoryBlock& buffer = context.MemoryStack[mb.index];
				buffer.index = 0;
				context.CallStack[context.CallStackCursor - 1].CurrentBufferIndex = mb.index;

				++context.MemoryStackDynamicAllocCursor;	// pour le header du buffer
				++context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::APPEND_BOOL_TO_STRING_BUFFER:
			{
				MemoryBlock& boolValue = context.ExecutionStack[context.ExecutionStackCursor - 1];

				uint32_t toBufferIndex = context.CallStack[context.CallStackCursor - 1].CurrentBufferIndex;
				MemoryBlock& toBuffer = context.MemoryStack[toBufferIndex];
				uint32_t toBufferCurrentSize = toBuffer.index;

				if (boolValue.b)
				{
					CopyCharacter(context.MemoryStack, 't', toBufferIndex, toBufferCurrentSize);
					CopyCharacter(context.MemoryStack, 'r', toBufferIndex, toBufferCurrentSize + 1);
					CopyCharacter(context.MemoryStack, 'u', toBufferIndex, toBufferCurrentSize + 2);
					CopyCharacter(context.MemoryStack, 'e', toBufferIndex, toBufferCurrentSize + 3);

					toBuffer.index += 4;
					context.MemoryStackDynamicAllocCursor += 4;
				}
				else
				{
					CopyCharacter(context.MemoryStack, 'f', toBufferIndex, toBufferCurrentSize);
					CopyCharacter(context.MemoryStack, 'a', toBufferIndex, toBufferCurrentSize + 1);
					CopyCharacter(context.MemoryStack, 'l', toBufferIndex, toBufferCurrentSize + 2);
					CopyCharacter(context.MemoryStack, 's', toBufferIndex, toBufferCurrentSize + 3);
					CopyCharacter(context.MemoryStack, 'e', toBufferIndex, toBufferCurrentSize + 4);

					toBuffer.index += 5;
					context.MemoryStackDynamicAllocCursor += 5;
				}

				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::APPEND_CHAR_TO_STRING_BUFFER:
			{
				MemoryBlock& charValue = context.ExecutionStack[context.ExecutionStackCursor - 1];

				uint32_t toBufferIndex = context.CallStack[context.CallStackCursor - 1].CurrentBufferIndex;
				MemoryBlock& toBuffer = context.MemoryStack[toBufferIndex];
				uint32_t toBufferCurrentSize = toBuffer.index;

				CopyCharacter(context.MemoryStack, charValue.c, toBufferIndex, toBufferCurrentSize);
				
				++toBuffer.index;
				++context.MemoryStackDynamicAllocCursor;

				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::APPEND_INT_TO_STRING_BUFFER:
			{
				MemoryBlock& intValue = context.ExecutionStack[context.ExecutionStackCursor - 1];

				uint32_t toBufferIndex = context.CallStack[context.CallStackCursor - 1].CurrentBufferIndex;
				MemoryBlock& toBuffer = context.MemoryStack[toBufferIndex];
				uint32_t toBufferCurrentSize = toBuffer.index;

				char formatBuffer[256];

				int size = sprintf_s<256>(formatBuffer, "%i", intValue.i32);

				for (int i = 0; i < size; ++i)
				{
					CopyCharacter(context.MemoryStack, formatBuffer[i], toBufferIndex, toBufferCurrentSize + i);
				}

				toBuffer.index += size;
				context.MemoryStackDynamicAllocCursor += size;

				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::APPEND_FLOAT_TO_STRING_BUFFER:
			{
				MemoryBlock& floatValue = context.ExecutionStack[context.ExecutionStackCursor - 1];

				uint32_t toBufferIndex = context.CallStack[context.CallStackCursor - 1].CurrentBufferIndex;
				MemoryBlock& toBuffer = context.MemoryStack[toBufferIndex];
				uint32_t toBufferCurrentSize = toBuffer.index;

				char formatBuffer[256];

				int size = sprintf_s<256>(formatBuffer, "%f", floatValue.f32);

				int sizeWithoutTrailingZeroes = size;

				while (sizeWithoutTrailingZeroes > 0 && formatBuffer[sizeWithoutTrailingZeroes - 1] == '0')
					--sizeWithoutTrailingZeroes;

				for (int i = 0; i < sizeWithoutTrailingZeroes; ++i)
				{
					CopyCharacter(context.MemoryStack, formatBuffer[i], toBufferIndex, toBufferCurrentSize + i);
				}

				toBuffer.index += sizeWithoutTrailingZeroes;
				context.MemoryStackDynamicAllocCursor += sizeWithoutTrailingZeroes;

				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::APPEND_STRING_TO_STRING_BUFFER:
			{
				MemoryBlock& stringAddress = context.ExecutionStack[context.ExecutionStackCursor - 1];

				uint32_t fromBufferIndex = stringAddress.index;
				MemoryBlock& fromBuffer = context.MemoryStack[fromBufferIndex];
				uint32_t fromBufferSize = fromBuffer.index;

				uint32_t toBufferIndex = context.CallStack[context.CallStackCursor - 1].CurrentBufferIndex;
				MemoryBlock& toBuffer = context.MemoryStack[toBufferIndex];
				uint32_t toBufferCurrentSize = toBuffer.index;

				for (uint32_t i = 0; i < fromBufferSize; ++i)
				{
					CopyCharacter(context.MemoryStack, fromBufferIndex, i, toBufferIndex, toBufferCurrentSize + i);
				}

				toBuffer.index += fromBufferSize;
				context.MemoryStackDynamicAllocCursor += fromBufferSize;

				--context.ExecutionStackCursor;
				++cursor;
			}
			break;
			case OpCode::END_STRING_BUFFER:
			{
				MemoryBlock& mb = context.ExecutionStack[context.ExecutionStackCursor - 1];
				MemoryBlock& buffer = context.MemoryStack[mb.index];

				++cursor;
			}
			break;

			case OpCode::JUMP:
			{
				MemoryBlock mb = GetParameter(program, cursor, 0);

				cursor = mb.index;
			}
			break;
			case OpCode::JUMP_IF:
			{
				if (context.ExecutionStackCursor < 1)
				{
					std::cout << "[ERROR] Insufficient stack for operation " << cursor << "\n";
					return false;
				}
				
				if(context.ExecutionStack[context.ExecutionStackCursor - 1].b)
					cursor += 5;
				else
				{
					MemoryBlock mb = GetParameter(program, cursor, 0);

					cursor = mb.index;
				}

				--context.ExecutionStackCursor;
			}
			break;

			case OpCode::CALL_FUNCTION:
			{
				if( context.CallStackCursor >= stackSize )
				{
					std::cout << "[ERROR] Stack overflow for operation " << cursor << "\n";
					return false;
				}

				CallFrame& callFrame = context.CallStack[context.CallStackCursor];
				callFrame.CallSite = cursor;
				callFrame.MemoryBaseAddress = context.MemoryStackDynamicAllocCursor;
				++context.CallStackCursor;

				MemoryBlock mb = GetParameter(program, cursor, 0);

				cursor = mb.index;
			}
			break;
			case OpCode::EXIT_FUNCTION:
			{
				if( context.CallStackCursor == 0 )
				{
					std::cout << "[ERROR] Stack underflow for operation " << cursor << "\n";
					return false;
				}

				--context.CallStackCursor;
				CallFrame& callFrame = context.CallStack[context.CallStackCursor];

				cursor = callFrame.CallSite + 5;	// +1 pour consommer l'opérateur d'appel de fonction, +4 pour les données d'index de curseur
				context.MemoryStackDynamicAllocCursor = callFrame.MemoryBaseAddress;
			}
			break;

			case OpCode::CALL_EXTERNAL_FUNCTION:
			{
				if (context.CallStackCursor >= stackSize)
				{
					std::cout << "[ERROR] Stack overflow for operation " << cursor << "\n";
					return false;
				}

				uint32_t functionIndex = GetParameter(program, cursor, 0).index;
				const IExternalFunction* func = context.ExternalFunctions.Catalog[functionIndex];

				func->Execute(context.ExecutionStack, context.ExecutionStackCursor);

				cursor += 5;
			}
			break;

			case OpCode::DEBUG_PRINT_INT:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				std::cout << context.ExecutionStack[context.ExecutionStackCursor - 1].i32;
				++cursor;
			}
			break;
			case OpCode::DEBUG_PRINT_FLOAT:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				std::cout << context.ExecutionStack[context.ExecutionStackCursor - 1].f32;
				++cursor;
			}
			break;
			case OpCode::DEBUG_PRINT_BOOL:
			{
				if( context.ExecutionStackCursor == 0 )
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				std::cout << ( context.ExecutionStack[context.ExecutionStackCursor - 1].b ? "true" : "false" );
				++cursor;
			}
			break;
			case OpCode::DEBUG_PRINT_CHAR:
			{
				if (context.ExecutionStackCursor == 0)
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				std::cout << context.ExecutionStack[context.ExecutionStackCursor - 1].c;
				++cursor;
			}
			break;
			case OpCode::DEBUG_PRINT_STRING:
			{
				if (context.ExecutionStackCursor == 0)
				{
					std::cout << "[ERROR] Empty stack for operation " << cursor << "\n";
					return false;
				}
				PrintMemoryString(context, context.ExecutionStack[context.ExecutionStackCursor - 1].index);
				++cursor;
			}
			break;
			case OpCode::EXIT:
			{
				if( context.CallStackCursor == 0 )
				{
					std::cout << "[ERROR] Stack underflow for operation " << cursor << "\n";
					return false;
				}

				// On referme proprement la CallFrame par défaut
				--context.CallStackCursor;
				context.MemoryStackDynamicAllocCursor = context.CallStack[context.CallStackCursor].MemoryBaseAddress;

				return true;
			}
			default:
				std::cout << "[ERROR] Unexpected op code " << (uint32_t) program.Ops[cursor] << " for operation " << cursor << "\n";
				return false;
		}
	}

	return true;
}


//----------------------------------------
static void PrintFunctionNameFromFirstOperation( const CompilationData& compilation_data, const TextData& text_data, const TokenCollection& tokens, uint32_t first_operation )
{
	for( uint32_t i = 0; i < compilation_data.FunctionRefCount; ++i )
	{
		const FunctionDefRef& funcRef = compilation_data.FunctionRefs[i];

		if( funcRef.FirstOperation == first_operation )
			PrintTokenValue( text_data, tokens, funcRef.Name, false);
	}
}

//----------------------------------------
uint32_t PrintOperation(const CompiledProgram& program, uint32_t op_index)
{
	std::cout << std::setw(4) << op_index << std::setw(0) << " " << std::setw(42);

	OpCode op = program.Ops[op_index];

	std::cout << GetOpCodeName(op);

	if (op == OpCode::CREATE_MEMORY)
	{
		MemoryBlock mb = GetParameter(program, op_index, 0);
		std::cout << " " << mb.index << " ";

		mb = GetParameter(program, op_index, 1);
		std::cout << " " << mb.index << "\n";
	}
	else if (op == OpCode::RESERVE_MEMORY_STACK || op == OpCode::COPY_MEMORY ||
		op == OpCode::PUSH_LOCAL_ADDRESS || op == OpCode::PUSH_GLOBAL_ADDRESS ||
		op == OpCode::JUMP || op == OpCode::JUMP_IF || op == OpCode::CALL_FUNCTION || op == OpCode::CALL_EXTERNAL_FUNCTION)
	{
		MemoryBlock mb = GetParameter(program, op_index, 0);
		std::cout << " " << mb.index << "\n";
	}
	else if (op == OpCode::PUSH_CONSTANT_BOOL)
	{
		std::cout << " " << ((uint32_t)program.Ops[op_index + 1] == 0 ? "false\n" : "true\n");
	}
	else if (op == OpCode::PUSH_CONSTANT_INDEX)
	{
		MemoryBlock mb = GetParameter(program, op_index, 0);
		std::cout << " " << mb.index << "\n";
	}
	else if (op == OpCode::PUSH_CONSTANT_INT)
	{
		MemoryBlock mb = GetParameter(program, op_index, 0);
		std::cout << " " << mb.i32 << "\n";
	}
	else if (op == OpCode::PUSH_CONSTANT_FLOAT)
	{
		MemoryBlock mb = GetParameter(program, op_index, 0);
		std::cout << " " << mb.f32 << "\n";
	}
	else if (op == OpCode::PUSH_CONSTANT_STRING)
	{
		MemoryBlock mb = GetParameter(program, op_index, 0);
		std::cout << " " << mb.index << ": ";
		PrintStringConstant(program, mb.index);
		std::cout << "\n";
	}
	else
	{
		std::cout << "\n";
	}

	return op_index + GetOpCodeSize(op);
}

//----------------------------------------
static void PrintMemoryChar(char c)
{
	if (c == '\n')
		std::cout << "\\n ";
	else if (c == '\t')
		std::cout << "\\t ";
	else if (c < 0 || !std::isprint(c))		// isprint s'attend à avoir un caractère non signé, on doit donc tester que ce n'est pas le cas pour éviter un assert
		std::cout << "###";
	else
		std::cout << "'" << c << "'";
}

//----------------------------------------
void PrintMemory(uint32_t count, const MemoryBlock* memory, bool reverse_order)
{
	std::cout << std::setw(7) << "Pos |" << std::setw(23) << "Store |" << std::setw(16) << "Index |" << std::setw(16) << "i32 |" << std::setw(16) << " f32 |" << std::setw(15) << " c8_4 |" << std::setw(7) << "c |" << std::setw(7) << "b\n";
	std::cout << "------------------------------------------------------------------------------------------------------------\n";

	for (uint32_t i = 0; i < count; ++i)
	{
		uint32_t pos = reverse_order ? (count - 1) - i : i;
		const MemoryBlock& mb = memory[pos];

		std::cout << std::setw(5) << (pos) << std::setw(0) << " |";
		std::cout <<
			std::setw(0) << " [" << std::setw(3) << (uint32_t)mb.store[0] <<
			std::setw(0) << "][" << std::setw(3) << (uint32_t)mb.store[1] <<
			std::setw(0) << "][" << std::setw(3) << (uint32_t)mb.store[2] <<
			std::setw(0) << "][" << std::setw(3) << (uint32_t)mb.store[3] <<
			std::setw(0) << "] |";
		std::cout << std::setw(14) << mb.index << std::setw(0) << " |";
		std::cout << std::setw(14) << mb.i32 << std::setw(0) << " |";
		std::cout << std::setw(14) << mb.f32 << std::setw(0) << " |";
		std::cout << std::setw(0) << " ";
		PrintMemoryChar(mb.c8_4[0]);
		PrintMemoryChar(mb.c8_4[1]);
		PrintMemoryChar(mb.c8_4[2]);
		PrintMemoryChar(mb.c8_4[3]);
		std::cout << std::setw(0) << " |";
		std::cout << std::setw(6);
		if (mb.c == '\n') std::cout << "\\n";
		else if (mb.c == '\r') std::cout << "\\r";
		else if (mb.c == '\t') std::cout << "\\t";
		else std::cout << mb.c;
		std::cout << std::setw(6) << (mb.b ? "true" : "false") << "\n";
	}

	std::cout << std::setw(0) << "\n";
}

//----------------------------------------
void PrintProgram(const CompiledProgram& program)
{
	std::cout << "## CONSTANTS ##\n";
	PrintMemory(program.ConstantTop, program.Constants, false);

	std::cout << "## OPERATIONS ##\n";
	uint32_t cursor = 0;

	while (cursor < program.OpCount)
		cursor = PrintOperation(program, cursor);
}

//----------------------------------------
void PrintExecutionStack( const ExecutionContext& context )
{
	std::cout << "Execution Stack (" << context.ExecutionStackCursor << " memory blocks):\n\n";

	PrintMemory( context.ExecutionStackCursor, context.ExecutionStack );
}

//----------------------------------------
void PrintMemoryStack( const ExecutionContext& context )
{
	std::cout << "Memory Stack (" << context.MemoryStackDynamicAllocCursor << " memory blocks):\n\n";

	PrintMemory( context.MemoryStackDynamicAllocCursor, context.MemoryStack );
}

//----------------------------------------
void PrintCallStack( const TextData& text_data, const TokenCollection& tokens, const CompiledProgram& program, const ExecutionContext& context, const CompilationData& compilation_data )
{
	std::cout << "Call Stack (" << context.CallStackCursor << " frames):\n\n";

	for( uint32_t i = context.CallStackCursor; i > 0; --i )
	{
		const CallFrame& callFrame = context.CallStack[i - 1];

		std::cout << std::setw( 5 ) << ( i - 1 ) << std::setw( 5 ) << callFrame.CallSite << std::setw( 0 ) << " ";

		MemoryBlock mb = GetParameter(program, callFrame.CallSite, 0);
		uint32_t firstFunctionOp = mb.index;

		std::cout << " ";
		PrintFunctionNameFromFirstOperation( compilation_data, text_data, tokens, firstFunctionOp );
		std::cout << "\n";

		std::cout << std::setw( 10 ) << callFrame.MemoryBaseAddress << std::setw( 0 ) << "\n";
	}
}