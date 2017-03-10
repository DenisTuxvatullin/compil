#include "DataType.h"
#include "Exception.h"
#include <assert.h>

using namespace std;

DataType::DataType(ATOMIC_TYPE simpleType)
	: m_type((DATA_TYPE)simpleType)
{
}

DataType::DataType(int dimension, ATOMIC_TYPE arrayType)
		: m_array_dimension(dimension)
		, m_array_type(arrayType)
{
	m_type = DATA_TYPE::TYPE_ARRAY;
	if (m_array_dimension < 1 || m_array_dimension > 4)
	{
		throw IntermediateError("Invalid array dimension");
	}
}

DataType::DataType(ATOMIC_TYPE keyType, ATOMIC_TYPE valueType)
	: m_dict_key_type(keyType)
	, m_dict_value_type(valueType)
{
	m_type = DATA_TYPE::TYPE_DICT;
}

bool DataType::IsNumeric() const
{
	return GetType() == DATA_TYPE::TYPE_FLOAT ||
		GetType() == DATA_TYPE::TYPE_INT;
}

bool DataType::IsAtomic() const
{
	return m_type == DATA_TYPE::TYPE_INT || m_type == DATA_TYPE::TYPE_FLOAT || 
		m_type == DATA_TYPE::TYPE_STRING || m_type == DATA_TYPE::TYPE_BOOL;
}

bool DataType::IsNotVoid() const
{
	return GetType() != DATA_TYPE::TYPE_VOID;
}

DATA_TYPE DataType::GetType() const
{
	return m_type;
}

int DataType::GetArrayDimension() const
{
	assert(m_type == DATA_TYPE::TYPE_ARRAY);
	return m_array_dimension;
}

ATOMIC_TYPE DataType::GetArrayType() const
{
	assert(m_type == DATA_TYPE::TYPE_ARRAY);
	return m_array_type;
}

ATOMIC_TYPE DataType::GetDictKeyType() const
{
	assert(m_type == DATA_TYPE::TYPE_DICT);
	return m_dict_key_type;
}

ATOMIC_TYPE DataType::GetDictValueType() const
{
	assert(m_type == DATA_TYPE::TYPE_DICT);
	return m_dict_value_type;
}

string DataType::AtomicTypeToIL(ATOMIC_TYPE dataType)
{
	switch (dataType)
	{
	case ATOMIC_TYPE::TYPE_BOOL:
		return "bool";
	case ATOMIC_TYPE::TYPE_FLOAT:
		return "float64";
	case ATOMIC_TYPE::TYPE_INT:
		return "int32";
	case ATOMIC_TYPE::TYPE_STRING:
		return "string";
	case ATOMIC_TYPE::TYPE_VOID:
		return "void";
	default:
		throw InternalError("AtomicTypeToIL: invalid type");
	}
}

string DataType::ToIL() const
{
	switch (GetType())
	{
	//case ATOMIC_TYPE::TYPE_BOOL:
	//case ATOMIC_TYPE::TYPE_FLOAT:
	//case ATOMIC_TYPE::TYPE_INT:
	//case ATOMIC_TYPE::TYPE_STRING:
	//case ATOMIC_TYPE::TYPE_VOID:
		return AtomicTypeToIL((ATOMIC_TYPE)GetType());
	case DATA_TYPE::TYPE_ARRAY:
		{
		string result(AtomicTypeToIL(GetArrayType()));

		result += "[";
		if (GetArrayDimension() > 1)
		{
			for (int i = 0; i < GetArrayDimension(); i++)
			{
				result += "0...";
				if (i < GetArrayDimension() - 1)
				{
					result += ",";
				}
			}
		}
		result += "]";

		return result;
		}
	case DATA_TYPE::TYPE_DICT:
		return "class [mscorlib]System.Collections.Generic.Dictionary`2<" + 
			AtomicTypeToIL(GetDictKeyType()) + "," + AtomicTypeToIL(GetDictValueType()) + ">";
	default:
		throw InternalError("Not implemented");
	}
}

string DataType::GetSimpleType(ATOMIC_TYPE dataType)
{
	switch (dataType)
	{
	case ATOMIC_TYPE::TYPE_BOOL:
		return "!";
	case ATOMIC_TYPE::TYPE_FLOAT:
		return "#";
	case ATOMIC_TYPE::TYPE_INT:
		return "%";
	case ATOMIC_TYPE::TYPE_STRING:
		return "$";
	default:
		throw InternalError("GetSimpleType: invalid DataType");
	}
}

string DataType::ToString() const
{
	switch (GetType())
	{
	case DATA_TYPE::TYPE_BOOL:
		return "bool!";
	case DATA_TYPE::TYPE_FLOAT:
		return "float#";
	case DATA_TYPE::TYPE_INT:
		return "int%";
	case DATA_TYPE::TYPE_STRING:
		return "string$";
	case DATA_TYPE::TYPE_VOID:
		return "void";
	case DATA_TYPE::TYPE_ARRAY:
		{
			string result = "array" + GetSimpleType(GetArrayType());
			for (int i = 0; i < GetArrayDimension(); i++)
			{
				result += "[]";
			}
			return result;
		}
	case DATA_TYPE::TYPE_DICT:
		return "dict" + GetSimpleType(GetDictKeyType()) + GetSimpleType(GetDictValueType()) + "()";
	default:
		throw InternalError("DataType::ToString: invalid DataType");
	}
}

bool DataType::operator ==(const DataType &dt) const
{
	if (GetType() == dt.GetType())
	{
		if (IsAtomic() || GetType() == DATA_TYPE::TYPE_VOID)
		{
			return true;
		}
		else
		{
			if (GetType() == DATA_TYPE::TYPE_ARRAY)
			{
				return GetArrayDimension() == dt.GetArrayDimension() &&
					GetArrayType() == dt.GetArrayType();
			}
			else if (GetType() == DATA_TYPE::TYPE_DICT)
			{
				return GetDictKeyType() == dt.GetDictKeyType() &&
					GetDictValueType() == dt.GetDictValueType();
			}
			else
			{
				throw InternalError("DataType::operator ==: impossible");
			}
		}
	}
	else
	{
		return false;
	}
}

bool DataType::operator !=(const DataType &dt) const
{
	return !operator==(dt);
}
