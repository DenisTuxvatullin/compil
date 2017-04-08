#include <assert.h>
#include <iostream>
#include "Lexer.h"
#include "Exception.h"

using namespace std;

// These should be in Lexer's privates, but I get
// an unresolved external if I put them there
std::map<std::string, KEYWORD> m_tokens;
std::set<std::string> m_delim;

string Lexer::StripComments(const string &line)
{
	bool isInsideString = false;

	for (unsigned int i = 0; i < line.size(); i++)
	{
		char ch = line[i];

		if (ch == '"')
		{
			isInsideString = !isInsideString;
		}
		else if (ch == '`' && !isInsideString)
		{
			return line.substr(0, i);
		}
	}

	return line;
}

string Lexer::GetUInt(InputStream &is)
{
	string result;

	int ch = is.GetChar();
	while (ch >= '0' && ch <= '9')
	{
		result += ch;
		is.Forward();
		ch = is.GetChar();
	}

	return result;
}

string Lexer::GetInt(InputStream &is)
{
	string result;

	is.PushPosition();
	if (is.GetChar() == '~')
	{
		result += "-";
		is.Forward();
	}

	result += GetUInt(is);

	if (result == "-" || result.size() == 0)
	{
		is.RestorePosition();
		return "";
	}

	is.PopPosition();
	return result;
}

Token Lexer::ParseFloat(InputStream &is)
{
	is.PushPosition();
	string numAccum(GetInt(is));

	if (numAccum.size() == 0)
	{
		is.RestorePosition();
		return Token::TokenError(is, is.Tell());
	}

	if (is.GetChar() != '.')
	{
		is.RestorePosition();
		return Token::TokenError(is, is.Tell());
	}

	numAccum += '.';
	is.Forward();

	string rightPart = GetUInt(is);
	if (rightPart.size() == 0)
	{
		is.RestorePosition();
		return Token::TokenError(is, is.Tell());
	}

	numAccum += rightPart;

	return Token::TokenFloat(atof(numAccum.c_str()), is, is.PopPosition());
}

Token Lexer::ParseNumber(InputStream &is)
{
	// TODO: proper overflow check

	is.PushPosition();
	string number(GetInt(is));

	if (number.size() == 0)
	{
		is.RestorePosition();
		return Token::TokenError(is, is.Tell());
	}

	if (number.size() > 10)
	{
		throw CompileError("Number too long", Token::TokenError(is, is.PopPosition()));
	}

	return Token::TokenInt(atoi(number.c_str()), is, is.PopPosition());
}

Token Lexer::ParseString(InputStream &is)
{
	if (is.GetChar() == '"')
	{
		string stringValue;

		is.PushPosition();
		while (true)
		{
			is.Forward();

			int ch = is.GetChar();
			switch (ch)
			{
			case '"':
				is.Forward();
				return Token::TokenString(stringValue, is, is.PopPosition());
			case EOS:
				throw CompileError("Unclosed double quote", Token::TokenError(is, is.PopPosition()));
			default:
				stringValue += ch;
			}
		}
	}

	return Token::TokenError(is, is.Tell());
}

Token Lexer::ParseDelimeter(InputStream &is)
{
	string delim;
	if (m_delim.find(delim = is.Get(2)) != m_delim.end() ||
		m_delim.find(delim = is.Get(1)) != m_delim.end())
	{
		is.Forward(delim.size());
		return Token::TokenDelimeter(m_tokens[delim], is, is.Tell() - delim.size());
	}

	return Token::TokenError(is, is.Tell());
}

Token Lexer::ParseID(InputStream &is)
{
	int startPos = is.Tell();

	int ch = tolower(is.GetChar());
	if ((ch >= 'a' && ch <= 'z') || ch == '_')
	{
		string idName;

		do
		{
			idName += ch;
			is.Forward();
			ch = tolower(is.GetChar());
		} while ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_');
		
		if (m_tokens.find(idName) != m_tokens.end())
		{
			return Token::TokenKeyword(m_tokens[idName], is, startPos);
		}
		else
		{
			return Token::TokenID(idName, is, startPos);
		}
	}

	return Token::TokenError(is, is.Tell());
}

typedef Token (*PARSE_FN_PTR)(InputStream &is);

