#include <memory>
#include <set>
#include <iostream>
#include <assert.h>
#include "TokenStream.h"
#include "Exception.h"
#include "DataType.h"
#include "Lexer.h"
#include "Function.h"
//#include "Compiler.h"
#include "Ast.h"
#include "Constant.h"

#include "LL.h"
#include "LR.h"

/*
-=-=-=- Backslash operator precedence -=-=-=-
1	()			Braces
	~			Unary minus
	not 		Logical NOT

2	* / mod		Multiplication, division, and remainder

3	+ - 		Addition and subtraction

4	< <= 		For relational operators < and ≤ respectively
	> >= 		For relational operators > and ≥ respectively

5	== <> 		For relational operators = and ≠ respectively

6	and 		Logical AND

7	or 			Logical OR
*/

// virtual function - when called, insted of normal call it inlines code specified in "inlineCode"


using namespace std;

// simple_const: IntNumber | FpNumber | String
shared_ptr<IAst> ParseConst(TokenStream &tk)
{
	TOKEN_TYPE tokenType = tk.Current().GetType();
	switch (tokenType)
	{
	case TOKEN_TYPE::TOKEN_FLOAT:
		{
			auto result = make_shared<AstFloatConst>(tk.Current().GetValueF());
			tk.Forward();
			return result;
		}
	case TOKEN_TYPE::TOKEN_STRING:
		{
			auto result = make_shared<AstStringConst>(tk.Current().GetValueS());
			tk.Forward();
			return result;
		}
	case TOKEN_TYPE::TOKEN_INT:
		{
			auto result = make_shared<AstIntConst>(tk.Current().GetValueI());
			tk.Forward();
			return result;
		}
	default:
		return make_shared<AstError>();
	}
}

bool ParseSimpleType(TokenStream &tk, ATOMIC_TYPE &varType)
{
	if (tk.Current().GetType() != TOKEN_DELIMETER)
	{
		return false;
	}

	KEYWORD typeKeyword = tk.Current().GetValueK();
	switch (typeKeyword)
	{
	case KW_STRING:
		varType = ATOMIC_TYPE::TYPE_STRING;
		break;
	case KW_INT:
		varType = ATOMIC_TYPE::TYPE_INT;
		break;
	case KW_FLOAT:
		varType = ATOMIC_TYPE::TYPE_FLOAT;
		break;
	case KW_BOOL:
		varType = ATOMIC_TYPE::TYPE_BOOL;
		break;
	default:
		return false;
	}

	tk.Forward();
	return true;
}

// simple_variable: id simple_type
shared_ptr<IAst> ParseSimpleVariable(TokenStream &tk)
{
	if (tk.Current().GetType() != TOKEN_ID)
	{
		return make_shared<AstError>();
	}

	tk.PushPosition();

	const string &varName = tk.Current().GetValueS();
	tk.Forward();

	ATOMIC_TYPE varType;
	if (!ParseSimpleType(tk, varType))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	tk.PopPosition();

	// TODO: check if varName is a const
	return make_shared<AstVariable>(varName, DataType(varType));
}

// array_decl: id simple_type ('[' ']')+
shared_ptr<IAst> ParseArrayDecl(TokenStream &tk)
{
	if (tk.Current().GetType() != TOKEN_ID)
	{
		return make_shared<AstError>();
	}

	tk.PushPosition();

	const string &varName = tk.Current().GetValueS();
	tk.Forward();

	ATOMIC_TYPE varType;
	if (!ParseSimpleType(tk, varType))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	unsigned dimension = 0;
	while (tk.Current().IsKeyword(KW_INDEX_L))
	{
		tk.Forward();
		if (!tk.Current().IsKeyword(KW_INDEX_R))
		{
			tk.RestorePosition();
			return make_shared<AstError>();
		}
		tk.Forward();
		dimension++;
	}

	if (dimension == 0)
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	tk.PopPosition();

	return make_shared<AstVariable>(varName, DataType(dimension, varType));
}

shared_ptr<IAst> ParseExpression(TokenStream &tk);

// DictDecl: id SimpleType SimpleType '(' ')'
// DictValue: id SimpleType SimpleType '(' Expression ')'
shared_ptr<IAst> ParseDict(TokenStream &tk, bool parseKey = false)
{
	if (tk.Current().GetType() != TOKEN_ID)
	{
		return make_shared<AstError>();
	}

	tk.PushPosition();

	const string &varName = tk.Current().GetValueS();
	tk.Forward();

	ATOMIC_TYPE keyType, valueType;
	if (!ParseSimpleType(tk, keyType) || !ParseSimpleType(tk, valueType))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	if (!tk.Current().IsKeyword(KW_BRACE_L))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	tk.Forward();

	shared_ptr<IAst> key;
	if (parseKey)
	{
		key = ParseExpression(tk);
		if (key->IsError())
		{
			tk.RestorePosition();
			return make_shared<AstError>();
		}
	}

	if (!tk.Current().IsKeyword(KW_BRACE_R))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	tk.Forward();

	tk.PopPosition();
	if (parseKey)
	{
		return make_shared<AstDictValue>(varName, keyType, valueType, key);
	}
	else
	{
		return make_shared<AstVariable>(varName, DataType(keyType, valueType));
	}
}

