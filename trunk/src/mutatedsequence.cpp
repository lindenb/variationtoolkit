#include "mutatedsequence.h"

MutedSequence::MutedSequence(const AbstractCharSequence* delegate):delegate(delegate)
	{
	}

MutedSequence::~MutedSequence()
	{
	}

char MutedSequence::at(int32_t index) const
	{
	std::map<int32_t,char>::const_iterator r= mutations.find(index);
	return r==mutations.end()?delegate->at(index):r->second;
	}

int32_t MutedSequence::size() const
	{
	return delegate->size();
	}

