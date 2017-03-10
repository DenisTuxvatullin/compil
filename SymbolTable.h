#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "Exception.h"

// == Таблица символов
// - По возможностям - практически аналог map'а
// - Есть проверка на повторное объявление символа
// - Есть возможность получить для символа его уникальный индекс
// - Предпологает, что у T есть метод const string &GetName()
template<class T> class SymbolTable
{
public:
	SymbolTable()
	{
	}

	bool IsDefined(const std::string &name) const
	{
		return m_indexTable.find(name) != m_indexTable.end();
	}

	void Define(std::shared_ptr<T> symb)
	{
		const std::string &name = symb->GetName();

		if (IsDefined(name))
		{
			throw IntermediateError("Symbol '" + name + "' was already defined");
		}

		m_symbols.push_back(symb);
		m_indexTable[name] = m_symbols.size() - 1;
	}

	unsigned int GetIndex(const std::string &name) const
	{
		AssertSymbolIsDefined(name);
		return m_indexTable.at(name);
	}

	std::shared_ptr<T> Get(const std::string &name) const
	{
		return m_symbols[GetIndex(name)];
	}

	std::shared_ptr<T> operator[](unsigned int index) const
	{
		return m_symbols[index];
	}

	unsigned int Count() const
	{
		return m_symbols.size();
	}

private:
	std::map<std::string, unsigned> m_indexTable;
	std::vector<std::shared_ptr<T>> m_symbols;

	void AssertSymbolIsDefined(const std::string &name) const
	{
		if (!IsDefined(name))
		{
			throw IntermediateError("Symbol '" + name + "' is undefined");
		}
	}
};
