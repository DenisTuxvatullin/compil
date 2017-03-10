#include "Function.h"
#include "Exception.h"

using namespace std;

const char ENTRY_POINT_NAME[] = "main";

Function::Function(const DataType &returnType, const string &name, const SymbolTable<Variable> &arguments)
	: m_maxstack(0)
	, m_stackDepth(0)
	, m_name(name)
	, m_returnType(returnType)
	, m_arguments(arguments)
	, m_code()
	, m_isRaw(false)
	, m_labelIndex(0)
{
	if (name == ENTRY_POINT_NAME)
	{
		if (returnType.IsNotVoid())
		{
			throw IntermediateError(string("Entry point '") +  ENTRY_POINT_NAME + "' must return void");
		}
		if (arguments.Count() != 0)
		{
			throw IntermediateError(string("Entry point '") +  ENTRY_POINT_NAME + "' takes no arguments");
		}
	}
}

void Function::AddRaw(const string &code)
{
	m_code << code << endl;
}

void Function::AddMultiformInstruction(const char *baseName, unsigned index)
{
	if (index <= 3)
	{
		m_code << baseName << "." << index << endl;
	}
	else if (index <= 0xFF)
	{
		m_code << baseName << ".s " << index << endl;
	}
	else if (index <= 0xFFFF)
	{
		m_code << baseName << " " << index << endl;
	}
	else
	{
		throw InternalError("Index overflow in Function::AddMultiformInstruction");
	}
}

void Function::ldarg(const string &argName)
{
	AddMultiformInstruction("ldarg", m_arguments.GetIndex(argName));
	IncreaseStack();
}

void Function::ldloc(const string &localName)
{
	AddMultiformInstruction("ldloc", m_locals.GetIndex(localName));
	IncreaseStack();
}

void Function::stloc(const string &localName)
{
	AddMultiformInstruction("stloc", m_locals.GetIndex(localName));
	DecreaseStack();
}

void Function::ldsfld(const std::string &globalName, const SymbolTable<Variable> &globalsTable)
{
	m_code << "ldsfld " << globalsTable.Get(globalName)->GetType().ToIL() << " " << globalName << endl;
	IncreaseStack();
}

void Function::stsfld(const std::string &globalName, const SymbolTable<Variable> &globalsTable)
{
	DecreaseStack();
	m_code << "stsfld " << globalsTable.Get(globalName)->GetType().ToIL() << " " << globalName << endl;
}

void Function::ldstr(const string &str)
{
	m_code << "ldstr \"" << str << "\"\n";
	IncreaseStack();
}

void Function::ret()
{
	m_code << "ret" << endl;

	if (ReturnsValue())
	{
		DecreaseStack();
	}

	CheckStackIsEmpty();
}

void Function::add()
{
	m_code << "add" << endl;

	DecreaseStack();
	DecreaseStack();
	IncreaseStack();
}

void Function::sub()
{
	m_code << "sub" << endl;

	DecreaseStack();
	DecreaseStack();
	IncreaseStack();
}

void Function::mul()
{
	m_code << "mul" << endl;

	DecreaseStack();
	DecreaseStack();
	IncreaseStack();
}

void Function::div()
{
	m_code << "div" << endl;

	DecreaseStack();
	DecreaseStack();
	IncreaseStack();
}

void Function::rem()
{
	m_code << "rem" << endl;

	DecreaseStack();
	DecreaseStack();
	IncreaseStack();
}

void Function::pop()
{
	m_code << "pop" << endl;
	DecreaseStack();
}

void Function::ldc(int value)
{
	if (value == -1)
	{
		m_code << "ldc.i4.m1\n";
	}
	else if (value >= 0 && value <= 8)
	{
		m_code << "ldc.i4." << value << endl;
	}
	else if (value >= -128 && value <= 127)
	{
		m_code << "ldc.i4.s " << value << endl;
	}
	else
	{
		m_code << "ldc.i4 " << value << endl;
	}

	IncreaseStack();
}

void Function::ldc(bool value)
{
	ldc(value ? 1 : 0);
}

void Function::ldc(double value)
{
	m_code << "ldc.r8 " << value << endl;
	IncreaseStack();
}

void Function::call(shared_ptr<Function> method)
{
	vector<DataType> args;

	// better than copypasting entire call_extern implementation
	for (unsigned i = 0; i < method->GetArguments().Count(); i++)
	{
		args.push_back(method->GetArguments()[i]->GetType());
	}

	call_extern(method->GetReturnType(), method->GetName(), args);
}

void Function::call_extern(const DataType &returnType, const string &name,
	const vector<DataType> &arguments, bool callInstance)
{
	m_code << "call ";
	
	if (callInstance)
	{
		m_code << "instance ";

		// Pop object reference
		DecreaseStack();
	}

	m_code << returnType.ToIL() << " " << name << "(";

	for (unsigned int i = 0; i < arguments.size(); i++)
	{
		DecreaseStack();

		m_code << arguments[i].ToIL();
		if (i < arguments.size() - 1)
		{
			m_code << ", ";
		}
	}

	m_code << ")\n";

	if (returnType.IsNotVoid())
	{
		IncreaseStack();
	}
}

void Function::and()
{
	DecreaseStack();
	DecreaseStack();
	m_code << "and\n";
	IncreaseStack();
}

void Function::or()
{
	DecreaseStack();
	DecreaseStack();
	m_code << "or\n";
	IncreaseStack();
}

void Function::ceq()
{
	DecreaseStack();
	DecreaseStack();
	m_code << "ceq\n";
	IncreaseStack();
}

void Function::cgt()
{
	DecreaseStack();
	DecreaseStack();
	m_code << "cgt\n";
	IncreaseStack();
}

