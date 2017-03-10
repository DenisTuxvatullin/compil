#pragma once
#include <string>
#include <memory>
#include <functional>
#include "Function.h"
#include "SymbolTable.h"
#include "Constant.h"

class AstFunctionCall;
class GeneratorVisitor;
typedef void (*InlineFnPtr)(GeneratorVisitor &v, const AstFunctionCall &ref);

class Compiler
{
public:
	Compiler(const std::string &programName);

	std::shared_ptr<Function> NewFunction(DataType returnType, const std::string &name, const SymbolTable<Variable> &arguments);
	std::shared_ptr<Function> GetFunction(const std::string &name);

	const std::string &GetProgramName() const;

	// Returns complete assembler code for current programm
	std::string GetCode() const;
	// Saves code to .il file
	void SaveCode(const std::string &fileName) const;
	// Compiles code to executable by calling ILAsm
	int Compile() const;

	// Returns symbol table of global variables
	SymbolTable<Variable> &GetGlobals();
	// Returns symbol table of constants
	SymbolTable<ConstantBase> &GetConstants();
	//
	std::map<std::string, InlineFnPtr> &GetInlines();

private:
	// program name
	std::string m_programName;
	// global variables
	SymbolTable<Variable> m_globals;
	// constants
	SymbolTable<ConstantBase> m_constants;
	// functions
	SymbolTable<Function> m_functions;
	// inline functions
	std::map<std::string, InlineFnPtr> m_inlines;

	// Returns code to be placed in the beginning of .il file
	std::string GetPrologue() const;
	// Returns a valid program name
	// (a string containing only letters, numerals and underscores)
	static std::string PrepareName(const std::string &name);
	// Adds built-in functions and constants
	void AddRuntimeLibary();

	// Returns path where current executable is located
	static std::string GetExecutablePath();
	std::string GetILAsmPath() const;
	static bool IsFileExists(const std::string &fileName);
};