// variable: dict_decl | array_decl | simple_variable
shared_ptr<IAst> ParseVariableRDP(TokenStream &tk)
{
	shared_ptr<IAst> result = ParseDict(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseArrayDecl(tk);
	if (!result->IsError())
	{
		return result;
	}

	return ParseSimpleVariable(tk);
}

shared_ptr<IAst> ParseVariable(TokenStream &tk)
{
	return ParseVariableLR(tk);
}

shared_ptr<IAst> ParseAndExpression(TokenStream &tk);
shared_ptr<IAst> ParseEqualityExpression(TokenStream &tk);
shared_ptr<IAst> ParseComparisonExpression(TokenStream &tk);
shared_ptr<IAst> ParseAdditiveExpression(TokenStream &tk);
shared_ptr<IAst> ParseMultiplicativeExpression(TokenStream &tk);
shared_ptr<IAst> ParsePrimary(TokenStream &tk);

/*
  applied to int, 'or' and 'and' are bitwise
  applied to bool, 'or' and 'and' are logical
  applied to string, '+' is concatenation
  array and dict are references

  This tables shows which operators can be applied to which types

            or | and | == | <> | < | <= | > | >= | + | - | * | / | mod | ~ | not
  string     - |  -  |  + |  + | - | -  | - | -  | + | - | - | - |  -  | - |  -
  int        + |  +  |  + |  + | + | +  | + | +  | + | + | + | + |  +  | + |  -
  float      - |  -  |  + |  + | + | +  | + | +  | + | + | + | + |  -  | + |  -
  bool       + |  +  |  + |  + | - | -  | - | -  | - | - | - | - |  -  | - |  +
  array      - |  -  |  - |  - | - | -  | - | -  | - | - | - | - |  -  | - |  -
  dict       - |  -  |  - |  - | - | -  | - | -  | - | - | - | - |  -  | - |  -
*/

typedef shared_ptr<IAst> (*OPTIMIZE_FUNCT)(AstExpression *left, AstExpression *right);
typedef pair<DATA_TYPE, KEYWORD> TypeOpPair;
map<TypeOpPair, OPTIMIZE_FUNCT> OperatorMap;

#define DEFINE_CALC_FUNCT(FUNCT_NAME, CALC_OPERATOR, RETURN_TYPE) \
	template<class CONST_TYPE> shared_ptr<IAst> FUNCT_NAME(AstExpression *left, AstExpression *right) \
	{ \
		auto LeftConst = dynamic_cast<CONST_TYPE*>(left); \
		auto RightConst = dynamic_cast<CONST_TYPE*>(right); \
		return make_shared<RETURN_TYPE>(LeftConst->GetValue() CALC_OPERATOR RightConst->GetValue()); \
	}

DEFINE_CALC_FUNCT(CalcBitwiseOr, |, AstIntConst)
DEFINE_CALC_FUNCT(CalcBitwiseAnd, &, AstIntConst)
DEFINE_CALC_FUNCT(CalcLogicalOr, ||, AstBoolConst)
DEFINE_CALC_FUNCT(CalcLogicalAnd, &&, AstBoolConst)

DEFINE_CALC_FUNCT(CalcEq, ==, AstBoolConst)
DEFINE_CALC_FUNCT(CalcNotEq, !=, AstBoolConst)
DEFINE_CALC_FUNCT(CalcLess, <, AstBoolConst)
DEFINE_CALC_FUNCT(CalcGreater, >, AstBoolConst)
DEFINE_CALC_FUNCT(CalcLessEq, <=, AstBoolConst)
DEFINE_CALC_FUNCT(CalcGreaterEq, >=, AstBoolConst)

DEFINE_CALC_FUNCT(CalcPlus, +, CONST_TYPE)
DEFINE_CALC_FUNCT(CalcMinus, -, CONST_TYPE)
DEFINE_CALC_FUNCT(CalcMul, *, CONST_TYPE)
DEFINE_CALC_FUNCT(CalcMod, %, CONST_TYPE)

//DEFINE_CALC_FUNCT(CalcDiv, /, CONST_TYPE)
template<class CONST_TYPE> shared_ptr<IAst> CalcDiv(AstExpression *left, AstExpression *right)
{
	auto LeftConst = dynamic_cast<CONST_TYPE*>(left);
	auto RightConst = dynamic_cast<CONST_TYPE*>(right);

	if (RightConst->GetValue() == 0)
	{
		throw IntermediateError("Division by zero");
	}

	return make_shared<CONST_TYPE>(LeftConst->GetValue() / RightConst->GetValue());
}

void InitOperatorMap()
{
	static bool IsMapInitialized = false;

	if (!IsMapInitialized)
	{
		IsMapInitialized = true;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_OR)] = &CalcBitwiseOr<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_AND)] = &CalcBitwiseAnd<AstIntConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_BOOL, KW_OR)] = &CalcLogicalOr<AstBoolConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_BOOL, KW_AND)] = &CalcLogicalAnd<AstBoolConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_STRING, KW_EQUAL)] = &CalcEq<AstStringConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_EQUAL)] = &CalcEq<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_EQUAL)] = &CalcEq<AstFloatConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_BOOL, KW_EQUAL)] = &CalcEq<AstBoolConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_STRING, KW_NOT_EQ)] = &CalcNotEq<AstStringConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_NOT_EQ)] = &CalcNotEq<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_NOT_EQ)] = &CalcNotEq<AstFloatConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_BOOL, KW_NOT_EQ)] = &CalcNotEq<AstBoolConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_LESS)] = &CalcLess<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_LESS)] = &CalcLess<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_GREATER)] = &CalcGreater<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_GREATER)] = &CalcGreater<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_LE_EQ)] = &CalcLessEq<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_LE_EQ)] = &CalcLessEq<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_GR_EQ)] = &CalcGreaterEq<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_GR_EQ)] = &CalcGreaterEq<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_STRING, KW_PLUS)] = &CalcPlus<AstStringConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_PLUS)] = &CalcPlus<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_PLUS)] = &CalcPlus<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_MINUS)] = &CalcMinus<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_MINUS)] = &CalcMinus<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_MUL)] = &CalcMul<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_MUL)] = &CalcMul<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_DIV)] = &CalcDiv<AstIntConst>;
		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_FLOAT, KW_DIV)] = &CalcDiv<AstFloatConst>;

		OperatorMap[TypeOpPair(DATA_TYPE::TYPE_INT, KW_MOD)] = &CalcMod<AstIntConst>;
	}
}

