/*
 * geneticcode.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
 */
#include <cctype>
#include "geneticcode.h"

static const GeneticCode STANDARD("FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG");
static const GeneticCode MITOCHONDRIAL("FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSS**VVVVAAAADDEEGGGG");


int GeneticCode::base2index(char c) const
    {
    switch(c)
	{
	case 'T':case 't': return 0;
	case 'C':case 'c': return 1;
	case 'A':case 'a': return 2;
	case 'G':case 'g': return 3;
	default: return -1;
	}
    }

char GeneticCode::translate(char b1,char b2,char b3) const
    {
    int base1= base2index(b1);
    int base2= base2index(b2);
    int base3= base2index(b3);
    if(base1==-1 || base2==-1 || base3==-1)
	    {
	    return '?';
	    }
    else
	    {
	    return ncbi.at(base1*16+base2*4+base3);
	    }
    }

GeneticCode::GeneticCode(const char* ncbi):ncbi(ncbi)
    {

    }

GeneticCode::~GeneticCode()
    {

    }

const GeneticCode* GeneticCode::standard()
    {
    return &STANDARD;
    }

const GeneticCode* GeneticCode::mitochondrial()
    {
    return &MITOCHONDRIAL;
    }
