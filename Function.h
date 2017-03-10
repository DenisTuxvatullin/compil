#pragma once
#include <string>
#include <sstream>
#include "DataType.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "Label.h"

extern const char ENTRY_POINT_NAME[];

// == Функция
// - Содержит информацию о прототипе функции: имя, аргументы, возвращаемый тип данных
// - Содержит скомпилированный код на языке MSIL
// - Позволяет добавлять новые ассемблерные инсструкции
// - Проводит дополнительные проверки на целостность стека
class Function
{
public:
	Function(const DataType &returnType, const std::string &name, const SymbolTable<Variable> &arguments);

	// Add raw string to function's code.
	void AddRaw(const std::string &code);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Load argument onto the stack.
	void ldarg(const std::string &argName);

	// Push local variable value onto stack.
	void ldloc(const std::string &localName);
	// Pop a value from stack into local variable.
	void stloc(const std::string &localName);

	// Push the value of field on the stack.
	void ldsfld(const std::string &globalName, const SymbolTable<Variable> &globalsTable);
	// Replace the value of field with val.
	void stsfld(const std::string &globalName, const SymbolTable<Variable> &globalsTable);

	// Push a string object for the literal str.
	void ldstr(const std::string &str);
	// Return from method, possibly with a value.
	void ret();

	// Add two values, returning a new value.
	void add();
	// Subtract value2 from value1, returning a new value.
	void sub();
	// Multiply values.
	void mul();
	// Divide two values to return a quotient or floating-point result.
	void div();
	// Remainder when dividing one value by another.
	void rem();

	// Pop value from the stack.
	void pop();
	// Push num of type int32 onto the stack as int32.
	void ldc(int value);
	// Alias of ldc(int32) for convenience.
	void ldc(bool value);
	// Push num of type float64 onto the stack as F.
	void ldc(double value);
	// Call a method.
	void call(std::shared_ptr<Function> method);
	// Call an external function
	void call_extern(const DataType &returnType, const std::string &name,
		const std::vector<DataType> &arguments, bool callInstance = false);

	// Bitwise AND of two integral values, returns an integral value.
	void and();
	// Bitwise OR of two integer values, returns an integer.
	void or();

	// Push 1 (of type int32) if value1 equals value2, else push 0.
	void ceq();
	// Negate value.
	void neg();
	// Push 1 (of type int32) if value1 > value2, else push 0.
	void cgt();
	// Push 1 (of type int32) if value1 < value2, else push 0.
	void clt();

	// Emit a label.
	void emit(Label &label);
	// Branch to target.
	void br(Label &label);
	// Branch to target if value is non-zero (true).
	void brtrue(Label &label);
	// Branch to target if value is zero (false).
	void brfalse(Label &label);

	// Create a new array with elements of type arrayType.
	void newarr(ATOMIC_TYPE arrayType);
	// Load the element at index onto the top of the stack.
	void ldelem(ATOMIC_TYPE arrayType);
	// Replace array element at index with the value on the stack
	void stelem(ATOMIC_TYPE arrayType);
	// Allocate an uninitialized object or value type and call ctor.
	void newobj(const std::string &className, const std::vector<std::string> &args);
	// Call a method associated with an object.
	void callvirt(const std::string &returnType, const std::string &className,
		const std::string &methodName, const std::vector<std::string> &args);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Get function's name
	const std::string &GetName() const;

	// Get function's code (fully formatted, with header)
	std::string GetCode();

	// Get symbol table of function's arguments
	const SymbolTable<Variable> &GetArguments() const;

	// Get symbol table of this function's locals
	SymbolTable<Variable> &GetLocals();

	// True if this function returns something (not void)
	bool ReturnsValue() const;

	// Get data type that this function returns
	const DataType &GetReturnType() const;

	// If function is set as raw, its source code will be written as is,
	// without the addition of .maxstack, .locals, ets.
	void SetRaw(bool isRaw);

private:
	// Is function raw
	bool m_isRaw;

	// current stack depth
	int m_stackDepth;
	// maximum stack depth
	int m_maxstack;
	// function name
	const std::string m_name;
	// function's code in CIL
	std::stringstream m_code;
	// function return data type
	const DataType m_returnType;
	// current label index
	unsigned m_labelIndex;

	// function arguments
	SymbolTable<Variable> m_arguments;
	// local variables
	SymbolTable<Variable> m_locals;

	// Adds CIL instruction that has short (.s) and single command (.0, .1, ...) forms
	void AddMultiformInstruction(const char *baseName, unsigned index);
	// Used in newobj and callvirt instructions
	void FormatCall(const std::string &returnType, const std::string &className,
		const std::string &methodName, const std::vector<std::string> &args);

	const char *ATypeToInternal(ATOMIC_TYPE aType);

	void DecreaseStack();
	void IncreaseStack();
	void CheckStackIsEmpty() const;
};