string FormatInvalidOpError(KEYWORD op, const DataType &dataType)
{
	string message("Operator '");

	switch (op)
	{
	case KW_OR:
		message += "or";
		break;
	case KW_AND:
		message += "and";
		break;
	case KW_EQUAL:
		message += "==";
		break;
	case KW_NOT_EQ:
		message += "<>";
		break;
	case KW_LESS:
		message += "<";
		break;
	case KW_LE_EQ:
		message += "<=";
		break;
	case KW_GREATER:
		message += ">";
		break;
	case KW_GR_EQ:
		message += ">=";
		break;
	case KW_PLUS:
		message += "+";
		break;
	case KW_MINUS:
		message += "-";
		break;
	case KW_MUL:
		message += "*";
		break;
	case KW_DIV:
		message += "/";
		break;
	case KW_MOD:
		message += "mod";
		break;
	case KW_UNARY_MINUS:
		message += "~";
		break;
	case KW_NOT:
		message += "not";
		break;
	default:
		throw InternalError("Invalid operator for FormatInvalidOpError");
	}

	message += "' cannot be applied to type " + dataType.ToString();

	return message;
}

// Performs various checks to make sure binary expression is valid
// Optimizes it if possible
shared_ptr<IAst> CreateBinaryExpression(shared_ptr<IAst> left, KEYWORD op, shared_ptr<IAst> right)
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

	DATA_TYPE exprType = leftEx->GetType().GetType();
	auto exprQualifier = TypeOpPair(exprType, op);
	if (OperatorMap.find(exprQualifier) == OperatorMap.end())
	{
		throw IntermediateError(FormatInvalidOpError(op, leftEx->GetType()));
	}
	
	// If both sides of expression are constant, we can optimize it
	// by calculating the result at compile time
	if (left->IsConstant() && right->IsConstant())
	{
		return OperatorMap.at(exprQualifier)(leftEx, rightEx);
	}
	// Boolean operators can be optimized if at least one operand is constant
	else if (exprType == DATA_TYPE::TYPE_BOOL && (left->IsConstant() || right->IsConstant()))
	{
		shared_ptr<IAst> dynamicPart;
		AstBoolConst *constPart;

		if (left->IsConstant())
		{
			constPart = dynamic_cast<AstBoolConst*>(leftEx);
			dynamicPart = right;
		}
		else
		{
			constPart = dynamic_cast<AstBoolConst*>(rightEx);
			dynamicPart = left;
		}

		if (op == KW_AND)
		{
			if (constPart->GetValue() == true)
			{
				// TRUE AND A == A
				return dynamicPart;
			}
			else
			{
				// FALSE AND A == FALSE
				return make_shared<AstBoolConst>(false);
			}
		}
		else if (op == KW_OR)
		{
			if (constPart->GetValue() == true)
			{
				// TRUE OR A == TRUE
				return make_shared<AstBoolConst>(true);
			}
			else
			{
				// FALSE OR A == A
				return dynamicPart;
			}
		}
	}

	// Otherwise we return binary expression as it is
	return make_shared<AstBinaryExpression>(left, op, right);
}

shared_ptr<IAst> CreateUnaryExpression(KEYWORD op, shared_ptr<IAst> ast_expr)
{
	AstExpression *expr = dynamic_cast<AstExpression*>(ast_expr.get());

	switch (op)
	{
	case KW_UNARY_MINUS:
		switch (expr->GetType().GetType())
		{
		case DATA_TYPE::TYPE_INT:
			if (expr->IsConstant())
			{
				AstIntConst *intConst = dynamic_cast<AstIntConst*>(expr);
				return make_shared<AstIntConst>(-intConst->GetValue());
			}
			break;
		case DATA_TYPE::TYPE_FLOAT:
			if (expr->IsConstant())
			{
				AstFloatConst *floatConst = dynamic_cast<AstFloatConst*>(expr);
				return make_shared<AstFloatConst>(-floatConst->GetValue());
			}
			break;
		default:
			throw IntermediateError(FormatInvalidOpError(op, expr->GetType()));
		}
		break;
	case KW_NOT:
		if (expr->GetType().GetType() != DATA_TYPE::TYPE_BOOL)
		{
			throw IntermediateError(FormatInvalidOpError(op, expr->GetType()));
		}
		if (expr->IsConstant())
		{
			AstBoolConst *boolConst = dynamic_cast<AstBoolConst*>(expr);
			return make_shared<AstBoolConst>(!boolConst->GetValue());
		}
		break;
	default:
		throw InternalError("Invalid operator for AstUnaryExpression");
	}

	return make_shared<AstUnaryExpression>(op, ast_expr);
}

