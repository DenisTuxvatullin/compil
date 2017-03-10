#include "LR.h"
#include "Parser.h"
#include "IAst.h"

VarDeclLR::VarDeclLR()
	: m_index(0)
{
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::VarDecl] = &Confirm;
		m[TOKEN::id] = &ID11;
		m_table[STATEMENT::BEGIN] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::VarType] = &VarType12;
		m[TOKEN::AtomicType] = &AT21AT31AT71;
		m[TOKEN::DictType] = &DT31;
		m[TOKEN::ArrType] = &ArT41;
		m[TOKEN::TypeInt] = &SimpleType;
		m[TOKEN::TypeBool] = &SimpleType;
		m[TOKEN::TypeDouble] = &SimpleType;
		m[TOKEN::TypeStr] = &SimpleType;
		m_table[STATEMENT::ID11] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::EOLN] = &Reduce1;
		m_table[STATEMENT::VT12] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::AtomicType] = &AT62;
		m[TOKEN::DimDecl] = &DimD72;
		m[TOKEN::indexL] = &RSB81RSB91;
		m[TOKEN::EOLN] = &Reduce2;
		m_table[STATEMENT::AT21AT61AT71] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::EOLN] = &Reduce3;
		m_table[STATEMENT::DT31] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::EOLN] = &Reduce4;
		m_table[STATEMENT::ArT41] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::AtomicType] = &Reduce5;
		m[TOKEN::BraceR] = &Reduce5;
		m[TOKEN::DimDecl] = &Reduce5;
		m[TOKEN::EOLN] = &Reduce5;
		m_table[STATEMENT::Type51] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::BraceR] = &Br63;
		m_table[STATEMENT::AT62] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::EOLN] = &Reduce7;
		m_table[STATEMENT::DimD72] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::indexR] = &LSB82LSB92;
		m_table[STATEMENT::RSB81RSB91] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::BraceR] = &Br64;
		m_table[STATEMENT::Br63] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::DimDecl] = &DimD82;
		m[TOKEN::EOLN] = &Reduce9;
		m_table[STATEMENT::LSB82LSB92] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::EOLN] = &Reduce6;
		m_table[STATEMENT::Br64] = m;
	}
	{
		std::map<TOKEN, PARSEFN> m;
		m[TOKEN::EOLN] = &Reduce8;
		m_table[STATEMENT::DimD82] = m;
	}
	m_ss.push(STATEMENT::BEGIN);
}

VarDeclLR::~VarDeclLR()
{
	m_ss.push(STATEMENT::BEGIN);
}

bool VarDeclLR::ID11(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::ID11);
	c.m_st.push(TOKEN::id);
	c.m_index++;
	return false;
}

bool VarDeclLR::Confirm(VarDeclLR &c)
{
	return true;
}

bool VarDeclLR::VarType12(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::VT12);
	c.m_st.push(TOKEN::VarType);
	c.m_index++;
	return false;
}

bool VarDeclLR::AT21AT31AT71(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::AT21AT61AT71);
	c.m_st.push(TOKEN::AtomicType);
	c.m_index++;
	return false;
}

bool VarDeclLR::SimpleType(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::Type51);
	c.m_st.push(TOKEN::TypeInt);
	c.m_index++;
	return false;
}

bool VarDeclLR::DT31(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::DT31);
	c.m_st.push(TOKEN::DictType);
	c.m_index++;
	return false;
}

bool VarDeclLR::ArT41(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::ArT41);
	c.m_st.push(TOKEN::ArrType);
	c.m_index++;
	return false;
}

bool VarDeclLR::AT62(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::AT62);
	c.m_st.push(TOKEN::AtomicType);
	c.m_index++;
	return false;
}

bool VarDeclLR::DimD72(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::DimD72);
	c.m_st.push(TOKEN::DimDecl);
	c.m_index++;
	return false;
}

bool VarDeclLR::RSB81RSB91(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::RSB81RSB91);
	c.m_st.push(TOKEN::indexL);
	c.m_index++;
	return false;
}

bool VarDeclLR::Br63(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::Br63);
	c.m_st.push(TOKEN::BraceL);
	c.m_index++;
	return false;
}

bool VarDeclLR::LSB82LSB92(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::LSB82LSB92);
	c.m_st.push(TOKEN::indexR);
	c.m_index++;
	return false;
}

bool VarDeclLR::Br64(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::Br64);
	c.m_st.push(TOKEN::BraceR);
	c.m_index++;
	return false;
}

bool VarDeclLR::DimD82(VarDeclLR &c)
{
	c.m_ss.push(STATEMENT::DimD82);
	c.m_st.push(TOKEN::DimDecl);
	c.m_index++;
	return false;
}

bool VarDeclLR::Reduce1(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::VarDecl);
	return false;
}

bool VarDeclLR::Reduce2(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::VarType);
	return false;
}

bool VarDeclLR::Reduce3(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::VarType);
	return false;
}

bool VarDeclLR::Reduce4(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::VarType);
	return false;
}

bool VarDeclLR::Reduce5(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::AtomicType);
	return false;
}

bool VarDeclLR::Reduce6(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::DictType);
	return false;
}

bool VarDeclLR::Reduce7(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::ArrType);
	return false;
}

bool VarDeclLR::Reduce8(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::DimDecl);
	return false;
}

bool VarDeclLR::Reduce9(VarDeclLR &c)
{
	c.m_ss.pop();
	c.m_st.pop();
	c.m_ss.pop();
	c.m_st.pop();
	c.m_st.push(TOKEN::DimDecl);
	return false;
}

bool VarDeclLR::Parse(const std::vector<TOKEN> &arg, size_t num)
{
	m_index = num;
	bool notEx = false;
	bool res = false;
	do
	{
		auto it = m_table[m_ss.top()].find(arg[m_index]);
		notEx = it == m_table[m_ss.top()].end();
		if (!notEx)
		{
			res = m_table[m_ss.top()][arg[m_index]](*this);
		}
	} while (!notEx && !res);
	return res;
}

TOKEN Lex(const Token &t)
{
	if (t.GetType() == TOKEN_ID)
	{
		return TOKEN::id;
	}
	else if (t.GetType() == TOKEN_KEYWORD || t.GetType() == TOKEN_DELIMETER)
	{
		switch (t.GetValueK())
		{
		case KW_BRACE_L:
			return TOKEN::BraceL;
		case KW_BRACE_R:
			return TOKEN::BraceR;
		case KW_BOOL:
			return TOKEN::TypeBool;
		case KW_INT:
			return TOKEN::TypeInt;
		case KW_FLOAT:
			return TOKEN::TypeDouble;
		case KW_STRING:
			return TOKEN::TypeStr;
		case KW_INDEX_L:
			return TOKEN::indexL;
		case KW_INDEX_R:
			return TOKEN::indexR;
		}
	}

	return TOKEN::EOLN;
}

std::shared_ptr<IAst> ParseVariableLR(TokenStream &tk)
{
	tk.PushPosition();

	std::vector<TOKEN> tokens;

	while (!tk.IsEOS())
	{
		tokens.push_back(Lex(tk.Current()));
		tk.Forward();
	}

	VarDeclLR parser;
	auto res = parser.Parse(tokens, 0);

	tk.PopPosition();
	return AstVariable(res);
}
