/*
#include <iostream>
#include "PrintVisitor.h"
#include "Ast.h"

using namespace std;

void PrintVisitor::visit(const AstError &ref)
{
	cout << "AstError";
}

void PrintVisitor::visit(const AstIntConst &ref)
{
	cout << "AstIntConst<" << ref.GetValue() << ">";
}

void PrintVisitor::visit(const AstFloatConst &ref)
{
	cout << "AstFloatConst<" << ref.GetValue() << ">";
}

void PrintVisitor::visit(const AstStringConst &ref)
{
	cout << "AstStringConst<" << ref.GetValue() << ">";
}

void PrintVisitor::visit(const AstBoolConst &ref)
{
	cout << "AstBoolConst<" << (ref.GetValue() ? "true" : "false") << ">";
}

void PrintVisitor::visit(const AstVariable &ref)
{
	cout << "AstVariable<Name=" << ref.GetName() << "; Type=" + ref.GetType().ToString() << ">";
}

void PrintVisitor::visit(const AstUnaryExpression &ref)
{
	cout << "AstUnaryExpression<Expr=";

	ref.GetExpression()->accept(*this);

	cout << "; Operator=" << KEYWORD_STRING[ref.GetOperator()] << ">";
}

void PrintVisitor::visit(const AstBinaryExpression &ref)
{
	cout << "AstBinaryExpr<Left=";

	ref.GetLeft()->accept(*this);
		
	cout << "; Operator=" << KEYWORD_STRING[ref.GetOperator()] << "; Right=";

	ref.GetRight()->accept(*this);

	cout << ">";
}

void PrintVisitor::visit(const AstAssignmentExpression &ref)
{
	cout << "AstAssignmentExpression<Variable=";
	ref.GetVariable()->accept(*this);
	cout << "; Expression=";
	ref.GetExpression()->accept(*this);
	cout << ">";
}

void PrintVisitor::visit(const AstList &ref)
{
	cout << "AstList<" << endl;

	for (shared_ptr<IAst> statement : ref.GetElements())
	{
		statement->accept(*this);
		cout << endl;
	}

	cout << "\n>";
}

void PrintVisitor::visit(const AstFunction &ref)
{
	cout << "AstFunction<Name=" << ref.GetName() << ";\n";
	cout << "ReturnType=" << ref.GetReturnType().ToString() << ";\n";
	cout << "Arguments="; ref.GetArguments()->accept(*this); cout << ";\n";
	cout << "Code="; ref.GetCode()->accept(*this);
	cout << "\n>";
}

void PrintVisitor::visit(const AstReturn &ref)
{
	if (ref.HasExpression())
	{
		cout << "AstReturn<Expr=";
		ref.GetExpression()->accept(*this);
		cout << ">\n";
	}
	else
	{
		cout << "AstReturn<>\n";
	}
}

void PrintVisitor::visit(const AstGlobal &ref)
{
	cout << "AstGlobal<";
	ref.GetDeclarations()->accept(*this);
	cout << ">\n";
}

void PrintVisitor::visit(const AstFunctionCall &ref)
{
	cout << "AstFunctionCall<Name=" << ref.GetName();
	cout << "; ReturnType=" << ref.GetType().ToString();
	cout << "; Args="; ref.GetArguments()->accept(*this);
	cout << ">\n";
}

void PrintVisitor::visit(const AstIf &ref)
{
	cout << "AstIf<Condition="; ref.GetCondition()->accept(*this);
	cout << "; Consequent="; ref.GetConsequent()->accept(*this);

	if (ref.HasElse())
	{
		cout << "; Alternative="; ref.GetAlternative()->accept(*this);
	}

	cout << ">\n";
}

void PrintVisitor::visit(const AstNewDict &ref)
{
	cout << "AstNewDict<" << ref.GetType().ToString() << ">";
}

void PrintVisitor::visit(const AstDictValue &ref)
{
	cout << "AstDictValue<Name=" << ref.GetName();
	cout << "; Type=" << ref.GetType().ToString();
	cout << "; Key="; ref.GetKey()->accept(*this);
	cout << ">";
}

void PrintVisitor::visit(const AstDictAssign &ref)
{
	cout << "AstDictAssign<Dict="; ref.GetDictValue()->accept(*this);
	cout << "; Expression="; ref.GetExpression()->accept(*this);
	cout << ">";
}

void PrintVisitor::visit(const AstConstantDecl &ref)
{
	cout << "AstConstantDecl<>";
}

void PrintVisitor::visit(const AstFor &ref)
{
	cout << "AstFor<";
	cout << "From="; ref.GetFrom()->accept(*this); cout << ";\n";
	cout << "To="; ref.GetTo()->accept(*this); cout << ";\n";
	cout << "Step="; ref.GetStep()->accept(*this); cout << ";\n";
	cout << "Code="; ref.GetCode()->accept(*this); cout << ">";
}

void PrintVisitor::visit(const AstWhile &ref)
{
	cout << "AstWhile<";
	cout << "Condition="; ref.GetCondition()->accept(*this); cout << ";\n";
	cout << "IsDoWhile=" << ref.IsDoWhile() << ";\n";
	cout << "Code="; ref.GetCode()->accept(*this); cout << ">";
}

void PrintVisitor::visit(const AstNewArray &ref)
{
	cout << "AstNewArray<Type=" << ref.GetType().ToString() << ";\n";
	cout << "Dimensions=";
	for (auto dim : ref.GetDimensions())
	{
		dim->accept(*this);
		cout << " ";
	}
	cout << ">";
}

void PrintVisitor::visit(const AstArrayValue &ref)
{
	cout << "AstArrayValue<Name=" << ref.GetName() << "; ";
	cout << "Type=" << ref.GetType().ToString() << "; ";
	cout << "Indexes="; ref.GetIndexes()->accept(*this); cout << ">";
}

void PrintVisitor::visit(const AstArrayAssign &ref)
{
	cout << "AstArrayAssign<Elem="; ref.GetElem()->accept(*this); cout << "; ";
	cout << "Expression="; ref.GetExpression()->accept(*this); cout << ">";
}
*/