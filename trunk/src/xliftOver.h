/*
 * xliftOver.h
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */

#ifndef XLIFTOVER_H_
#define XLIFTOVER_H_
#include <memory>
#include <string>
#include "segments.h"

class LiftOver
	{
	public:
		LiftOver(const char* mapFile);
		virtual ~LiftOver();
		void minBlocks(double v);
		void minMatch(double v);
		double minBlocks() const;
		double minMatch() const;
		const char* lastError() const;
		std::auto_ptr<ChromStrandStartEnd> convert(const ChromStrandStartEnd* src);
		std::auto_ptr<ChromStartEnd> convert(const ChromStartEnd* src);
	private:
		void *chainHash;
		double _minMatch;
		double _minBlocks;
		std::auto_ptr<std::string> last_error;
	};


#endif /* XLIFTOVER_H_ */
