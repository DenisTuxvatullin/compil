#pragma once
#include <string>
#include "DataType.h"

// Holds all information about a variable or constant
// (its name and data type)
class Variable
{
public:
	Variable(const std::string &varName, const DataType &varType);
	virtual ~Variable();

	const std::string &GetName() const;
	const DataType &GetType() const;

private:
	const std::string m_name;
	const DataType m_type;
};
