/*
 * geneticcode.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
 */

#ifndef GENETICCODE_H
#define GENETICCODE_H

#include <string>

class GeneticCode
    {
    private:
	std::string ncbi;
	int base2index(char c) const;

    public:
	GeneticCode(const char* ncbi);
	~GeneticCode();
	bool isStop(char c) const;
	char translate(char b1,char b2,char b3) const;
	static const GeneticCode* standard();
	static const GeneticCode* mitochondrial();
    };

#endif /* GENETICCODE_CPP_ */
