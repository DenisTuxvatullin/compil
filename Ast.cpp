#include "Ast.h"
#include "Exception.h"

using namespace std;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  IAst
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

IAst::IAst()
{
}

IAst::~IAst()
{
}

bool IAst::IsExpression() const
{
	return false;
}

bool IAst::IsConstant() const
{
	return false;
}

bool IAst::IsError() const
{
	return false;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstError
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstError::AstError()
{
}

bool AstError::IsError() const
{
	return true;
}

void AstError::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstExpression
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstExpression::AstExpression(DataType exprType)
	: m_type(exprType)
{
}

bool AstExpression::IsExpression() const
{
	return true;
}

bool AstExpression::IsNumeric() const
{
	return m_type.GetType() == DATA_TYPE::TYPE_FLOAT ||
		m_type.GetType() == DATA_TYPE::TYPE_INT;
}

const DataType &AstExpression::GetType() const
{
	return m_type;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstVariable
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstVariable::AstVariable(const string &varName, DataType varType)
	: AstExpression(varType)
	, m_varName(varName)
{
}

const string &AstVariable::GetName() const
{
	return m_varName;
}

void AstVariable::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstUnaryExpression
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstUnaryExpression::AstUnaryExpression(KEYWORD op, shared_ptr<IAst> expr)
	: AstExpression(GetExpressionType(expr))
	, m_op(op)
	, m_expr(expr)
{
}

shared_ptr<IAst> AstUnaryExpression::GetExpression() const
{
	return m_expr;
}

KEYWORD AstUnaryExpression::GetOperator() const
{
	return m_op;
}

void AstUnaryExpression::accept(IVisitor &v)
{
	v.visit(*this);
}

DataType AstUnaryExpression::GetExpressionType(shared_ptr<IAst> expr)
{
	if (!expr->IsExpression())
	{
		throw InternalError("Attempt to create AstUnaryExpression from non AstExpression based node");
	}

	return dynamic_cast<AstExpression*>(expr.get())->GetType();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstBinaryExpression
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstBinaryExpression::AstBinaryExpression(shared_ptr<IAst> left, KEYWORD op, shared_ptr<IAst> right)
	: AstExpression(GetExpressionType(left, op, right, m_inType))
	, m_left(left)
	, m_op(op)
	, m_right(right)
{
}

shared_ptr<IAst> AstBinaryExpression::GetLeft() const
{
	return m_left;
}

shared_ptr<IAst> AstBinaryExpression::GetRight() const
{
	return m_right;
}

KEYWORD AstBinaryExpression::GetOperator() const
{
	return m_op;
}

DATA_TYPE AstBinaryExpression::GetInnerType() const
{
	return m_inType;
}

void AstBinaryExpression::accept(IVisitor &v)
{
	v.visit(*this);
}

DataType AstBinaryExpression::GetExpressionType(shared_ptr<IAst> left, KEYWORD op, shared_ptr<IAst> right, DATA_TYPE &inType)
{
	if (!left->IsExpression() || !right->IsExpression())
	{
		throw InternalError("Attempt to create AstBinaryExpression from non AstExpression based node");
	}

	AstExpression *leftEx = dynamic_cast<AstExpression*>(left.get());
	AstExpression *rightEx = dynamic_cast<AstExpression*>(right.get());

	if (leftEx->GetType() != rightEx->GetType())
	{
		throw IntermediateError("Type mismatch: " + leftEx->GetType().ToString() + " and " + rightEx->GetType().ToString());
	}

	inType = leftEx->GetType().GetType();

	// Comparison operators always return bool
	// Other operators retain data type of their operands
	switch (op)
	{
	case KW_EQUAL:
	case KW_NOT_EQ:
	case KW_LESS:
	case KW_LE_EQ:
	case KW_GREATER:
	case KW_GR_EQ:
		return DataType(ATOMIC_TYPE::TYPE_BOOL);
	default:
		return leftEx->GetType();
	}
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstAssignmentExpression
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstAssignmentExpression::AstAssignmentExpression(shared_ptr<AstVariable> variable, shared_ptr<AstExpression> expression)
	: m_variable(variable)
	, m_expression(expression)
{
	if (m_variable->GetType() != m_expression->GetType())
	{
		throw IntermediateError("Type mismatch: value of type " + m_expression->GetType().ToString() + 
			" cannot be assigned to variable of type " + m_variable->GetType().ToString());
	}
}

shared_ptr<AstVariable> AstAssignmentExpression::GetVariable() const
{
	return m_variable;
}

shared_ptr<AstExpression> AstAssignmentExpression::GetExpression() const
{
	return m_expression;
}

void AstAssignmentExpression::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstReturn
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstReturn::AstReturn()
	: m_expr(nullptr)
{
}

AstReturn::AstReturn(shared_ptr<AstExpression> expr)
	: m_expr(expr)
{
}

bool AstReturn::HasExpression() const
{
	return m_expr != nullptr;
}

shared_ptr<AstExpression> AstReturn::GetExpression() const
{
	return m_expr;
}

void AstReturn::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstList
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstList::AstList()
{
}

void AstList::AddElement(shared_ptr<IAst> statement)
{
	m_statements.push_back(statement);
}

const vector<shared_ptr<IAst>> &AstList::GetElements() const
{
	return m_statements;
}

bool AstList::IsEmpty() const
{
	return m_statements.size() == 0;
}

void AstList::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstFunction
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstFunction::AstFunction(const string &name, DataType returnType, shared_ptr<AstList> arguments, shared_ptr<AstList> code)
	: m_name(name)
	, m_returnType(returnType)
	, m_code(code)
	, m_arguments(arguments)
{
}

const string &AstFunction::GetName() const
{
	return m_name;
}

const DataType &AstFunction::GetReturnType() const
{
	return m_returnType;
}

shared_ptr<AstList> AstFunction::GetArguments() const
{
	return m_arguments;
}

shared_ptr<AstList> AstFunction::GetCode() const
{
	return m_code;
}

void AstFunction::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstGlobal
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstGlobal::AstGlobal(shared_ptr<AstList> declList)
	: m_declList(declList)
{
}

shared_ptr<AstList> AstGlobal::GetDeclarations() const
{
	return m_declList;
}

void AstGlobal::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstFunctionCall
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstFunctionCall::AstFunctionCall(const string &functionName, const DataType &returnType, shared_ptr<AstList> arguments)
	: AstExpression(returnType)
	, m_name(functionName)
	, m_args(arguments)
{
}

const string &AstFunctionCall::GetName() const
{
	return m_name;
}

shared_ptr<AstList> AstFunctionCall::GetArguments() const
{
	return m_args;
}

void AstFunctionCall::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstIf
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstIf::AstIf(shared_ptr<IAst> condition,
	shared_ptr<IAst> consequent, shared_ptr<IAst> alternative)
	: m_consequent(consequent)
	, m_alternative(alternative)
{
	m_condition = dynamic_pointer_cast<AstExpression>(condition);
	if (m_condition == nullptr)
	{
		throw InternalError("AstIf::AstIf: condition is not an expression");
	}
	if (m_condition->GetType().GetType() != DATA_TYPE::TYPE_BOOL)
	{
		throw IntermediateError("If statement requires a boolean expression as its condition");
	}
}

shared_ptr<AstExpression> AstIf::GetCondition() const
{
	return m_condition;
}

shared_ptr<IAst> AstIf::GetConsequent() const
{
	return m_consequent;
}

shared_ptr<IAst> AstIf::GetAlternative() const
{
	return m_alternative;
}

bool AstIf::HasElse() const
{
	return m_alternative != nullptr;
}

void AstIf::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstNewDict
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstNewDict::AstNewDict(ATOMIC_TYPE keyType, ATOMIC_TYPE valueType)
	: AstExpression(DataType(keyType, valueType))
{
}

void AstNewDict::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstDictValue
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstDictValue::AstDictValue(const string &dictName, ATOMIC_TYPE keyType, ATOMIC_TYPE valueType, shared_ptr<IAst> key)
	: AstExpression(DataType(valueType))
	, m_dictType(keyType, valueType)
	, m_name(dictName)
{
	m_key = dynamic_pointer_cast<AstExpression>(key);
	if (m_key == nullptr)
	{
		throw InternalError("AstDictValue: key must be an expression");
	}
	if (m_key->GetType().GetType() != static_cast<DATA_TYPE>(keyType))
	{
		throw IntermediateError("Dictionary key type mismatch");
	}
}

shared_ptr<AstExpression> AstDictValue::GetKey() const
{
	return m_key;
}

const DataType &AstDictValue::GetDictType() const
{
	return m_dictType;
}

const string &AstDictValue::GetName() const
{
	return m_name;
}

void AstDictValue::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstDictAssign
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstDictAssign::AstDictAssign(std::shared_ptr<AstDictValue> dict, std::shared_ptr<AstExpression> expression)
	: m_dict(dict)
	, m_expression(expression)
{
}

std::shared_ptr<AstDictValue> AstDictAssign::GetDictValue() const
{
	return m_dict;
}

std::shared_ptr<AstExpression> AstDictAssign::GetExpression() const
{
	return m_expression;;
}

void AstDictAssign::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstConstantDecl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstConstantDecl::AstConstantDecl(shared_ptr<ConstantBase> constant)
	: m_const(constant)
{
}

shared_ptr<ConstantBase> AstConstantDecl::GetConst() const
{
	return m_const;
}

void AstConstantDecl::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstFor
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstFor::AstFor(shared_ptr<IAst> from, shared_ptr<IAst> to,
	shared_ptr<IAst> step, shared_ptr<IAst> code)
	: m_code(code)
{
	m_from = dynamic_pointer_cast<AstAssignmentExpression>(from);
	if (!m_from)
	{
		throw InternalError("AstFor::AstFor: from must be AstAssignmentExpression");
	}
	if (!m_from->GetVariable()->GetType().IsNumeric())
	{
		throw IntermediateError("Counter variable in FOR loop must be numeric");
	}
	DATA_TYPE counterType = m_from->GetVariable()->GetType().GetType();

	m_to = dynamic_pointer_cast<AstExpression>(to);
	if (!m_to)
	{
		throw InternalError("AstFor::AstFor: to must be AstExpression");
	}
	if (m_to->GetType().GetType() != counterType)
	{
		throw IntermediateError("Type mismatch: type of TO differs from type of FROM in FOR loop");
	}

	if (step)
	{
		m_step = dynamic_pointer_cast<AstExpression>(step);
		if (!m_step)
		{
			throw InternalError("AstFor::AstFor: step must be AstExpression");
		}
		if (m_step->GetType().GetType() != counterType)
		{
			throw IntermediateError("Type mismatch: type of STEP differs from type of FROM in FOR loop");
		}
	}
	else
	{
		if (counterType == DATA_TYPE::TYPE_INT)
		{
			m_step = make_shared<AstIntConst>(1);
		}
		else if (counterType == DATA_TYPE::TYPE_FLOAT)
		{
			m_step = make_shared<AstFloatConst>(1.0);
		}
		else
		{
			throw InternalError("AstFor::AstFor: unknown counter type");
		}
	}
}

shared_ptr<AstAssignmentExpression> AstFor::GetFrom() const
{
	return m_from;
}

shared_ptr<AstExpression> AstFor::GetTo() const
{
	return m_to;
}

shared_ptr<AstExpression> AstFor::GetStep() const
{
	return m_step;
}

shared_ptr<IAst> AstFor::GetCode() const
{
	return m_code;
}

void AstFor::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstWhile
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstWhile::AstWhile(shared_ptr<IAst> condition, shared_ptr<IAst> code,
	bool isDoWhile)
	: m_dowhile(isDoWhile)
	, m_code(code)
{
	m_condition = dynamic_pointer_cast<AstExpression>(condition);
	if (!m_condition)
	{
		throw InternalError("AstWhile::AstWhile: condition must be AstExpression");
	}
	if (m_condition->GetType().GetType() != DATA_TYPE::TYPE_BOOL)
	{
		throw IntermediateError("Condition in WHILE loop must be boolean");
	}
}

shared_ptr<AstExpression> AstWhile::GetCondition() const
{
	return m_condition;
}

shared_ptr<IAst> AstWhile::GetCode() const
{
	return m_code;
}

bool AstWhile::IsDoWhile() const
{
	return m_dowhile;
}

void AstWhile::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstNewArray
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstNewArray::AstNewArray(ATOMIC_TYPE dataType, shared_ptr<AstList> dimensions)
	: AstExpression(DataType(dimensions->GetElements().size(), dataType))
	, m_baseType(dataType)
{
	for (auto dimAst : dimensions->GetElements())
	{
		shared_ptr<AstExpression> dimExpr = dynamic_pointer_cast<AstExpression>(dimAst);
		if (!dimExpr)
		{
			throw InternalError("AstNewArray::AstNewArray: dimExpr is not AstExpression");
		}
		if (dimExpr->GetType().GetType() != DATA_TYPE::TYPE_INT)
		{
			throw IntermediateError("Array length must be int");
		}

		m_dimensions.push_back(dimExpr);
	}
}

const vector<shared_ptr<AstExpression>> &AstNewArray::GetDimensions() const
{
	return m_dimensions;
}

ATOMIC_TYPE AstNewArray::GetBaseType() const
{
	return m_baseType;
}

void AstNewArray::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstArrayValue
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstArrayValue::AstArrayValue(const string &name, ATOMIC_TYPE arrayType,
	shared_ptr<AstList> indexes)
	: AstExpression(DataType(arrayType))
	, m_indexes(indexes)
	, m_name(name)
{
	const auto &elem = indexes->GetElements();

	if (elem.size() < 1 ||
		elem.size() > 4)
	{
		throw IntermediateError("Array value can have between 1 and 4 indexes");
	}

	for (auto indexAst : elem)
	{
		auto index = dynamic_pointer_cast<AstExpression>(indexAst);
		if (!index)
		{
			throw InternalError("AstArrayValue::AstArrayValue: index must be AstExpression");
		}
		if (index->GetType().GetType() != DATA_TYPE::TYPE_INT)
		{
			throw IntermediateError("Array index must be of type int");
		}
	}
}

const string &AstArrayValue::GetName() const
{
	return m_name;
}

shared_ptr<AstList> AstArrayValue::GetIndexes() const
{
	return m_indexes;
}

DataType AstArrayValue::GetArrayType() const
{
	return DataType(m_indexes->GetElements().size(), (ATOMIC_TYPE)GetType().GetType());
}

void AstArrayValue::accept(IVisitor &v)
{
	v.visit(*this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AstArrayAssign
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

AstArrayAssign::AstArrayAssign(shared_ptr<IAst> elem, shared_ptr<IAst> expression)
{
	m_elem = dynamic_pointer_cast<AstArrayValue>(elem);
	if (!m_elem)
	{
		throw InternalError("AstArrayAssign::AstArrayAssign: elem must be AstArrayValue");
	}

	m_expression = dynamic_pointer_cast<AstExpression>(expression);
	if (!m_expression)
	{
		throw InternalError("AstArrayAssign::AstArrayAssign: expression must be AstExpression");
	}

	if (m_elem->GetType() != m_expression->GetType())
	{
		throw IntermediateError("Type mismatch");
	}
}

shared_ptr<AstArrayValue> AstArrayAssign::GetElem() const
{
	return m_elem;
}

shared_ptr<AstExpression> AstArrayAssign::GetExpression() const
{
	return m_expression;
}

void AstArrayAssign::accept(IVisitor &v)
{
	v.visit(*this);
}
