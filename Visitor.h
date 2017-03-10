#pragma once
#include "AstFwd.h"
#include "Exception.h"
#include "Constant.h"

class IVisitor
{
public:
	virtual ~IVisitor()
	{
	}

	virtual void visit(const IAst &ref)
	{
		throw InternalError("Visiting an abstract class");
	}

	virtual void visit(const AstError &ref) = 0;
	virtual void visit(const AstFloatConst &ref) = 0;
	virtual void visit(const AstStringConst &ref) = 0;
	virtual void visit(const AstIntConst &ref) = 0;
	virtual void visit(const AstBoolConst &ref) = 0;
	virtual void visit(const AstVariable &ref) = 0;
	virtual void visit(const AstUnaryExpression &ref) = 0;
	virtual void visit(const AstBinaryExpression &ref) = 0;
	virtual void visit(const AstAssignmentExpression &ref) = 0;
	virtual void visit(const AstList &ref) = 0;
	virtual void visit(const AstFunction &ref) = 0;
	virtual void visit(const AstReturn &ref) = 0;
	virtual void visit(const AstGlobal &ref) = 0;
	virtual void visit(const AstFunctionCall &ref) = 0;
	virtual void visit(const AstIf &ref) = 0;
	virtual void visit(const AstNewDict &ref) = 0;
	virtual void visit(const AstDictValue &ref) = 0;
	virtual void visit(const AstDictAssign &ref) = 0;
	virtual void visit(const AstConstantDecl &ref) = 0;
	virtual void visit(const AstFor &ref) = 0;
	virtual void visit(const AstWhile &ref) = 0;
	virtual void visit(const AstNewArray &ref) = 0;
	virtual void visit(const AstArrayValue &ref) = 0;
	virtual void visit(const AstArrayAssign &ref) = 0;
};
