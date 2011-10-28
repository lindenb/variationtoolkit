/*
 * xliftOver.h
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */

#ifndef XLIFTOVER_H_
#define XLIFTOVER_H_
#include "segments.h"

class LiftOver
	{
	private:
		void *chainHash;
		double minMatch;
		double minBlocks;
	public:
		LiftOver(const char* mapFile);
		virtual ~LiftOver();
		void convert(const char* chrom,int start,int end, char* strand);
	};


#endif /* XLIFTOVER_H_ */
