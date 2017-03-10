#pragma once
#include <string>
#include <memory>
#include <vector>
#include "DataType.h"
#include "Visitor.h"
#include "Token.h"
#include "Constant.h"

class IAst
{
protected:
	IAst();

public:
	virtual ~IAst();

	virtual bool IsExpression() const;
	virtual bool IsConstant() const;
	virtual bool IsError() const;

	virtual void accept(IVisitor &v) = 0;
};

class AstError : public IAst
{
public:
	AstError();

	bool IsError() const override;

	void accept(IVisitor &v) override;
};

// Anything that has a return value is based off this class
class AstExpression : public IAst
{
protected:
	AstExpression(DataType exprType);

public:
	bool IsExpression() const override;
	bool IsNumeric() const;

	const DataType &GetType() const;

private:
	DataType m_type;
};

// Constant (has a name, data type and a value)
template<class T, ATOMIC_TYPE DATA_TYPE> class AstConstant : public AstExpression
{
public:
	AstConstant(T value)
		: AstExpression(DATA_TYPE)
		, m_value(value)
	{
	}

	bool IsConstant() const override
	{
		return true;
	}

	T GetValue() const
	{
		return m_value;
	}

	void accept(IVisitor &v) override
	{
		v.visit(*this);
	}

private:
	T m_value;
};

// AstVariable can be:
// Simple variable: i%, name$
// Array or dict declaration: matrix%[][], users$%()
class AstVariable : public AstExpression
{
public:
	AstVariable(const std::string &varName, DataType varType);

	const std::string &GetName() const;

	void accept(IVisitor &v) override;

private:
	std::string m_varName;
};

class AstUnaryExpression : public AstExpression
{
public:
	AstUnaryExpression(KEYWORD op, std::shared_ptr<IAst> expr);

	std::shared_ptr<IAst> GetExpression() const;
	KEYWORD GetOperator() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<IAst> m_expr;
	KEYWORD m_op;

	static DataType GetExpressionType(std::shared_ptr<IAst> expr);
};

class AstBinaryExpression : public AstExpression
{
public:
	AstBinaryExpression(std::shared_ptr<IAst> left, KEYWORD op, std::shared_ptr<IAst> right);

	std::shared_ptr<IAst> GetLeft() const;
	std::shared_ptr<IAst> GetRight() const;
	KEYWORD GetOperator() const;

	// Type of left and right parts of the expression
	// Because comparison expressions always return bool
	DATA_TYPE GetInnerType() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<IAst> m_left, m_right;
	KEYWORD m_op;
	DATA_TYPE m_inType;

	static DataType GetExpressionType(std::shared_ptr<IAst> left, KEYWORD op,
		std::shared_ptr<IAst> right, DATA_TYPE &inType);
};

// variable = expression
class AstAssignmentExpression : public IAst
{
public:
	AstAssignmentExpression(std::shared_ptr<AstVariable> variable, std::shared_ptr<AstExpression> expression);

	std::shared_ptr<AstVariable> GetVariable() const;
	std::shared_ptr<AstExpression> GetExpression() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstVariable> m_variable;
	std::shared_ptr<AstExpression> m_expression;
};

// return statement
class AstReturn : public IAst
{
public:
	// return (from procedure)
	AstReturn();
	// return ... (from function)
	AstReturn(std::shared_ptr<AstExpression> expr);

	bool HasExpression() const;
	std::shared_ptr<AstExpression> GetExpression() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstExpression> m_expr;
};

// Container for a sequence of IAst references
class AstList : public IAst
{
public:
	AstList();

	void AddElement(std::shared_ptr<IAst> statement);
	const std::vector<std::shared_ptr<IAst>> &GetElements() const;
	bool IsEmpty() const;

	void accept(IVisitor &v) override;

private:
	std::vector<std::shared_ptr<IAst>> m_statements;
};

// Everything about a function, including its code
class AstFunction : public IAst
{
public:
	AstFunction(const std::string &name, DataType returnType,
		std::shared_ptr<AstList> arguments, std::shared_ptr<AstList> code);

	const std::string &GetName() const;
	const DataType &GetReturnType() const;
	std::shared_ptr<AstList> GetArguments() const;
	std::shared_ptr<AstList> GetCode() const;

	void accept(IVisitor &v) override;

private:
	std::string m_name;
	DataType m_returnType;
	std::shared_ptr<AstList> m_arguments;
	std::shared_ptr<AstList> m_code;
};

class AstGlobal : public IAst
{
public:
	AstGlobal(std::shared_ptr<AstList> declList);

	std::shared_ptr<AstList> GetDeclarations() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstList> m_declList;
};

class AstFunctionCall : public AstExpression
{
public:
	AstFunctionCall(const std::string &functionName,
		const DataType &returnType, std::shared_ptr<AstList> arguments);

	const std::string &GetName() const;
	std::shared_ptr<AstList> GetArguments() const;