typedef shared_ptr<IAst> (*PARSE_FUNCT)(TokenStream &tk);
// BinaryExpression: parseNext ( {acceptableTokens} parseNext )+
shared_ptr<IAst> ParseBinaryExpression(TokenStream &tk, const set<KEYWORD> &acceptableTokens, PARSE_FUNCT parseNext)
{
	tk.PushPosition();

	shared_ptr<IAst> left = parseNext(tk);
	if (left->IsError())
	{
		tk.RestorePosition();
		return left;
	}

	while ((tk.Current().GetType() == TOKEN_DELIMETER ||
		tk.Current().GetType() == TOKEN_KEYWORD) && 
		(acceptableTokens.find(tk.Current().GetValueK()) != acceptableTokens.end()))
	{
		tk.PopPosition();
		tk.PushPosition();

		KEYWORD op = tk.Current().GetValueK();
		tk.Forward();

		shared_ptr<IAst> right = parseNext(tk);
		if (right->IsError())
		{
			tk.RestorePosition();
			return left;
		}
		else
		{
			left = CreateBinaryExpression(left, op, right);
		}
	}

	tk.PopPosition();
	return left;
}

// Expression: OrExpression | OpNew
// Expression: AndExpression < 'or' Expression >
shared_ptr<IAst> ParseExpression(TokenStream &tk)
{
	set<KEYWORD> tokens;
	tokens.insert(KW_OR);
	return ParseBinaryExpression(tk, tokens, &ParseAndExpression);
}

// AndExpression: EqualityExpression < 'and' Expression >
shared_ptr<IAst> ParseAndExpression(TokenStream &tk)
{
	set<KEYWORD> tokens;
	tokens.insert(KW_AND);
	return ParseBinaryExpression(tk, tokens, &ParseEqualityExpression);
}

// EqualityExpression: ComparisonExpression < ( '==' | '<>' ) Expression >
shared_ptr<IAst> ParseEqualityExpression(TokenStream &tk)
{
	set<KEYWORD> tokens;
	tokens.insert(KW_EQUAL);
	tokens.insert(KW_NOT_EQ);
	return ParseBinaryExpression(tk, tokens, &ParseComparisonExpression);
}

// ComparisonExpression: AdditiveExpression < ( '<' | '<=' |  '>' | '>=' ) Expression >
shared_ptr<IAst> ParseComparisonExpression(TokenStream &tk)
{
	set<KEYWORD> tokens;
	tokens.insert(KW_LESS);
	tokens.insert(KW_GREATER);
	tokens.insert(KW_GR_EQ);
	tokens.insert(KW_LE_EQ);
	return ParseBinaryExpression(tk, tokens, &ParseAdditiveExpression);
}

// AdditiveExpression: MultiplicativeExpression < ( '+' | '-' ) Expression >
shared_ptr<IAst> ParseAdditiveExpression(TokenStream &tk)
{
	set<KEYWORD> tokens;
	tokens.insert(KW_PLUS);
	tokens.insert(KW_MINUS);
	return ParseBinaryExpression(tk, tokens, &ParseMultiplicativeExpression);
}

// MultiplicativeExpression: Primary < ( '*' | '/' | 'mod' ) Expression >
shared_ptr<IAst> ParseMultiplicativeExpression(TokenStream &tk)
{
	set<KEYWORD> tokens;
	tokens.insert(KW_MUL);
	tokens.insert(KW_DIV);
	tokens.insert(KW_MOD);
	return ParseBinaryExpression(tk, tokens, &ParsePrimary);
}

shared_ptr<AstList> ParseCommaSeparatedList(TokenStream &tk, PARSE_FUNCT parseElement);

// new array%(10, 10, 20)
// new dict$%()
shared_ptr<IAst> ParseNewRDP(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_NEW))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	if (tk.Current().IsKeyword(KW_ARRAY))
	{
		tk.Forward();
		
		ATOMIC_TYPE dataType;
		if (!ParseSimpleType(tk, dataType))
		{
			throw IntermediateError("Expected array type");
		}

		if (!tk.Current().IsKeyword(KW_BRACE_L))
		{
			throw IntermediateError("Expected '('");
		}
		tk.Forward();

		auto dimensions = ParseCommaSeparatedList(tk, &ParseExpression);
		if (dimensions->GetElements().size() < 1 ||
			dimensions->GetElements().size() > 4)
		{
			throw IntermediateError("Only 1,2,3 and 4-dimension arrays are supported");
		}

		if (!tk.Current().IsKeyword(KW_BRACE_R))
		{
			throw IntermediateError("Expected ')'");
		}
		tk.Forward();

		return make_shared<AstNewArray>(dataType, dimensions);
	}
	else if (tk.Current().IsKeyword(KW_DICT))
	{
		tk.Forward();

		ATOMIC_TYPE keyType, valueType;
		if (!ParseSimpleType(tk, keyType) || !ParseSimpleType(tk, valueType))
		{
			throw IntermediateError("Expected key type and value type");
		}

		if (!tk.Current().IsKeyword(KW_BRACE_L))
		{
			throw IntermediateError("Expected '('");
		}
		tk.Forward();

		if (!tk.Current().IsKeyword(KW_BRACE_R))
		{
			throw IntermediateError("Expected ')'");
		}
		tk.Forward();

		return make_shared<AstNewDict>(keyType, valueType);
	}
	else
	{
		throw IntermediateError("Expected 'array' or 'dict'");
	}
}

shared_ptr<IAst> ParseCall(TokenStream &tk, bool discardResult);

