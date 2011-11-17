#ifndef MUTATED_CHAR_SEQ_H
#define MUTATED_CHAR_SEQ_H
#include <map>
#include "abstractcharsequence.h"

class MutedSequence:public AbstractCharSequence
    {
    private:
	    const AbstractCharSequence* delegate;
    public:
	    std::map<int32_t,char> mutations;
	    MutedSequence(const AbstractCharSequence* delegate);
	    virtual ~MutedSequence();
	    virtual char at(int32_t index) const;
	    virtual int32_t size() const;
	};

#endif
