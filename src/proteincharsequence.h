#ifndef PROTEIN_CHAR_SEQUENCE_H
#define PROTEIN_CHAR_SEQUENCE_H

#include "abstractcharsequence.h"
#include "geneticcode.h"

/**
 * ProteinCharSequence
 */
class ProteinCharSequence:public AbstractCharSequence
    {
    private:
		const GeneticCode* code;
		const AbstractCharSequence* delegate;
    public:
		ProteinCharSequence(
			const GeneticCode* code,
			const AbstractCharSequence* delegate);
		virtual ~ProteinCharSequence();
		virtual char at(int32_t index) const;
		virtual int32_t size() const;
	};

#endif