// arr%[0][1]
// id SimpleType ('[' Expression ']')+
shared_ptr<IAst> ParseArrayValue(TokenStream &tk)
{
	if (tk.Current().GetType() != TOKEN_ID)
	{
		return make_shared<AstError>();
	}

	tk.PushPosition();

	const string &arrayName = tk.Current().GetValueS();
	tk.Forward();

	ATOMIC_TYPE arrayType;
	if (!ParseSimpleType(tk, arrayType))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	auto indexes = make_shared<AstList>();

	while (tk.Current().IsKeyword(KW_INDEX_L))
	{
		tk.Forward();

		auto expr = ParseExpression(tk);
		if (expr->IsError())
		{
			tk.RestorePosition();
			return make_shared<AstError>();
		}
		indexes->AddElement(expr);

		if (!tk.Current().IsKeyword(KW_INDEX_R))
		{
			throw IntermediateError("Expected ']'");
		}
		tk.Forward();
	}
	
	if (indexes->GetElements().size() == 0)
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	tk.PopPosition();
	return make_shared<AstArrayValue>(arrayName, arrayType, indexes);
}

// Primary: '(' Expression ')' | '~' Primary | 'not' Expression | Const | Variable | Call | New
// ArrayValue | DictValue
shared_ptr<IAst> ParsePrimary(TokenStream &tk)
{
	if (tk.Current().GetType() == TOKEN_DELIMETER || 
		tk.Current().GetType() == TOKEN_KEYWORD)
	{
		KEYWORD currentKw = tk.Current().GetValueK();
		switch (currentKw)
		{
		case KW_UNARY_MINUS:
		case KW_NOT:
			{
				tk.Forward();

				shared_ptr<IAst> expr = currentKw == KW_UNARY_MINUS ? ParsePrimary(tk) : ParseExpression(tk);
				if (expr->IsError())
				{
					throw IntermediateError("Expected expression");
				}

				return CreateUnaryExpression(currentKw, expr);
			}
		case KW_BRACE_L:
			{
				tk.Forward();

				shared_ptr<IAst> expr = ParseExpression(tk);
				if (expr->IsError())
				{
					throw IntermediateError("Expected expression");
				}

				if (!tk.Current().IsKeyword(KW_BRACE_R))
				{
					throw IntermediateError("Expected closing brace ')'");
				}

				tk.Forward();

				return expr;
			}
		}
	}

	shared_ptr<IAst> result = ParseCall(tk, false);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseDict(tk, true);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseArrayValue(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = LLParseNew(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseVariable(tk);
	if (!result->IsError())
	{
		return result;
	}

	return ParseConst(tk);
}

// ArrayAssign: ArrayValue = Expression
shared_ptr<IAst> ParseArrayAssignment(TokenStream &tk)
{
	tk.PushPosition();

	auto elem = ParseArrayValue(tk);
	if (elem->IsError())
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	if (!tk.Current().IsKeyword(KW_ASSIGN))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	tk.Forward();

	auto expr = ParseExpression(tk);
	if (expr->IsError())
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	tk.PopPosition();
	return make_shared<AstArrayAssign>(elem, expr);
}

// Assignment: Variable '=' Expression
shared_ptr<IAst> ParseAssignment(TokenStream &tk)
{
	auto arrayAssignment = ParseArrayAssignment(tk);
	if (!arrayAssignment->IsError())
	{
		return arrayAssignment;
	}

	tk.PushPosition();

	shared_ptr<IAst> var = ParseDict(tk, true);
	bool dictAssign = true;
	if (var->IsError())
	{
		var = ParseVariable(tk);
		if (var->IsError())
		{
			tk.RestorePosition();
			return make_shared<AstError>();
		}
		else
		{
			dictAssign = false;
		}
	}

	if (!tk.Current().IsKeyword(KW_ASSIGN))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	tk.Forward();

	auto expr = ParseExpression(tk);
	if (expr->IsError())
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}

	tk.PopPosition();

	if (dictAssign)
	{
		return make_shared<AstDictAssign>(dynamic_pointer_cast<AstDictValue>(var),
			dynamic_pointer_cast<AstExpression>(expr));
	}
	else
	{
		return make_shared<AstAssignmentExpression>(dynamic_pointer_cast<AstVariable>(var),
			dynamic_pointer_cast<AstExpression>(expr));
	}
}

shared_ptr<IAst> ParseReturn(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_RETURN))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	auto expr = ParseExpression(tk);
	if (expr->IsError())
	{
		return make_shared<AstReturn>();
	}
	else
	{
		return make_shared<AstReturn>(dynamic_pointer_cast<AstExpression>(expr));
	}
}

// FunctionName: id <TypeSpecifier>
bool ParseFunctionName(TokenStream &tk, string &functionName, DataType &dataType)
{
	shared_ptr<IAst> nameType = ParseVariable(tk);
	if (!nameType->IsError())
	{
		AstVariable *var = dynamic_cast<AstVariable*>(nameType.get());
		functionName = var->GetName();
		dataType = var->GetType();
	}
	else if (tk.Current().GetType() == TOKEN_ID)
	{
		functionName = tk.Current().GetValueS();
		dataType = DataType(ATOMIC_TYPE::TYPE_VOID);
		tk.Forward();
	}
	else
	{
		return false;
	}

	return true;
}

// SequentialList: [parseElement]
shared_ptr<AstList> ParseSequentialList(TokenStream &tk, PARSE_FUNCT parseElement)
{
	shared_ptr<AstList> list = make_shared<AstList>();

	while (true)
	{
		shared_ptr<IAst> statement = parseElement(tk);

		if (statement->IsError())
		{
			break;
		}

		list->AddElement(statement);
	}

	return list;
}

