#pragma once
#include <string>

// Used in flow control instructions (br, brtrue...) in Function class
class Label
{
	friend class Function;

private:
	// To prevent emitting same label twice
	void SetEmitted();

public:
	Label();
	~Label();

	// Returns name of the label, for example "L_12"
	std::string ToString() const;

private:
	unsigned m_index;
	bool m_emitted;
};
