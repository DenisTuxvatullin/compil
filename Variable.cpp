#include "Variable.h"

using namespace std;

Variable::Variable(const string &varName, const DataType &varType)
	: m_name(varName)
	, m_type(varType)
{
}

Variable::~Variable()
{
}

const string &Variable::GetName() const
{
	return m_name;
}

const DataType &Variable::GetType() const
{
	return m_type;
}
