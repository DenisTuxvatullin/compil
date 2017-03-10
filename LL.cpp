#include <iostream>
#include <map>
#include <stack>
#include "LL.h"
#include "Token.h"
#include "Parser.h"
#include "IAst.h"

using namespace std;


int ParsingTable[7][13] = 
{
	//"kw_new","kw_array","id","kw_dict","(",")","!","%","#","$","[","]","$"
	{2,  14, 14, 14, 14, 14, 14, 14, 14, 13, 14, 14, 13},
	{14,  3, 14, 14, 14, 14, 14, 14, 14, 13, 14, 14, 13},
	{14, 14, 14,  4, 14, 14, 14, 14, 14, 13, 14, 14, 13},
	{14, 14, 14, 14, 14, 14,  5,  5,  5,  5, 14, 14,  5},
	{14, 14, 14, 14, 14, 14,  6,  6,  6,  6, 14, 14,  6},
	{14, 14, 14, 14, 13, 14,  7,  8,  9, 10, 13, 14, 10},
	{14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 12, 14, 13}
};

// Negative: Term
#define TERM(x) (-static_cast<int>(x))
// Positive: Rule
#define RULE(x) (static_cast<int>(x))

int RulesTable[13][5] = 
{
	{0},			// -
	{2,-1,0},		// 1
	{3,-1,0},		// 2
	{5,-3,-2,0},	// 3
	{4,-3,-4,0},	// 4
	{-6,-5,6,6,0},	// 5
	{7,6,0},		// 6
	{-7,0},			// 7
	{-8,0},			// 8
	{-9,0},			// 9
	{-10,0},		// 10
	{7,-12,-11,0},	// 11
	{-12,-11,0}		// 12
};

enum class TSymbol
{
	// Terminals
	T_NEW = 1,	//kw_new
	T_ARRAY,	//kw_array
	T_ID,		//id
	T_DICT,		//kw_dict
	T_BRACE_L,	//(
	T_BRACE_R,	//)
	T_BOOL,		//!
	T_INT,		//%
	T_FLOAT,	//#
	T_STR,		//$
	T_INDEX_L,	//[
	T_INDEX_R,	//]
	T_END,
	T_INVALID
};

enum class NSymbol
{
	// Non-terminals
	N_OPNEW = 1,
	N_ARRAY,
	N_DICT,
	N_DICTTYPE,
	N_ARRAYTYPE,
	N_ATOMICTYPE,
	N_DIMDECL
};

TSymbol Lex(TokenStream &tk)
{
	const Token &t = tk.Current();

	if (t.GetType() == TOKEN_ID)
	{
		return TSymbol::T_ID;
	}
	else if (t.GetType() == TOKEN_KEYWORD || t.GetType() == TOKEN_DELIMETER)
	{
		switch (t.GetValueK())
		{
		case KW_NEW:
			return TSymbol::T_NEW;
		case KW_ARRAY:
			return TSymbol::T_ARRAY;
		case KW_DICT:
			return TSymbol::T_DICT;
		case KW_BRACE_L:
			return TSymbol::T_BRACE_L;
		case KW_BRACE_R:
			return TSymbol::T_BRACE_R;
		case KW_BOOL:
			return TSymbol::T_BOOL;
		case KW_INT:
			return TSymbol::T_INT;
		case KW_FLOAT:
			return TSymbol::T_FLOAT;
		case KW_STRING:
			return TSymbol::T_STR;
		case KW_INDEX_L:
			return TSymbol::T_INDEX_L;
		case KW_INDEX_R:
			return TSymbol::T_INDEX_R;
		}
	}
	else if (t.GetType() == TOKEN_EOF)
	{
		return TSymbol::T_END;
	}

	return TSymbol::T_INVALID;
}

shared_ptr<IAst> LLParseNew(TokenStream &tk)
{
	tk.PushPosition();

	stack<int> s;

	s.push(TERM(TSymbol::T_END));
	s.push(RULE(NSymbol::N_OPNEW));

    while (!s.empty())
	{
        int symb = s.top();
		s.pop();

        TSymbol token = Lex(tk);
        if (symb < 0) // Term
		{
			if (abs(symb) == static_cast<int>(token))
			{
                tk.Forward();	// ok
			}
			else
			{
                break;			// bad
			}
		}
		else if (symb > 0)	// Rule
		{
			int rule = ParsingTable[symb][static_cast<int>(token)];

			if (rule == static_cast<int>(TSymbol::T_INVALID))
			{
				break;
			}

			int *r = RulesTable[rule - 1];

			// Reverse
			stack<int> temp;
			do
			{
				temp.push(*r);
				r++;
			} while (*r != 0);

			while (!temp.empty())
			{
				s.push(temp.top());
				temp.pop();
			}                
		}
	}

	tk.PopPosition();
	return AstNew(s);
}
