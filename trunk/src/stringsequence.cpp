#include "stringsequence.h"


StringSequence::StringSequence()
	{
	}

StringSequence::~StringSequence()
	{
	}

char StringSequence::at(int32_t index) const
	{
	return content.at(index);
	}
int32_t StringSequence::size() const
	{
	return (int32_t)content.size();
	}