Token Lexer::ParseToken(InputStream &is)
{
	PARSE_FN_PTR parseFunctions[] = {
		ParseFloat,
		ParseNumber,
		ParseString,
		ParseDelimeter,
		ParseID
	};

	for (auto fn : parseFunctions)
	{
		Token result = fn(is);
		if (!result.IsError())
		{
			return result;
		}
	}

	return Token::TokenError(is, is.Tell());
}

vector<Token> Lexer::ParseLine(const string &line, int lineNumber) const
{
	InputStream is = InputStream(StripComments(line), lineNumber);
	vector<Token> result;

	bool whitespaceSkipped = is.SkipSpaces() != 0;
	while (!is.IsEOS())
	{
		Token token = ParseToken(is);
		if (token.IsError())
		{
			throw CompileError("Lexical error", token);
		}
		result.push_back(token);

		if (result.size() >= 2)
		{
			if (!result[result.size() - 2].IsDelimeter() &&
				!result[result.size() - 1].IsDelimeter())
			{
				if (!whitespaceSkipped)
				{
					// only TOKEN_DELIMETER can adjoin other tokens
					// without being separated by whitespace

					// this is to prevent situations when one
					// erroneous token is being parsed as two
					// Example: '5f6' being parsed as TOKEN_NUMBER<5> TOKEN_ID<f6>
					throw CompileError("Lexical error", token);
				}
			}
		}

		whitespaceSkipped = is.SkipSpaces() != 0;
	}

	return result;
}

TokenStream Lexer::Parse(const std::string &line, int lineNumber) const
{
	return TokenStream(ParseLine(line, lineNumber));
}

Lexer::Lexer()
{
	if (m_tokens.size() == 0)
	{
		m_tokens["const"] = KW_CONST;
		m_tokens["global"] = KW_GLOBAL;
		m_tokens["def"] = KW_DEF;

		m_tokens["return"] = KW_RETURN;
		m_tokens["if"] = KW_IF;
		m_tokens["else"] = KW_ELSE;
		m_tokens["do"] = KW_DO;
		m_tokens["while"] = KW_WHILE;
		m_tokens["for"] = KW_FOR;
		m_tokens["to"] = KW_TO;
		m_tokens["step"] = KW_STEP;
		
		m_tokens["="] = KW_ASSIGN;
		m_tokens[","] = KW_COMMA;

		m_tokens["("] = KW_BRACE_L;
		m_tokens[")"] = KW_BRACE_R;
		m_tokens["{"] = KW_BLOCK_L;
		m_tokens["}"] = KW_BLOCK_R;
		m_tokens["["] = KW_INDEX_L;
		m_tokens["]"] = KW_INDEX_R;

		m_tokens["~"] = KW_UNARY_MINUS;
		m_tokens["+"] = KW_PLUS;
		m_tokens["-"] = KW_MINUS;
		m_tokens["*"] = KW_MUL;
		m_tokens["/"] = KW_DIV;
		m_tokens["mod"] = KW_MOD;

		m_tokens["or"] = KW_OR;
		m_tokens["and"] = KW_AND;
		m_tokens["not"] = KW_NOT;

		m_tokens["<"] = KW_LESS;
		m_tokens[">"] = KW_GREATER;
		m_tokens["=="] = KW_EQUAL;
		m_tokens[">="] = KW_GR_EQ;
		m_tokens["<="] = KW_LE_EQ;
		m_tokens["<>"] = KW_NOT_EQ;

		m_tokens["$"] = KW_STRING;
		m_tokens["%"] = KW_INT;
		m_tokens["#"] = KW_FLOAT;
		m_tokens["!"] = KW_BOOL;

		m_tokens["new"] = KW_NEW;
		m_tokens["delete"] = KW_DELETE;
		m_tokens["array"] = KW_ARRAY;
		m_tokens["dict"] = KW_DICT;
	}

	if (m_delim.size() == 0)
	{
		m_delim.insert("==");
		m_delim.insert(">=");
		m_delim.insert("<=");
		m_delim.insert("<>");
		m_delim.insert("<");
		m_delim.insert(">");

		m_delim.insert("$");
		m_delim.insert("%");
		m_delim.insert("#");
		m_delim.insert("!");

		m_delim.insert(",");

		m_delim.insert("(");
		m_delim.insert(")");
		m_delim.insert("{");
		m_delim.insert("}");
		m_delim.insert("[");
		m_delim.insert("]");

		m_delim.insert("=");
		m_delim.insert("+");
		m_delim.insert("-");
		m_delim.insert("*");
		m_delim.insert("/");
		m_delim.insert("~");
	}
}
