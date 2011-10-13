/*
 * KentBigWig.cpp
 *
 *  Created on: Oct 13, 2011
 *      Author: lindenb
 */
#ifndef KENT_BIG_WIG_H
#define KENT_BIG_WIG_H

class BigWig
	{
	public:
			BigWig(const char* filename);
			virtual ~BigWig();
			virtual void close();
			double mean(const char*  chrom,int start,int end);
			double maximum(const char*  chrom,int start,int end);
			double minimum(const char*  chrom,int start,int end);
			double coverage(const char*  chrom,int start,int end);
			double stdDev(const char*  chrom,int start,int end);
	private:
			void* bbFile;
			double summary(const char*  chrom,int start,int end,int summaryType);
	};

#endif