// CommaSeparatedList: <parseElement [',' parseElement]>
shared_ptr<AstList> ParseCommaSeparatedList(TokenStream &tk, PARSE_FUNCT parseElement)
{
	shared_ptr<AstList> list = make_shared<AstList>();

	bool firstElement = true;
	while (true)
	{
		shared_ptr<IAst> astVar = parseElement(tk);
		if (astVar->IsError())
		{
			if (firstElement)
			{
				break;
			}
			else
			{
				throw IntermediateError("Expected element");
			}
		}
		list->AddElement(astVar);
		firstElement = false;

		if (!tk.Current().IsKeyword(KW_COMMA))
		{
			break;
		}
		tk.Forward();
	}

	return list;
}

// Call: FunctionName '(' <Expression [',' Expression]> ')'
shared_ptr<IAst> ParseCall(TokenStream &tk, bool discardResult)
{
	tk.PushPosition();

	string functName;
	DataType returnType(ATOMIC_TYPE::TYPE_VOID);
	if (!ParseFunctionName(tk, functName, returnType))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	
	if (!tk.Current().IsKeyword(KW_BRACE_L))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	tk.Forward();

	auto args = ParseCommaSeparatedList(tk, &ParseExpression);

	if (!tk.Current().IsKeyword(KW_BRACE_R))
	{
		tk.RestorePosition();
		return make_shared<AstError>();
	}
	tk.Forward();

	if (discardResult && returnType.IsNotVoid())
	{
		throw IntermediateError("In top level function calls return type must be converted to void");
	}

	tk.PopPosition();
	return make_shared<AstFunctionCall>(functName, returnType, args);
}

shared_ptr<AstList> ParseCode(TokenStream &tk);

// If: 'if' Expression Code <'else' Code>
shared_ptr<IAst> ParseIf(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_IF))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	auto condition = ParseExpression(tk);
	if (condition->IsError())
	{
		throw IntermediateError("Expected expression");
	}

	shared_ptr<IAst> consequent = ParseCode(tk);
	shared_ptr<IAst> alternative(nullptr);

	if (tk.Current().IsKeyword(KW_ELSE))
	{
		tk.Forward();

		alternative = ParseCode(tk);
	}

	return make_shared<AstIf>(condition, consequent, alternative);
}

// For: 'for' Assignment 'to' Expression <'step' Expression> Code
shared_ptr<IAst> ParseFor(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_FOR))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	auto from = ParseAssignment(tk);
	if (from->IsError())
	{
		throw IntermediateError("Expected assignment"); 
	}

	if (!tk.Current().IsKeyword(KW_TO))
	{
		throw IntermediateError("Expected 'to'");
	}
	tk.Forward();

	auto to = ParseExpression(tk);
	if (to->IsError())
	{
		throw IntermediateError("Expected expression"); 
	}

	shared_ptr<IAst> step;
	if (tk.Current().IsKeyword(KW_STEP))
	{
		tk.Forward();

		step = ParseExpression(tk);
		if (step->IsError())
		{
			throw IntermediateError("Expected expression"); 
		}
	}

	auto code = ParseCode(tk);

	return make_shared<AstFor>(from, to, step, code);
}

// While: 'while' Expression Code
shared_ptr<IAst> ParseWhile(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_WHILE))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	auto condition = ParseExpression(tk);
	if (condition->IsError())
	{
		throw IntermediateError("Expected expression"); 
	}

	auto code = ParseCode(tk);
	if (code->IsError())
	{
		throw IntermediateError("Expected code"); 
	}

	return make_shared<AstWhile>(condition, code, false);
}

// DoWhile: 'do' Code 'while' Expression
shared_ptr<IAst> ParseDoWhile(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_DO))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	auto code = ParseCode(tk);
	if (code->IsError())
	{
		throw IntermediateError("Expected code"); 
	}

	if (!tk.Current().IsKeyword(KW_WHILE))
	{
		throw IntermediateError("Expected 'while'");
	}
	tk.Forward();

	auto condition = ParseExpression(tk);
	if (condition->IsError())
	{
		throw IntermediateError("Expected expression"); 
	}

	return make_shared<AstWhile>(condition, code, true);
}

// Statement: Assignment | Return | If | For | While | DoWhile | Call
shared_ptr<IAst> ParseStatement(TokenStream &tk)
{
	auto result = ParseAssignment(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseReturn(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseIf(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseFor(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseWhile(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseDoWhile(tk);
	if (!result->IsError())
	{
		return result;
	}

	return ParseCall(tk, true);
}

// Code: '{' [Statement] '}'
shared_ptr<AstList> ParseCode(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_BLOCK_L))
	{
		throw IntermediateError("Expected '{'");
	}
	tk.Forward();

	shared_ptr<AstList> result = ParseSequentialList(tk, &ParseStatement);

	if (!tk.Current().IsKeyword(KW_BLOCK_R))
	{
		throw IntermediateError("Expected '}'");
	}
	tk.Forward();

	return result;
}

// Function: 'def' {Variable Id} '(' <Variable [',' Variable]> ')' Code
shared_ptr<IAst> ParseFunction(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_DEF))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	string functionName;
	DataType returnType(ATOMIC_TYPE::TYPE_VOID);
	if (!ParseFunctionName(tk, functionName, returnType))
	{
		throw IntermediateError("Expected function name");
	}

	if (!tk.Current().IsKeyword(KW_BRACE_L))
	{
		throw IntermediateError("Expected '('");
	}
	tk.Forward();

	shared_ptr<AstList> arguments = ParseCommaSeparatedList(tk, &ParseVariable);

	if (!tk.Current().IsKeyword(KW_BRACE_R))
	{
		throw IntermediateError("Expected ')'");
	}
	tk.Forward();

	shared_ptr<AstList> code = ParseCode(tk);

	return make_shared<AstFunction>(functionName, returnType, arguments, code);
}

shared_ptr<IAst> ParseGlobal(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_GLOBAL))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	auto list = ParseCommaSeparatedList(tk, &ParseVariable);
	if (list->IsEmpty())
	{
		throw IntermediateError("Expected at least one variable declaration");
	}

	return make_shared<AstGlobal>(list);
}

