/*
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fstream>
#include "Compiler.h"

using namespace std;

Compiler::Compiler(const string &programName)
	: m_programName(PrepareName(programName))
{
	AddRuntimeLibary();
}

shared_ptr<Function> Compiler::NewFunction(DataType returnType, const string &name, const SymbolTable<Variable> &arguments)
{
	if (m_inlines.find(name) != m_inlines.end())
	{
		throw IntermediateError("'" + name +  "' is a reserved function name");
	}

	m_functions.Define(make_shared<Function>(returnType, name, arguments));
	return GetFunction(name);
}

shared_ptr<Function> Compiler::GetFunction(const string &name)
{
	return m_functions.Get(name);
}

const string &Compiler::GetProgramName() const
{
	return m_programName;
}

string Compiler::GetCode() const
{
	stringstream code;

	code << GetPrologue();
	
	for (unsigned i = 0; i < m_globals.Count(); i++)
	{
		code << ".field private static " << m_globals[i]->GetType().ToIL() << " " << m_globals[i]->GetName() << endl;
	}

	for (unsigned i = 0; i < m_functions.Count(); i++)
	{
		code << m_functions[i]->GetCode();
	}
		
	return code.str();
}

void Compiler::SaveCode(const string &fileName) const
{
	ofstream outf(fileName);
	outf << GetCode();
	outf.close();
}

string Compiler::GetExecutablePath()
{
	const unsigned NAME_BUFFER_SIZE = 0xFF;

	char *pathBuffer = new char[NAME_BUFFER_SIZE];
	GetModuleFileName(NULL, pathBuffer, NAME_BUFFER_SIZE);
	string executableName(pathBuffer);
	delete pathBuffer;

	return executableName.substr(0, executableName.find_last_of('\\'));
}

bool Compiler::IsFileExists(const string &fileName)
{
    ifstream infile(fileName);
    return infile.good();
}

string Compiler::GetILAsmPath() const
{
	string possiblePaths[] = {"C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\ilasm.exe",
		"C:\\Windows\\Microsoft.NET\\Framework\\v2.0.50727\\ilasm.exe",
		GetExecutablePath() + "\\ilasm.exe",
		"ilasm.exe"};

	for (const string &path : possiblePaths)
	{
		if (IsFileExists(path))
		{
			return path;
		}
	}

	throw InternalError("ILAsm was not found on your system\n"
		"Please copy ilasm.exe to the current directory to fix this error\n"
		"Compilation aborted\n");
}

int Compiler::Compile() const
{
	if (!m_functions.IsDefined(ENTRY_POINT_NAME))
	{
		throw IntermediateError(string("Entry point function '")
			+ ENTRY_POINT_NAME + "' is missing");
	}

	string outputName = GetProgramName() + ".il";
	SaveCode(outputName);

	string commandLine = GetILAsmPath() + " \"" + outputName + "\"";

	int returnCode = system(commandLine.c_str());

	if (returnCode == 0)
	{
		DeleteFile(outputName.c_str());
	}

	return returnCode;
}

SymbolTable<Variable> &Compiler::GetGlobals()
{
	return m_globals;
}

SymbolTable<ConstantBase> &Compiler::GetConstants()
{
	return m_constants;
}

map<string, InlineFnPtr> &Compiler::GetInlines()
{
	return m_inlines;
}

string Compiler::GetPrologue() const
{
	return string(
		".assembly extern mscorlib {}\n"
		".assembly " + GetProgramName() + "\n"
		"{\n"
		"\t.ver 1:0:0:0\n"
		"}\n"
		".module " + GetProgramName() + ".exe\n"
		// For random%(from%, to%)
		".field private static class [mscorlib]System.Random __RNG\n"
		);
}

string Compiler::PrepareName(const string &name)
{
	string result;

	for (char ch : name)
	{
		result += isalnum(ch) ? ch : '_';
	}

	return result;
}

void Compiler::AddRuntimeLibary()
{
	// Константы по умолчанию
	m_constants.Define(make_shared<BoolConst>("true", true));
	m_constants.Define(make_shared<BoolConst>("false", false));

	vector<DataType> externArg;
	externArg.push_back(DataType(ATOMIC_TYPE::TYPE_STRING));
	SymbolTable<Variable> fnArg;
	fnArg.Define(make_shared<Variable>("text", DataType(ATOMIC_TYPE::TYPE_STRING)));

	// Print(text$)
	{
		auto Print = NewFunction(DataType(ATOMIC_TYPE::TYPE_VOID), "print", fnArg);
		Print->ldarg("text");
		Print->call_extern(DataType(ATOMIC_TYPE::TYPE_VOID), "[mscorlib]System.Console::Write", externArg);
		Print->ret();
	}

	// PrintN(text$)
	{
		auto PrintN = NewFunction(DataType(ATOMIC_TYPE::TYPE_VOID), "printn", fnArg);
		PrintN->ldarg("text");
		PrintN->call_extern(DataType(ATOMIC_TYPE::TYPE_VOID), "[mscorlib]System.Console::WriteLine", externArg);
		PrintN->ret();
	}

	// Read$()
	{
		auto Read = NewFunction(DataType(ATOMIC_TYPE::TYPE_STRING), "read", SymbolTable<Variable>());
		Read->call_extern(DataType(ATOMIC_TYPE::TYPE_STRING), "[mscorlib]System.Console::ReadLine", vector<DataType>());
		Read->ret();		
	}

	// Random%(from%, to%)
	{
		SymbolTable<Variable> args;
		args.Define(make_shared<Variable>("from", DataType(ATOMIC_TYPE::TYPE_INT)));
		args.Define(make_shared<Variable>("to", DataType(ATOMIC_TYPE::TYPE_INT)));

		auto Random = NewFunction(DataType(ATOMIC_TYPE::TYPE_INT), "random", args);

		Random->SetRaw(true);
		Random->AddRaw(".maxstack 3");
		Random->AddRaw("ldsfld class [mscorlib]System.Random __RNG");
		Random->AddRaw("ldarg.0");
		Random->AddRaw("ldarg.1");
		Random->AddRaw("callvirt instance int32 [mscorlib]System.Random::Next(int32, int32)");
		Random->AddRaw("ret");
	}

	// Substr$(str$, start%, length%)
	{
		SymbolTable<Variable> args;
		args.Define(make_shared<Variable>("str", DataType(ATOMIC_TYPE::TYPE_STRING)));
		args.Define(make_shared<Variable>("start", DataType(ATOMIC_TYPE::TYPE_INT)));
		args.Define(make_shared<Variable>("length", DataType(ATOMIC_TYPE::TYPE_INT)));

		auto Substr = NewFunction(DataType(ATOMIC_TYPE::TYPE_STRING), "substr", args);

		Substr->ldarg("str");
		Substr->ldarg("start");
		Substr->ldarg("length");

		//callvirt instance string [mscorlib]System.String::Substring(int32, int32)
		vector<string> callArgs;
		callArgs.push_back("int32");
		callArgs.push_back("int32");

		Substr->callvirt("string", "[mscorlib]System.String", "Substring", callArgs);
		Substr->ret();
	}

	// Strlen%(str$)
	{
		SymbolTable<Variable> args;
		args.Define(make_shared<Variable>("str", DataType(ATOMIC_TYPE::TYPE_STRING)));

		auto Strlen = NewFunction(DataType(ATOMIC_TYPE::TYPE_INT), "strlen", args);

		Strlen->ldarg("str");
		// callvirt instance int32 [mscorlib]System.String::get_Length()
		Strlen->callvirt("int32", "[mscorlib]System.String", "get_Length", vector<string>());
		Strlen->ret();
	}
}
*/