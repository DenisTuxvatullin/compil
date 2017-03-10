#include <sstream>
#include <iostream>
#include "Label.h"
#include "Exception.h"

using namespace std;

const char LABEL_PREFIX[] = "L_";

Label::Label()
	: m_emitted(false)
{
	static unsigned globalIndex = 0;
	m_index = globalIndex++;
}

Label::~Label()
{
	if (!m_emitted)
	{
		cout << "!!! Label " << ToString() << " was not emitted\n";

		// Don't throw exceptions in destructor
		//throw InternalError("Label " + ToString() + " was not emitted");
	}
}

void Label::SetEmitted()
{
	if (m_emitted)
	{
		throw InternalError("Label " + ToString() + " was already emitted");
	}

	m_emitted = true;
}

std::string Label::ToString() const
{
	stringstream labelName;
	labelName << LABEL_PREFIX << m_index;
	return labelName.str();
}
