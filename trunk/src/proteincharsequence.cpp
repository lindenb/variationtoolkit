#include "proteincharsequence.h"


ProteinCharSequence::ProteinCharSequence(
		const GeneticCode* code,
		const AbstractCharSequence* delegate):
		    code(code),delegate(delegate)
	{
	}

ProteinCharSequence::~ProteinCharSequence()
	{
	}

char ProteinCharSequence::at(int32_t index) const
	{
	return code->translate(
		delegate->at(index*3+0),
		delegate->at(index*3+1),
		delegate->at(index*3+2)
		);
	}

int32_t ProteinCharSequence::size() const
	{
	return delegate->size()/3;
	}

