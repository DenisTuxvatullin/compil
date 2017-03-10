#pragma once
#include "DataType.h"
#include "Variable.h"
#include <string>

// Specific constant classes are inherited from this
class ConstantBase : public Variable
{
public:
	ConstantBase(const std::string &varName, const DataType &varType)
		: Variable(varName, varType)
	{
	}

	virtual ~ConstantBase()
	{
	}
};

// Holds information about a constant
template<class CTYPE, ATOMIC_TYPE ATYPE> class Constant : public ConstantBase
{
public:
	Constant(const std::string &constName, CTYPE constValue)
		: ConstantBase(constName, DataType(ATYPE))
		, m_value(constValue)
	{
	}

	CTYPE GetValue() const
	{
		return m_value;
	}

private:
	CTYPE m_value;
};

typedef Constant<double, ATOMIC_TYPE::TYPE_FLOAT> FloatConst;
typedef Constant<std::string, ATOMIC_TYPE::TYPE_STRING> StringConst;
typedef Constant<int, ATOMIC_TYPE::TYPE_INT> IntConst;
typedef Constant<bool, ATOMIC_TYPE::TYPE_BOOL> BoolConst;
