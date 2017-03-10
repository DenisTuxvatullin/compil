#include <sstream>
#include "Exception.h"
#include "Token.h"

using namespace std;

CompileError::CompileError(const char *description, const Token &token)
	: runtime_error(PrepareMessage(description, token))
{
}

string CompileError::PrepareMessage(const char *description, const Token &token)
{
	stringstream result;

	result << "Error at line " << token.GetLineNumber() << ": " << description << endl;
	result << token.GetLine() << endl;

	int position = token.GetOffset();
	for (int i = 0 ; i < position; i++)
	{
		result << ' ';
	}
	result << '^' << endl;

	return result.str();
}

InternalError::InternalError(const string &description)
	: runtime_error("Internal error: " + description)
{
}

IntermediateError::IntermediateError(const string &description)
	: runtime_error(description)
{
}
