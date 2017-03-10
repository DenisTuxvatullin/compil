#pragma once
#include <vector>
#include <stack>
#include "Token.h"

class TokenStream
{
public:
	TokenStream();
	TokenStream(const std::vector<Token> &tokens);

	const Token Current() const;
	const Token Previous() const;

	void Forward();

	void PushPosition();
	void PopPosition();
	void RestorePosition();

	bool IsEOS() const;
	void AssertStackIsEmpty() const;

private:
	const std::vector<Token> m_tokens;
	size_t m_position;
	std::stack<size_t> m_positionStack;
};
