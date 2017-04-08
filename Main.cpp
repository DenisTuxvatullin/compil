#include <iostream>
#include <fstream>
#include "Lexer.h"
#include "Token.h"
#include "Exception.h"
#include "Parser.h"
#include "AssemblerTest.h"

using namespace std;

void Compile(const string &filePath)
{
	ifstream is(filePath);

	if (!is.is_open())
	{
		cout << "Cannot open file '" << filePath << "'\n";
		return;
	}

	Lexer lex;
	vector<Token> tokens;
	int lineNumber = 1;

	while (!is.eof())
	{
		string expr;

		getline(is, expr);
		if (expr.length() == 0)
		{
			continue;
		}
		
		vector<Token> line = lex.ParseLine(expr, lineNumber++);
		for (const Token &token : line)
		{
			tokens.push_back(token);
			token.Print();
			cout << endl;

		}
	}
	shared_ptr<IAst> ast = Parse(tokens);
}

int main(int argc, char *argv[])
{

	string file = "Adder.txt" ;
	//cout << "Input file: ";
	//cin >> file;
	InitOperatorMap();
	try
	{	
		Compile(file);		
	}
	catch (const IntermediateError &ex)
	{
		cout << "Semantic error: " << ex.what() << endl;
	}
	catch (const CompileError &ex)
	{
		cout << ex.what() << endl;
	}
	catch (const InternalError &ex)
	{
		cout << ex.what() << endl;
	}
	cout << "Done\n";
	cin >> file;
	return 0;
}
