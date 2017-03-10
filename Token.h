#pragma once
#include <string>
#include "InputStream.h"

// preprocessor magic

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define DEFINE_ENUM(NAME, FOREACH)				\
	enum NAME {									\
		FOREACH(GENERATE_ENUM)					\
	};											\
	static const char * NAME ## _STRING[] = {	\
		FOREACH(GENERATE_STRING)				\
	};

#define ENUM_KEYWORD(DECL)	\
	DECL(KW_CONST)			\
	DECL(KW_GLOBAL)			\
	DECL(KW_DEF)			\
	DECL(KW_RETURN)			\
	DECL(KW_IF)				\
	DECL(KW_ELSE)			\
	DECL(KW_DO)				\
	DECL(KW_WHILE)			\
	DECL(KW_FOR)			\
	DECL(KW_TO)				\
	DECL(KW_STEP)			\
	DECL(KW_ASSIGN)			\
	DECL(KW_COMMA)			\
	DECL(KW_BRACE_L)		\
	DECL(KW_BRACE_R)		\
	DECL(KW_BLOCK_L)		\
	DECL(KW_BLOCK_R)		\
	DECL(KW_INDEX_L)		\
	DECL(KW_INDEX_R)		\
	DECL(KW_UNARY_MINUS)	\
	DECL(KW_PLUS)			\
	DECL(KW_MINUS)			\
	DECL(KW_MUL)			\
	DECL(KW_DIV)			\
	DECL(KW_MOD)			\
	DECL(KW_OR)				\
	DECL(KW_AND)			\
	DECL(KW_NOT)			\
	DECL(KW_LESS)			\
	DECL(KW_GREATER)		\
	DECL(KW_EQUAL)			\
	DECL(KW_GR_EQ)			\
	DECL(KW_LE_EQ)			\
	DECL(KW_NOT_EQ)			\
	DECL(KW_STRING)			\
	DECL(KW_INT)			\
	DECL(KW_FLOAT)			\
	DECL(KW_BOOL)			\
	DECL(KW_NEW)			\
	DECL(KW_DELETE)			\
	DECL(KW_ARRAY)			\
	DECL(KW_DICT)

DEFINE_ENUM(KEYWORD, ENUM_KEYWORD)

#define ENUM_TOKEN_TYPE(DECL)	\
	DECL(TOKEN_ERROR)			\
	DECL(TOKEN_EOF)				\
	DECL(TOKEN_ID)				\
	DECL(TOKEN_STRING)			\
	DECL(TOKEN_DELIMETER)		\
	DECL(TOKEN_KEYWORD)			\
	DECL(TOKEN_INT)				\
	DECL(TOKEN_FLOAT)

DEFINE_ENUM(TOKEN_TYPE, ENUM_TOKEN_TYPE)

class Token
{
protected:
	// TOKEN_ERROR
	Token(TOKEN_TYPE type,								const InputStream &is, int offset);
	// TOKEN_FLOAT
	Token(TOKEN_TYPE type, double value,				const InputStream &is, int offset);
	// TOKEN_INT
	Token(TOKEN_TYPE type, int value,					const InputStream &is, int offset);
	// TOKEN_STRING, TOKEN_ID
	Token(TOKEN_TYPE type, const std::string &value,	const InputStream &is, int offset);
	// TOKEN_DELIMETER, TOKEN_KEYWORD
	Token(TOKEN_TYPE type, KEYWORD value,				const InputStream &is, int offset);

public:
	static Token TokenError(							const InputStream &is, int offset);
	static Token TokenEOF(								const InputStream &is, int offset);
	static Token TokenFloat(double value,				const InputStream &is, int offset);
	static Token TokenInt(int value,					const InputStream &is, int offset);
	static Token TokenString(const std::string &value,	const InputStream &is, int offset);
	static Token TokenID(const std::string &value,		const InputStream &is, int offset);
	static Token TokenDelimeter(KEYWORD value,			const InputStream &is, int offset);
	static Token TokenKeyword(KEYWORD value,			const InputStream &is, int offset);

	void Print() const;

	TOKEN_TYPE GetType() const;
	bool IsError() const;
	bool IsDelimeter() const;
	bool IsKeyword(KEYWORD kw) const;

	double GetValueF() const;
	int GetValueI() const;
	const std::string GetValueS() const;
	KEYWORD GetValueK() const;

	const char *GetLine() const;
	int GetLineNumber() const;
	int GetOffset() const;

private:
	TOKEN_TYPE m_type;

	union
	{
		double m_fvalue;
		int m_ivalue;
		KEYWORD m_kvalue;
	};
	std::string m_svalue;

	std::string m_line;
	int m_lineNumber, m_offset;
};
