#include <iostream>

#include "FileReader.h"
#include "Compiler.h"
#include "VirtualMachine.h"

// Déplacé ici du début de run pour corriger l'avertissement qui indique qu'on utilise une taille de stack trop grande
FileData fileData;
TokenCollection tokens;
ASTNodeCollection ast;
SemanticAnalyzisData analyzisData;
CompilationData compilationData;
CompiledProgram program;
ExecutionContext context;

//----------------------------------------
bool run(const char* file, bool print_tokens, bool print_ast, bool print_analyzis, bool print_ops, bool print_stacks, bool no_execution)
{
	if (!LoadFile(file, fileData))
	{
		std::cout << "Failed to load file " << file << "\n";
		CloseFile(fileData);
		return false;
	}

	TokenizeText(fileData.Text, tokens);

	if (print_tokens)
	{
		std::cout << "TOKENS: ------------\n";
		PrintTokens(fileData.Text, tokens, false);
		std::cout << "--------------------\n";
	}

	if (Error error = ParseTokens(fileData.Text, tokens, ast))
	{
		PrintError(fileData.Text, tokens, error);
		CloseFile(fileData);
		return false;
	}

	SetupStandardLibrary(context.ExternalFunctions);

	if (Error error = AnalyzeAST(fileData.Text, tokens, context.ExternalFunctions, ast, analyzisData))
	{
		PrintError(fileData.Text, tokens, error);

		ClearFunctionCatalog(context.ExternalFunctions);
		CloseFile(fileData);
		return false;
	}

	if (print_ast)
	{
		std::cout << "AST: ---------------\n";
		PrintAST(fileData.Text, tokens, ast, analyzisData);
		std::cout << "--------------------\n";
	}

	if (print_analyzis)
	{
		std::cout << "ANALYZIS: ----------\n";
		PrintAnalyzis(fileData.Text, tokens, ast, analyzisData);
		std::cout << "--------------------\n";
	}

	if (Error error = CompileAST(fileData.Text, tokens, ast, analyzisData, context.ExternalFunctions, program, compilationData))
	{
		PrintError(fileData.Text, tokens, error);

		ClearFunctionCatalog(context.ExternalFunctions);
		CloseFile(fileData);
		return false;
	}

	if (print_ops)
	{
		std::cout << "OPERATIONS: --------\n";
		PrintProgram(program);
		std::cout << "--------------------\n";
	}

	if (no_execution)
	{
		ClearFunctionCatalog(context.ExternalFunctions);
		CloseFile(fileData);
		return true;
	}
	else
	{
		bool success = ExecuteProgram(program, context);

		if (print_stacks)
		{
			std::cout << "--------------------\n";
			PrintExecutionStack(context);
			std::cout << "--------------------\n";
			PrintMemoryStack(context);
			std::cout << "--------------------\n";
			PrintCallStack(fileData.Text, tokens, program, context, compilationData);
			std::cout << "--------------------\n";
		}

		ClearFunctionCatalog(context.ExternalFunctions);
		CloseFile(fileData);

		if (!success)
		{
			std::cout << "\nProgram execution failure\n";
			return false;
		}
		else
		{
			std::cout << "\nProgram execution success\n";
			return true;
		}
	}
}

//----------------------------------------
void main( int argc, char** argv )
{
	bool displayHelp = false;
	bool compileFile = false;

	bool printTokens = false;
	bool printAST = false;
	bool printAnalyzis = false;
	bool printOps = false;
	bool printStacks = false;
	bool noExecution = false;

	if (argc == 2 && strcmp(argv[1], "-h") == 0)
		displayHelp = true;
	else if (argc < 2)
		std::cout << "No argument: missing file to compile\n";
	else
	{
		compileFile = true;

		for (int i = 2; i < argc; ++i)
		{
			if (strcmp(argv[i], "-tok") == 0)
				printTokens = true;
			else if (strcmp(argv[i], "-ast") == 0)
				printAST = true;
			else if (strcmp(argv[i], "-sem") == 0)
				printAnalyzis = true;
			else if (strcmp(argv[i], "-ops") == 0)
				printOps = true;
			else if (strcmp(argv[i], "-stacks") == 0)
				printStacks = true;
			else if (strcmp(argv[i], "-noexe") == 0)
				noExecution = true;

			else if (strcmp(argv[i], "-h") == 0)
				displayHelp = true;
		}
	}

	if (displayHelp)
	{
		std::cout << "Help:\n";
		std::cout << "Usage: compiler.exe path_tofile_to_compile [-h|-tok|-ast|-sem|-ops|-stacks|-noexe]\n";
		std::cout << "  -h: Display this help (path to file can be omitted in this case)\n";
		std::cout << "  -tok: Print file tokenization\n";
		std::cout << "  -ast: Print abstract syntax tree (with embedded semantic data)\n";
		std::cout << "  -sem: Print semantic analyzis data\n";
		std::cout << "  -ops: Print compiled operations\n";
		std::cout << "  -stacks: Print stacks at execution end (wether execution succeeded or failed). Useless if used with -noexe\n";
		std::cout << "  -noexe: Prevent program execution, useful when checking for tokenization or AST without actually running the code\n";
	}
	else if (compileFile)
	{
		run(argv[1], printTokens, printAST, printAnalyzis, printOps, printStacks, noExecution);
	}
}