#pragma once
#include <vector>
#include <memory>
#include "Token.h"
#include "AstFwd.h"

void InitOperatorMap();
std::shared_ptr<IAst> Parse(const std::vector<Token> &tokens);
void ParserTests();

std::shared_ptr<IAst> ParseNewRDP(TokenStream &tk);
std::shared_ptr<IAst> ParseVariableRDP(TokenStream &tk);

/*
class Parser
{
public:
	Parser();

	shared_ptr<IAst> Parse(const vector<Token> &tokens);

private:
	void InitOperatorMap();
};
*/