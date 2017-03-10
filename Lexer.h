#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "InputStream.h"
#include "TokenStream.h"
#include "Token.h"

class Lexer
{
public:
	// Lexer(InputStream &is);
	Lexer();

	// should return token stream instead of vector of tokens
	// token stream: nextToken, reset, forward, token
	// returns TokenEOF when there's noting more

	// ��������� �� ������ ���� � ������ fileName
	// - � ������ ������ ���������� ������ ����������� � ������,
	// ���������� �������� ����� �������� ������� ������� GetTokens()
	// - � ������ ������ ������������� ���������� ���� CompileError
	//void Parse(const char *fileName);

	// �������� ����������� ������ ����� ������ ����
	// - ���������� ���������� ������ � ��������
	// - � ������� ����� ������ ��������������� ������ lineNumber
	// - � ������ ������ ����������� ���������� CompileError
	// - �� ��������� ������ ��������� � ������!
	std::vector<Token> ParseLine(const std::string &line, int lineNumber = 0) const;

	TokenStream Parse(const std::string &line, int lineNumber = 0) const;

	// ���������� ��������� ������������ ������� - ������ �������
	const std::vector<Token> *GetTokens() const;

private:
	// make InputStream &is a private variable
	// so we won't have to pass it to all those static
	// functions manually
	static std::string StripComments(const std::string &line);
	static std::string GetUInt(InputStream &is);
	static std::string GetInt(InputStream &is);
	static Token ParseFloat(InputStream &is);
	static Token ParseNumber(InputStream &is);
	static Token ParseString(InputStream &is);
	static Token ParseDelimeter(InputStream &is);
	static Token ParseID(InputStream &is);
	static Token ParseToken(InputStream &is);
};
