#pragma once
#include <string>
#include <stack>

#define EOS		(-1)

class InputStream
{
public:
	// ������� ������� ����� �� ������ ������
	InputStream(const std::string &text, int lineNumber);

	// ���������� ������� � ������
	size_t Tell() const;

	// ������������� �������
	// ���������� ������� ������� ������� ����������
	size_t Seek(size_t pos);

	// ���������� ������� ������ �� shift ��������
	// ���������� ���-�� �������� �� ������� ������� ����������
	size_t Forward(size_t shift = 1);

	// ���������� �������, ��������� � �.�.
	// ���������� ���-�� ����������� ��������
	size_t SkipSpaces();

	// ���������� true ���� ��������� ����� ������
	bool IsEOS() const;

	// ���������� length �������� � ������� �������
	std::string Get(size_t length) const;

	// ���������� true ���� ����� � ������� ������� InputStream
	// �������� ��������� � ������� � ��������� text
	bool Match(const char *text) const;

	// ���� ����� ���������, �� ������� �� ������� �����
	// ���������� �������
	bool InputStream::MatchF(const char *text);

	// ���������� ���������� �����
	const char *GetRemainingText() const;

	// ���� � ������� �������
	int GetChar() const;

	// ��������� �������
	void PushPosition();
	// �������� �� ����� ��������� ����������� �������, �� �������������� �
	size_t PopPosition();
	// ������������ ��������� ����������� �������
	size_t RestorePosition();

	// ���������� ����� ������ � �������� �����
	int GetLineNumber() const;

	// ���������� ������ ���������� ������,
	// �� ������ ������� ��� ������ �����
	const char *GetLine() const;

private:
	const std::string m_data;

	// ������� �������
	std::string::const_iterator m_pos;

	// ����, � ������� �������� ���������� �������
	std::stack<size_t> m_pos_stack;

	// ����� ������ � �������� �����
	int m_lineNumber;
};