void Function::clt()
{
	DecreaseStack();
	DecreaseStack();
	m_code << "clt\n";
	IncreaseStack();
}

void Function::neg()
{
	DecreaseStack();
	m_code << "neg\n";
	IncreaseStack();
}

void Function::emit(Label &label)
{
	m_code << label.ToString() << ":\n";
	label.SetEmitted();
}

void Function::br(Label &label)
{
	m_code << "br " << label.ToString() << endl;
}

void Function::brtrue(Label &label)
{
	DecreaseStack();
	m_code << "brtrue " << label.ToString() << endl;
}

void Function::brfalse(Label &label)
{
	DecreaseStack();
	m_code << "brfalse " << label.ToString() << endl;
}

void Function::newarr(ATOMIC_TYPE arrayType)
{
	DecreaseStack();

	m_code << "newarr [mscorlib]System.";
	switch (arrayType)
	{
	case ATOMIC_TYPE::TYPE_BOOL:
		m_code << "Boolean";
		break;
	case ATOMIC_TYPE::TYPE_FLOAT:
		m_code << "Double";
		break;
	case ATOMIC_TYPE::TYPE_INT:
		m_code << "Int32";
		break;
	case ATOMIC_TYPE::TYPE_STRING:
		m_code << "String";
		break;
	default:
		throw InternalError("Function::newarr: invalid arrayType");
	}
	m_code << endl;

	IncreaseStack();
}

const char *Function::ATypeToInternal(ATOMIC_TYPE aType)
{
	switch (aType)
	{
	case ATOMIC_TYPE::TYPE_BOOL:
		return "i1";
	case ATOMIC_TYPE::TYPE_FLOAT:
		return "r8";
	case ATOMIC_TYPE::TYPE_INT:
		return "i4";
	case ATOMIC_TYPE::TYPE_STRING:
		return "ref";
	default:
		throw InternalError("Function::ATypeToInternal: invalid aType");
	}
}

void Function::ldelem(ATOMIC_TYPE arrayType)
{
	// Array
	DecreaseStack();
	// Index
	DecreaseStack();

	m_code << "ldelem." << ATypeToInternal(arrayType) << endl;
	
	// Element
	IncreaseStack();
}

void Function::stelem(ATOMIC_TYPE arrayType)
{
	// Array
	DecreaseStack();
	// Index
	DecreaseStack();
	// Value
	DecreaseStack();

	m_code << "stelem." << ATypeToInternal(arrayType) << endl;
}

void Function::newobj(const string &className, const vector<string> &args)
{
	m_code << "newobj ";
	FormatCall("void", className, ".ctor", args);

	// push reference to newly created object
	IncreaseStack();
}

void Function::callvirt(const string &returnType, const string &className,
		const string &methodName, const vector<string> &args)
{
	// pop reference to object whose method we are calling
	DecreaseStack();

	m_code << "callvirt ";
	FormatCall(returnType, className, methodName, args);
}

void Function::FormatCall(const string &returnType, const string &className,
		const string &methodName, const vector<string> &args)
{
	m_code << "instance " << returnType << " " << className << "::" << methodName;

	m_code << "(";
	for (unsigned i = 0; i < args.size(); i++)
	{
		// pop argument
		DecreaseStack();
		m_code << args[i];
		if (i < args.size() - 1)
		{
			m_code << ", ";
		}
	}
	m_code << ")\n";

	if (returnType != "void")
	{
		// push return value
		IncreaseStack();
	}
}

const string &Function::GetName() const
{
	return m_name;
}

string Function::GetCode()
{
	CheckStackIsEmpty();

	stringstream code;

	code << ".method public static " << m_returnType.ToIL() << " " << m_name << "(";
		
	for (unsigned int i = 0; i < m_arguments.Count(); i++)
	{
		code << m_arguments[i]->GetType().ToIL();

		if (i < m_arguments.Count() - 1)
		{
			code << ", ";
		}
	}

	code << ") cil managed\n";
	code << "{\n";

	if (!m_isRaw)
	{
		if (m_locals.Count() > 0)
		{
			code << ".locals init (";
			for (unsigned int i = 0; i < m_locals.Count(); i++)
			{
				code << m_locals[i]->GetType().ToIL();

				if (i < m_locals.Count() - 1)
				{
					code << ", ";
				}
			}
			code << ")\n";
		}

		code << ".maxstack " << m_maxstack << endl;

		if (m_name == ENTRY_POINT_NAME)
		{
			code << ".entrypoint\n";

			// For random%(from%, to%)
			code << "newobj instance void [mscorlib]System.Random::.ctor()\n";
			code << "stsfld class [mscorlib]System.Random __RNG\n";
		}
	}

	code << m_code.str();
	code << "}\n";

	return code.str();
}

const SymbolTable<Variable> &Function::GetArguments() const
{
	return m_arguments;
}

SymbolTable<Variable> &Function::GetLocals()
{
	return m_locals;
}

bool Function::ReturnsValue() const
{
	return m_returnType.IsNotVoid();
}

const DataType &Function::GetReturnType() const
{
	return m_returnType;
}

void Function::SetRaw(bool isRaw)
{
	m_isRaw = isRaw;
}

void Function::DecreaseStack()
{
	m_stackDepth--;
	if (m_stackDepth < 0)
	{
		throw InternalError("Stack integrity check failed in function '" + GetName() + "': pop from empty stack");
	}
}

void Function::IncreaseStack()
{
	m_stackDepth++;
	if (m_stackDepth > m_maxstack)
	{
		m_maxstack = m_stackDepth;
	}
}

void Function::CheckStackIsEmpty() const
{
	if (m_stackDepth != 0)
	{
		throw InternalError("Stack integrity check failed in function '" + GetName() + "': stack is not empty");
	}
}
