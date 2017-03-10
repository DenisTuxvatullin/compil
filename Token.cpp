#include <iostream>
#include <assert.h>
#include "Token.h"

using namespace std;

Token::Token(TOKEN_TYPE type, const InputStream &is, int offset)
	: m_type(type)

	, m_line(is.GetLine())
	, m_lineNumber(is.GetLineNumber())
	, m_offset(offset)
{
	assert(type == TOKEN_ERROR || type == TOKEN_EOF);
}

Token::Token(TOKEN_TYPE type, double value, const InputStream &is, int offset)
	: m_type(type)
	, m_fvalue(value)

	, m_line(is.GetLine())
	, m_lineNumber(is.GetLineNumber())
	, m_offset(offset)
{
	assert(type == TOKEN_FLOAT);
}

Token::Token(TOKEN_TYPE type, int value, const InputStream &is, int offset)
	: m_type(type)
	, m_ivalue(value)

	, m_line(is.GetLine())
	, m_lineNumber(is.GetLineNumber())
	, m_offset(offset)
{
	assert(type == TOKEN_INT);
}

Token::Token(TOKEN_TYPE type, const string &value, const InputStream &is, int offset)
	: m_type(type)
	, m_svalue(value)

	, m_line(is.GetLine())
	, m_lineNumber(is.GetLineNumber())
	, m_offset(offset)
{
	assert(type == TOKEN_STRING || type == TOKEN_ID);
}

Token::Token(TOKEN_TYPE type, KEYWORD value, const InputStream &is, int offset)
	: m_type(type)
	, m_kvalue(value)

	, m_line(is.GetLine())
	, m_lineNumber(is.GetLineNumber())
	, m_offset(offset)
{
	assert(type == TOKEN_DELIMETER || type == TOKEN_KEYWORD);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

Token Token::TokenError(const InputStream &is, int offset)
{
	return Token(TOKEN_ERROR, is, offset);
}

Token Token::TokenEOF(const InputStream &is, int offset)
{
	return Token(TOKEN_EOF, is, offset);
}

Token Token::TokenFloat(double value, const InputStream &is, int offset)
{
	return Token(TOKEN_FLOAT, value, is, offset);
}

Token Token::TokenInt(int value, const InputStream &is, int offset)
{
	return Token(TOKEN_INT, value, is, offset);
}

Token Token::TokenString(const std::string &value, const InputStream &is, int offset)
{
	return Token(TOKEN_STRING, value, is, offset);
}

Token Token::TokenID(const std::string &value, const InputStream &is, int offset)
{
	return Token(TOKEN_ID, value, is, offset);
}

Token Token::TokenDelimeter(KEYWORD value, const InputStream &is, int offset)
{
	return Token(TOKEN_DELIMETER, value, is, offset);
}

Token Token::TokenKeyword(KEYWORD value, const InputStream &is, int offset)
{
	return Token(TOKEN_KEYWORD, value, is, offset);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

TOKEN_TYPE Token::GetType() const
{
	return m_type;
}

void Token::Print() const
{
	cout << TOKEN_TYPE_STRING[GetType()];

	switch (GetType())
	{
	case TOKEN_ID:
	case TOKEN_STRING:
		cout << "<" << GetValueS() << ">";
		break;

	case TOKEN_KEYWORD:
	case TOKEN_DELIMETER:
		cout << "<" << KEYWORD_STRING[GetValueK()] << ">";
		break;

	case TOKEN_FLOAT:
		cout << "<" << GetValueF() << ">";
		break;
	
	case TOKEN_INT:
		cout << "<" << GetValueI() << ">";
		break;
	}

	cout << " line " << GetLineNumber(); //<< ":" << GetOffset() << " in line '" << GetLine() << "'";
}

bool Token::IsError() const
{
	return GetType() == TOKEN_ERROR;
}

bool Token::IsDelimeter() const
{
	return GetType() == TOKEN_DELIMETER;
}

bool Token::IsKeyword(KEYWORD kw) const
{
	return (GetType() == TOKEN_DELIMETER || GetType() == TOKEN_KEYWORD)
		&& (GetValueK() == kw);
}

double Token::GetValueF() const
{
	assert(GetType() == TOKEN_FLOAT);
	return m_fvalue;
}

int Token::GetValueI() const
{
	assert(GetType() == TOKEN_INT);
	return m_ivalue;
}

const string Token::GetValueS() const
{
	assert(GetType() == TOKEN_STRING || GetType() == TOKEN_ID);
	return m_svalue;
}

KEYWORD Token::GetValueK() const
{
	assert(GetType() == TOKEN_KEYWORD || GetType() == TOKEN_DELIMETER);
	return m_kvalue;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

const char *Token::GetLine() const
{
	return m_line.c_str();
}

int Token::GetLineNumber() const
{
	return m_lineNumber;
}

int Token::GetOffset() const
{
	return m_offset;
}
