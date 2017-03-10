#include "TokenStream.h"
#include "Exception.h"

using namespace std;

TokenStream::TokenStream(const vector<Token> &tokens)
	: m_tokens(tokens)
	, m_position(0)
{

}
	
const Token TokenStream::Current() const
{
	if (m_position >= m_tokens.size())
	{
		if (m_tokens.size())
		{
			const Token &tk = m_tokens[m_tokens.size() - 1];
			InputStream is(tk.GetLine(), tk.GetLineNumber());
			is.Forward(strlen(is.GetLine()));
			return Token::TokenEOF(is, is.Tell());
		}
		else
		{
			return Token::TokenEOF(InputStream("", 1), 0);
		}
	}
	else
	{
		return m_tokens[m_position];
	}
}

const Token TokenStream::Previous() const
{
	if (m_position == 0)
	{
		return Current();
	}
	else
	{
		return m_tokens[m_position - 1];
	}
}

void TokenStream::Forward()
{
	if (m_position < m_tokens.size())
	{
		m_position++;
	}
}

void TokenStream::PushPosition()
{
	m_positionStack.push(m_position);
}

void TokenStream::PopPosition()
{
	m_positionStack.pop();
}

void TokenStream::RestorePosition()
{
	m_position = m_positionStack.top();
	PopPosition();
}

bool TokenStream::IsEOS() const
{
	  return m_position >= m_tokens.size();
}

void TokenStream::AssertStackIsEmpty() const
{
	if (!m_positionStack.empty())
	{
		throw IntermediateError("TokenStream::AssertStackIsEmpty(): position stack isn't empty");
	}
}
