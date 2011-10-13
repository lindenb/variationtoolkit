/*
 * KentBigWig.cpp
 *
 *  Created on: Oct 13, 2011
 *      Author: lindenb
 */
#ifndef KENT_BIG_BED_H
#define KENT_BIG_BED_H

#include <vector>
#include <string>
#include <stdint.h>
#include <memory>
class BigBed
	{
	public:
			class Interval
				{
				public:
					int32_t start;
					int32_t end;
					std::string line;
					Interval(int32_t start,int32_t end,const char* rest);
					Interval(const Interval& cp);
					Interval& operator=(const Interval& cp);
				};

			BigBed(const char* filename);
			virtual ~BigBed();
			virtual void close();
			virtual std::auto_ptr<std::vector<Interval > > query(const char* chrom,int32_t start,int32_t end,int32_t maxItems=0);
	private:
			void* bbFile;
	};

#endif
