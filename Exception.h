#pragma once
#include <stdexcept>
#include <string>

class Token;

// == ������ ����������
// - ������������� ����� � �������� ���� �������������� �����������,
// �������������� ��� ������������� ������
// - �������� � ������ ������ � ���������� ����������
class CompileError : public std::runtime_error
{
public:
	CompileError(const char *description, const Token &token);

private:
	static std::string PrepareMessage(const char *description, const Token &token);
};

// == ���������� ������
// - ������������� ����� ��������� ���-��, ��� �� ������ ����������� � ��������
// - �������� � ����������� ���������� ������ �����������
class InternalError : public std::runtime_error
{
public:
	InternalError(const std::string &description);
};

// == ������������� ������
// - ������������� ����� ����� ��������� CompileError, ������ ��������������
// ����������, ����������� ��� ����� (����� ������, ������� � ������ � �.�.)
// � ������ ������ ����������
// - � ����������� �������� ���������� ������� ���� ����������������� ��� CompileError
class IntermediateError : public std::runtime_error
{
public:
	IntermediateError(const std::string &description);
};

/*
TODO:
move local symbol table to function, arguments in local symbol table
IL code writer (with stack checks)
Tests (lexer, parse)

parsing simple expressions::
var declarations: a$, array[][]
var values: b%, array[1][2]
constants: number, fpnumber, string
const declarations: const name$ = "boo";

in lexer, make InputStream &is a class variable, and Parse... functions class members

*/
