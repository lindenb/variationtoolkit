#include <sstream>
#include "abstractcharsequence.h"


AbstractCharSequence::AbstractCharSequence()
	{
	}
AbstractCharSequence::~AbstractCharSequence()
	{
	}

char AbstractCharSequence::operator[](int32_t index) const
	{
	return at(index);
	}

void AbstractCharSequence::print(std::ostream& out) const
	{
	for(int32_t i=0;i< size();++i)
		{
		out << at(i);
		}
	}

std::auto_ptr<std::string> AbstractCharSequence::toString() const
	{
	std::ostringstream os;
	print(os);
	return std::auto_ptr<std::string>(new std::string(os.str()));
	}
