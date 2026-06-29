#pragma once

#include "Analyzer.h"
#include "VirtualMachine.h"

//----------------------------------------
struct FunctionDefRef
{
	token_index Name;
	uint32_t FirstOperation;
};

//----------------------------------------
struct StringRef
{
	ast_node_index Node;
	uint32_t ConstantAddress;
};

//----------------------------------------
struct CompilationData
{
	FunctionDefRef FunctionRefs[128];
	uint32_t FunctionRefCount{ 0 };

	StringRef StringRefs[128];
	uint32_t StringRefCount{ 0 };

	uint32_t MainNameToken{ 0 };
};

uint32_t GetOpCodeSize(OpCode op_code);

Error CompileAST( const TextData& text_data, const TokenCollection& tokens, const ASTNodeCollection& ast, const SemanticAnalyzisData& analyzis_data, const ExternalFunctionCatalog& ex_func_catalog, CompiledProgram& out_program, CompilationData& out_compilation_data );

void PrintCallStack(const TextData& text_data, const TokenCollection& tokens, const CompiledProgram& program, const ExecutionContext& context, const CompilationData& compilation_data);