// Const: 'const' Assignment [',' Assignment]
shared_ptr<IAst> ParseConstDecl(TokenStream &tk)
{
	if (!tk.Current().IsKeyword(KW_CONST))
	{
		return make_shared<AstError>();
	}
	tk.Forward();

	shared_ptr<AstList> cList = ParseCommaSeparatedList(tk, &ParseAssignment);

	if (cList->IsEmpty())
	{
		throw IntermediateError("Expected at least one constant declaration");
	}

	shared_ptr<AstList> declList(new AstList());
	
	for (auto decl : cList->GetElements())
	{
		auto assign = dynamic_cast<AstAssignmentExpression*>(decl.get());

		if (!assign)
		{
			throw InternalError("ParseConst: declaration is not AstAssignmentExpression");
		}

		if (!assign->GetExpression()->IsConstant())
		{
			throw IntermediateError("Value of a constant must be constant");
		}

		shared_ptr<ConstantBase> constant;

		switch (assign->GetVariable()->GetType().GetType())
		{
		case DATA_TYPE::TYPE_BOOL:
		{
			auto val = dynamic_cast<AstBoolConst*>(assign->GetExpression().get());
			constant = make_shared<BoolConst>(assign->GetVariable()->GetName(), val->GetValue());
			break;
		}
		case DATA_TYPE::TYPE_FLOAT:
		{
			auto val = dynamic_cast<AstFloatConst*>(assign->GetExpression().get());
			constant = make_shared<FloatConst>(assign->GetVariable()->GetName(), val->GetValue());
			break;
		}
		case DATA_TYPE::TYPE_INT:
		{
			auto val = dynamic_cast<AstIntConst*>(assign->GetExpression().get());
			constant = make_shared<IntConst>(assign->GetVariable()->GetName(), val->GetValue());
			break;
		}
		case DATA_TYPE::TYPE_STRING:
		{
			auto val = dynamic_cast<AstStringConst*>(assign->GetExpression().get());
			constant = make_shared<StringConst>(assign->GetVariable()->GetName(), val->GetValue());
			break;
		}
		default:
			throw IntermediateError("Constants must have atomic type");
		}

		declList->AddElement(make_shared<AstConstantDecl>(constant));
	}

	return declList;
}

// Definition: Const | Global | Function
shared_ptr<IAst> ParseDefinition(TokenStream &tk)
{
	auto result = ParseFunction(tk);
	if (!result->IsError())
	{
		return result;
	}

	result = ParseConstDecl(tk);
	if (!result->IsError())
	{
		return result;
	}

	return ParseGlobal(tk);
}

shared_ptr<IAst> ParseProgram(TokenStream &tk)
{
	return ParseSequentialList(tk, &ParseDefinition);
}

// TokenStream &tk - private in class Parser
// Different data types: DataType and AtomicDataType to prevent all the error checking

shared_ptr<IAst> Parse(const vector<Token> &tokens)
{
	TokenStream ts(tokens);

	try
	{
		shared_ptr<IAst> result = ParseProgram(ts);

		if (!ts.IsEOS())
		{
			throw CompileError("Syntax error", ts.Current());
		}
		ts.AssertStackIsEmpty();

		return result;
	}
	catch (const IntermediateError &ex)
	{
		throw CompileError(ex.what(), ts.Previous());
	}
}

shared_ptr<IAst> TestParse(PARSE_FUNCT pFunct, const string &code)
{
	Lexer lex;
	vector<Token> tokens = lex.ParseLine(code);
	TokenStream ts(tokens);
	auto result =  pFunct(ts);
	if (!result->IsError())
	{
		assert(ts.IsEOS());
	}
	return result;
}

// for compatability with old test
shared_ptr<IAst> ParseDictDecl(TokenStream &tk)
{
	return ParseDict(tk);
}

