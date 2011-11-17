#ifndef ABSTRACT_CHARSEQUENCE_H
#define ABSTRACT_CHARSEQUENCE_H

#include <iostream>
#include <stdint.h>
#include <memory>
#include <string>
/**
 * AbstractCharSequence
 */
class AbstractCharSequence
	{
	public:
	    AbstractCharSequence();
	    virtual ~AbstractCharSequence();
	    virtual char at(int32_t index) const=0;
	    virtual int32_t size() const=0;
	    char operator[](int32_t index) const;
	    virtual void print(std::ostream& out) const;
	    virtual std::auto_ptr<std::string> toString() const;
	};

#endif
