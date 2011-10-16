#ifndef SEGMENT_H
#define SEGMENT_H

#include <iostream>
#include <string>
#include <stdint.h>

class ChromPosition
	{
	public:
		std::string chrom;
		int32_t pos;
		ChromPosition(const char* s,int32_t pos);
		ChromPosition(const ChromPosition& cp);
		~ChromPosition();
		ChromPosition& operator=(const ChromPosition& cp);
		bool operator==(const ChromPosition& cp);
		bool operator<(const ChromPosition& cp);
	};

std::ostream& operator << (std::ostream& out,const ChromPosition& cp);


struct StartEnd
	{
	int32_t start;
	int32_t end;
	StartEnd(int32_t start,int32_t end);
	StartEnd& operator=(const StartEnd& cp);
	bool operator==(const StartEnd& cp);
	bool operator<(const StartEnd& cp);
	};

std::ostream& operator << (std::ostream& out,const struct StartEnd& cp);

class ChromStartEnd:public StartEnd
	{
	public:
		std::string chrom;
		ChromStartEnd(const char* s,int32_t start,int32_t end);
		ChromStartEnd(const ChromStartEnd& cp);
		~ChromStartEnd();
		ChromStartEnd& operator=(const ChromStartEnd& cp);
		bool operator==(const ChromStartEnd& cp);
		bool operator<(const ChromStartEnd& cp);
	};

std::ostream& operator << (std::ostream& out,const ChromStartEnd& cp);


#endif

