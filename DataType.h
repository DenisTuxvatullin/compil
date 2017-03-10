#pragma once
#include <string>

// Non composite data type
enum class ATOMIC_TYPE
{
	TYPE_INT = 0,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_VOID		// Used only as a return type for procedures
};

// All data types, including
// arrays and dictionaries
enum class DATA_TYPE
{
	TYPE_INT = 0,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_VOID,

	TYPE_ARRAY,
	TYPE_DICT
};

// Holds information about data type
// (variable data type, expression result or function return value)
class DataType
{
public:
	// Atomic data type
	DataType(ATOMIC_TYPE simpleType);
	// Array
	DataType(int dimension, ATOMIC_TYPE arrayType);
	// Dictionary
	DataType(ATOMIC_TYPE keyType, ATOMIC_TYPE valueType);

	bool IsNumeric() const;
	bool IsAtomic() const;
	bool IsNotVoid() const;

	std::string ToIL() const;
	std::string ToString() const;

	DATA_TYPE GetType() const;
	int GetArrayDimension() const;
	ATOMIC_TYPE GetArrayType() const;
	ATOMIC_TYPE GetDictKeyType() const;
	ATOMIC_TYPE GetDictValueType() const;

	bool operator ==(const DataType &dt) const;
	bool operator !=(const DataType &dt) const;

private:
	DATA_TYPE m_type;

	union
	{
		// array
		int m_array_dimension;
		// dict
		ATOMIC_TYPE m_dict_key_type;
	};
	union
	{
		// array
		ATOMIC_TYPE m_array_type;
		// dict
		ATOMIC_TYPE m_dict_value_type;
	};

	static std::string GetSimpleType(ATOMIC_TYPE dataType);
	static std::string AtomicTypeToIL(ATOMIC_TYPE dataType);
};
