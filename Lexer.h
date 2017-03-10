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

	// Разбивает на токены файл с именем fileName
	// - В случае успеха полученные токены добавляются в вектор,
	// содержимое которого можно получить вызовом функции GetTokens()
	// - В случае ошибки выбрасывается исключение типа CompileError
	//void Parse(const char *fileName);

	// Проводит лексический анализ одной строки кода
	// - Возвращает полученный массив с токенами
	// - У токенов номер строки устанавливается равным lineNumber
	// - В случае ошибки выбрасывает исключение CompileError
	// - По окончанию тестов перенести в приват!
	std::vector<Token> ParseLine(const std::string &line, int lineNumber = 0) const;

	TokenStream Parse(const std::string &line, int lineNumber = 0) const;

	// Возвращает результат лексического анализа - массив токенов
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
