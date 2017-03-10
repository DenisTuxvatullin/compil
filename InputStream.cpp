#include "InputStream.h"
#include <string.h>

using namespace std;

InputStream::InputStream(const string &text, int lineNumber)
	: m_data(text)
	, m_pos(m_data.begin())
	, m_lineNumber(lineNumber)
{
}

size_t InputStream::Tell() const
{
	return m_pos - m_data.begin();
}

size_t InputStream::Seek(size_t pos)
{
	if (pos < m_data.size())
	{
		m_pos = m_data.begin() + pos;
	} else {
		m_pos = m_data.end();
	}

	return Tell();
}

size_t InputStream::Forward(size_t shift)
{
	size_t old_pos = Tell();
	return Seek(old_pos + shift) - old_pos;
}

size_t InputStream::SkipSpaces()
{
	size_t skipped = 0;

	for (auto skip_iter = m_pos; skip_iter != m_data.end(); skip_iter++)
	{
		if (isspace(*skip_iter))
		{
			skipped++;
		} else {
			break;
		}
	}
	
	Forward(skipped);
	return skipped;
}

bool InputStream::IsEOS() const
{
	return m_pos == m_data.end();
}

string InputStream::Get(size_t length) const
{
	return m_data.substr(Tell(), length);
}

bool InputStream::Match(const char *text) const
{
	if (*text == '\0')
		return true;

	for (auto ch = m_pos; ch != m_data.end(); ch++, text++)
	{
		if (*text == '\0')
			return true;

		if (*text != *ch)
			return false;
	}

	return *text == '\0';
}

bool InputStream::MatchF(const char *text)
{
	if (Match(text))
	{
		Forward(strlen(text));
		return true;
	}

	return false;
}

const char *InputStream::GetRemainingText() const
{
	return m_data.c_str() + Tell();
}

int InputStream::GetChar() const
{
	if (IsEOS())
	{
		return EOS;
	}
	else
	{
		return *m_pos;
	}
}

void InputStream::PushPosition()
{
	m_pos_stack.push(Tell());
}

size_t InputStream::PopPosition()
{
	size_t newPos = m_pos_stack.top();
	m_pos_stack.pop();
	return newPos;
}

size_t InputStream::RestorePosition()
{
	size_t newPos = m_pos_stack.top();
	Seek(newPos);
	m_pos_stack.pop();
	return newPos;
}

int InputStream::GetLineNumber() const
{
	return m_lineNumber;
}

const char *InputStream::GetLine() const
{
	return m_data.c_str();
}