	void accept(IVisitor &v) override;

private:
	std::string m_name;
	std::shared_ptr<AstList> m_args;
};

// If statement
class AstIf : public IAst
{
public:
	AstIf(std::shared_ptr<IAst> condition,
		std::shared_ptr<IAst> consequent, std::shared_ptr<IAst> alternative);

	std::shared_ptr<AstExpression> GetCondition() const;
	// Consequent is a part of code that executes when condition is true
	std::shared_ptr<IAst> GetConsequent() const;
	// Alternative is a part of code that executes when condition is false (after else)
	// In case an if statement has no else clause, GetAlternative() returns nullptr
	std::shared_ptr<IAst> GetAlternative() const;

	bool HasElse() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstExpression> m_condition;
	std::shared_ptr<IAst> m_consequent;
	std::shared_ptr<IAst> m_alternative;
};

// While: 'while' Expression Code
// DoWhile: 'do' Code 'while' Expression
class AstWhile : public IAst
{
public:
	AstWhile(std::shared_ptr<IAst> condition, std::shared_ptr<IAst> code,
		bool isDoWhile);

	std::shared_ptr<AstExpression> GetCondition() const;
	std::shared_ptr<IAst> GetCode() const;
	bool IsDoWhile() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstExpression> m_condition;
	std::shared_ptr<IAst> m_code;
	bool m_dowhile;
};

// For: 'for' From 'to' To <'step' Step> Code
class AstFor : public IAst
{
public:
	// step can be null
	// in that case it'll be default (1)
	AstFor(std::shared_ptr<IAst> from, std::shared_ptr<IAst> to,
		std::shared_ptr<IAst> step, std::shared_ptr<IAst> code);

	std::shared_ptr<AstAssignmentExpression> GetFrom() const;
	std::shared_ptr<AstExpression> GetTo() const;
	std::shared_ptr<AstExpression> GetStep() const;
	std::shared_ptr<IAst> GetCode() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstAssignmentExpression> m_from;
	std::shared_ptr<AstExpression> m_to;
	std::shared_ptr<AstExpression> m_step;
	std::shared_ptr<IAst> m_code;
};

// new array%(10, 10, 20)
class AstNewArray : public AstExpression
{
public:
	AstNewArray(ATOMIC_TYPE dataType, std::shared_ptr<AstList> dimensions);

	const std::vector<std::shared_ptr<AstExpression>> &GetDimensions() const;
	ATOMIC_TYPE GetBaseType() const;

	void accept(IVisitor &v) override;

private:
	std::vector<std::shared_ptr<AstExpression>> m_dimensions;
	ATOMIC_TYPE m_baseType;
};

// arr%[0][1]
class AstArrayValue : public AstExpression
{
public:
	AstArrayValue(const std::string &name, ATOMIC_TYPE arrayType,
		std::shared_ptr<AstList> indexes);

	const std::string &GetName() const;
	std::shared_ptr<AstList> GetIndexes() const;
	DataType GetArrayType() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstList> m_indexes;
	std::string m_name;
};

// ArrayAssign: Elem = Expression
class AstArrayAssign : public IAst
{
public:
	AstArrayAssign(std::shared_ptr<IAst> elem, std::shared_ptr<IAst> expression);

	std::shared_ptr<AstArrayValue> GetElem() const;
	std::shared_ptr<AstExpression> GetExpression() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstArrayValue> m_elem;
	std::shared_ptr<AstExpression> m_expression;
};

// new dict$%()
class AstNewDict : public AstExpression
{
public:
	AstNewDict(ATOMIC_TYPE keyType, ATOMIC_TYPE valueType);

	void accept(IVisitor &v) override;
};

// Type in AstExpression is dict's value type
// Type returned by GetDictType() is full dict type
// Example: for dict$%("abc") 
// GetType() is int%
// GetDictType() is dict$%()
class AstDictValue : public AstExpression
{
public:
	AstDictValue(const std::string &dictName, ATOMIC_TYPE keyType, ATOMIC_TYPE valueType, std::shared_ptr<IAst> key);

	std::shared_ptr<AstExpression> GetKey() const;
	const DataType &GetDictType() const;
	const std::string &GetName() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstExpression> m_key;
	DataType m_dictType;
	std::string m_name;
};

class AstDictAssign : public IAst
{
public:
	AstDictAssign(std::shared_ptr<AstDictValue> dict, std::shared_ptr<AstExpression> expression);

	std::shared_ptr<AstDictValue> GetDictValue() const;
	std::shared_ptr<AstExpression> GetExpression() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<AstDictValue> m_dict;
	std::shared_ptr<AstExpression> m_expression;
};

// declaration of a single constant
class AstConstantDecl : public IAst
{
public:
	AstConstantDecl(std::shared_ptr<ConstantBase> constant);

	std::shared_ptr<ConstantBase> GetConst() const;

	void accept(IVisitor &v) override;

private:
	std::shared_ptr<ConstantBase> m_const;
};
