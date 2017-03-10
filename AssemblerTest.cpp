#include "AssemblerTest.h"
#include "DataType.h"
#include "Variable.h"
#include "SymbolTable.h"
#include "Function.h"
#include "Constant.h"
#include "Compiler.h"

using namespace std;

void TestAssembler()
{
	Compiler comp("Methods");

	{
		SymbolTable<Variable> args;
		args.Define(make_shared<Variable>("a", ATOMIC_TYPE::TYPE_INT));
		args.Define(make_shared<Variable>("b", ATOMIC_TYPE::TYPE_INT));
		auto fn = comp.NewFunction(DataType(ATOMIC_TYPE::TYPE_INT), "DoSum", args);

		fn->ldarg("a");
		fn->ldarg("b");
		fn->add();
		fn->ret();
	}

	{
		SymbolTable<Variable> args;
		args.Define(make_shared<Variable>("value", ATOMIC_TYPE::TYPE_INT));
		auto fn = comp.NewFunction(DataType(ATOMIC_TYPE::TYPE_VOID), "PrintSum", args);

		fn->ldstr("The Result is: ");
		fn->call(comp.GetFunction("PrintString"));
    
		fn->ldarg("value");
		fn->call(comp.GetFunction("PrintInt"));
    
		fn->ret();
	}

	{
		auto fn = comp.NewFunction(DataType(ATOMIC_TYPE::TYPE_VOID), "main", SymbolTable<Variable>());

		fn->ldc(10);
		fn->ldc(20);
		fn->call(comp.GetFunction("DoSum"));
		fn->call(comp.GetFunction("PrintSum"));
		fn->ret();
	}

	comp.Compile();
}
