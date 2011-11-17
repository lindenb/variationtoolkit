#ifndef STRING_SEQUENCE_H
#define STRING_SEQUENCE_H
#include <string>
#include "abstractcharsequence.h"

/**
 * StringSequence
 */
class StringSequence:public AbstractCharSequence
    {
    public:
		std::string content;
		StringSequence();
		virtual ~StringSequence();
		virtual char at(int32_t index) const;
		virtual int32_t size() const;
    };

#endif