void TestVariablesAndConsts()
{
	// -=-=-=- ParseConst -=-=-=-
	{
		shared_ptr<IAst> res = TestParse(&ParseConst, "10");
		auto intConst = dynamic_cast<AstIntConst*>(res.get());
		assert(intConst->GetType() == DataType(ATOMIC_TYPE::TYPE_INT));
		assert(intConst->GetValue() == 10);
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseConst, "~45.9");
		auto floatConst = dynamic_cast<AstFloatConst*>(res.get());
		assert(floatConst->GetType() == DataType(ATOMIC_TYPE::TYPE_FLOAT));
		assert(floatConst->GetValue() == -45.9);
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseConst, "\"Test ` String\"");
		auto strConst = dynamic_cast<AstStringConst*>(res.get());
		assert(strConst->GetType() == DataType(ATOMIC_TYPE::TYPE_STRING));
		assert(strConst->GetValue() == "Test ` String");
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseConst, "Kappa");
		assert(res->IsError());
	}

	// -=-=-=- ParseSimpleVariable -=-=-=-
	{
		shared_ptr<IAst> res = TestParse(&ParseSimpleVariable, "age%");
		auto intVar = dynamic_cast<AstVariable*>(res.get());
		assert(intVar->GetName() == "age");
		assert(intVar->GetType() == DataType(ATOMIC_TYPE::TYPE_INT));
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseSimpleVariable, "flag!");
		auto boolVar = dynamic_cast<AstVariable*>(res.get());
		assert(boolVar->GetName() == "flag");
		assert(boolVar->GetType() == DataType(ATOMIC_TYPE::TYPE_BOOL));
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseSimpleVariable, "Keepo");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseSimpleVariable, "");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseSimpleVariable, "$");
		assert(res->IsError());
	}

	// -=-=-=- ParseArrayDecl -=-=-=-
	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "matrix # [ ] [ ]");
		auto arrayVar = dynamic_cast<AstVariable*>(res.get());
		assert(arrayVar->GetName() == "matrix");
		assert(arrayVar->GetType() == DataType(2, ATOMIC_TYPE::TYPE_FLOAT));
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "text$[]");
		auto arrayVar = dynamic_cast<AstVariable*>(res.get());
		assert(arrayVar->GetName() == "text");
		assert(arrayVar->GetType() == DataType(1, ATOMIC_TYPE::TYPE_STRING));
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "multi![][][][]");
		auto arrayVar = dynamic_cast<AstVariable*>(res.get());
		assert(arrayVar->GetName() == "multi");
		assert(arrayVar->GetType() == DataType(4, ATOMIC_TYPE::TYPE_BOOL));
	}

	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "lines$");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "users$[10]");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "typeless[][]");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseArrayDecl, "notadict##[][]");
		assert(res->IsError());
	}

	// -=-=-=- ParseDictDecl -=-=-=-
	{
		shared_ptr<IAst> res = TestParse(&ParseDictDecl, "user_id$%()");
		auto dictVar = dynamic_cast<AstVariable*>(res.get());
		assert(dictVar->GetName() == "user_id");
		assert(dictVar->GetType() == DataType(ATOMIC_TYPE::TYPE_STRING, ATOMIC_TYPE::TYPE_INT));
	}

	{
		shared_ptr<IAst> res = TestParse(&ParseDictDecl, "err$%");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseDictDecl, "test$()");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseDictDecl, "qwe!!)");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseDictDecl, "test%#(");
		assert(res->IsError());
	}

	// -=-=-=- ParseVariable -=-=-=-
	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "_%");
		auto variable = dynamic_cast<AstVariable*>(res.get());
		assert(variable->GetName() == "_");
		assert(variable->GetType() == DataType(ATOMIC_TYPE::TYPE_INT));
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "d_ct!#()");
		auto variable = dynamic_cast<AstVariable*>(res.get());
		assert(variable->GetName() == "d_ct");
		assert(variable->GetType() == DataType(ATOMIC_TYPE::TYPE_BOOL, ATOMIC_TYPE::TYPE_FLOAT));
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "arr#[][]");
		auto variable = dynamic_cast<AstVariable*>(res.get());
		assert(variable->GetName() == "arr");
		assert(variable->GetType() == DataType(2, ATOMIC_TYPE::TYPE_FLOAT));
	}

	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "\"abc\"");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "123.5");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "$");
		assert(res->IsError());
	}
	{
		shared_ptr<IAst> res = TestParse(&ParseVariable, "id");
		assert(res->IsError());
	}
}

shared_ptr<AstList> TestParseList(const string &code)
{
	Lexer lex;
	vector<Token> tokens = lex.ParseLine(code);
	TokenStream ts(tokens);
	auto result = ParseCommaSeparatedList(ts, &ParseConst);
	assert(ts.IsEOS());
	return result;
}

void TestParseList()
{
	{
		auto result = TestParseList("");
		assert(result->GetElements().size() == 0);
	}
	{
		auto result = TestParseList("10");
		assert(result->GetElements().size() == 1);
		auto elem = result->GetElements()[0];
		AstIntConst *ielem = dynamic_cast<AstIntConst*>(elem.get());
		assert(ielem);
		assert(ielem->GetValue() == 10);
	}
	{
		bool caught = false;
		try
		{
			auto result = TestParseList("10,");
		}
		catch (const IntermediateError &ex)
		{
			ex;
			caught = true;
		}
		assert(caught);
	}
	{
		auto result = TestParseList("10 , 15");
		assert(result->GetElements().size() == 2);

		AstIntConst *elem1 = dynamic_cast<AstIntConst*>(result->GetElements()[0].get());
		assert(elem1);
		assert(elem1->GetValue() == 10);

		AstIntConst *elem2 = dynamic_cast<AstIntConst*>(result->GetElements()[1].get());
		assert(elem2);
		assert(elem2->GetValue() == 15);
	}
	{
		Lexer lex;
		vector<Token> tokens = lex.ParseLine("(\"abc\", ~10, 135.55)");
		TokenStream ts(tokens);

		assert(ts.Current().IsKeyword(KW_BRACE_L));
		ts.Forward();

		auto list = ParseCommaSeparatedList(ts, &ParseConst);
		assert(list->GetElements().size() == 3);

		AstStringConst *elem1 = dynamic_cast<AstStringConst*>(list->GetElements()[0].get());
		assert(elem1);
		assert(elem1->GetValue() == "abc");

		AstIntConst *elem2 = dynamic_cast<AstIntConst*>(list->GetElements()[1].get());
		assert(elem2);
		assert(elem2->GetValue() == -10);

		AstFloatConst *elem3 = dynamic_cast<AstFloatConst*>(list->GetElements()[2].get());
		assert(elem3);
		assert(elem3->GetValue() == 135.55);

		assert(ts.Current().IsKeyword(KW_BRACE_R));
		ts.Forward();

		assert(ts.IsEOS());
	}
}

void ParserTests()
{
	TestVariablesAndConsts();
	TestParseList();
}
