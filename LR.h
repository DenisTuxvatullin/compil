#pragma once
#include <stack>
#include <map>
#include <functional>
#include <vector>
#include <memory>
#include "Ast.h"
#include "TokenStream.h"

std::shared_ptr<IAst> ParseVariableLR(TokenStream &tk);

/*
VarDecl -> id VarType
VarType -> AtomicType | DictType | ArrayType
AtomicType -> ! | % | # | $
DictType -> AtomicType AtomicType ( )
ArrayType -> AtomicType DimDecl
DimDecl -> [ ] DimDecl | [ ]
*/

class VarDeclLR;
typedef bool(*PARSEFN)(VarDeclLR &c);

enum class STATEMENT
{
	BEGIN,
	ID11,
	VT12,
	AT21AT61AT71,
	DT31,
	ArT41,
	Type51,
	AT62,
	DimD72,
	RSB81RSB91,
	Br63,
	LSB82LSB92,
	Br64,
	DimD82
};

enum class TOKEN
{
	VarDecl,
	id,
	VarType,
	AtomicType,
	DictType,
	ArrType,
	TypeBool,
	TypeInt,
	TypeDouble,
	TypeStr,
	BraceR,
	BraceL,
	indexL,
	indexR,
	DimDecl,
	EOLN
};

class VarDeclLR
{
public:
	VarDeclLR();
	~VarDeclLR();
	bool Parse(const std::vector<TOKEN> &arg, size_t num);

private:
	static bool ID11(VarDeclLR &c);
	static bool Confirm(VarDeclLR &c);
	static bool VarType12(VarDeclLR &c);
	static bool AT21AT31AT71(VarDeclLR &c);
	static bool SimpleType(VarDeclLR &c);
	static bool Reduce1(VarDeclLR &c);
	static bool Reduce2(VarDeclLR &c);
	static bool Reduce3(VarDeclLR &c);
	static bool Reduce4(VarDeclLR &c);
	static bool Reduce5(VarDeclLR &c);
	static bool Reduce6(VarDeclLR &c);
	static bool Reduce7(VarDeclLR &c);
	static bool Reduce8(VarDeclLR &c);
	static bool Reduce9(VarDeclLR &c);
	static bool DT31(VarDeclLR &c);
	static bool ArT41(VarDeclLR &c);
	static bool AT62(VarDeclLR &c);
	static bool DimD72(VarDeclLR &c);
	static bool RSB81RSB91(VarDeclLR &c);
	static bool Br63(VarDeclLR &c);
	static bool LSB82LSB92(VarDeclLR &c);
	static bool Br64(VarDeclLR &c);
	static bool DimD82(VarDeclLR &c);

	std::stack<STATEMENT> m_ss;
	std::stack<TOKEN> m_st;
	size_t m_index;
	std::map<STATEMENT, std::map<TOKEN, PARSEFN>> m_table;
};
