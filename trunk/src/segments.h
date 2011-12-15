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
		ChromPosition(const std::string& ,int32_t pos);
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
		ChromStartEnd(const std::string& s,int32_t start,int32_t end);
		ChromStartEnd(const char* s,int32_t start,int32_t end);
		ChromStartEnd(const ChromStartEnd& cp);
		~ChromStartEnd();
		ChromStartEnd& operator=(const ChromStartEnd& cp);
		bool operator==(const ChromStartEnd& cp);
		bool operator<(const ChromStartEnd& cp);
	};

std::ostream& operator << (std::ostream& out,const ChromStartEnd& cp);

class TidStartEnd:public StartEnd
	{
	public:
		int32_t chrom;
		TidStartEnd(int32_t tid,int32_t start,int32_t end);
		TidStartEnd(const TidStartEnd& cp);
		~TidStartEnd();
		TidStartEnd& operator=(const TidStartEnd& cp);
		bool operator==(const TidStartEnd& cp);
		bool operator<(const TidStartEnd& cp);
	};

std::ostream& operator << (std::ostream& out,const TidStartEnd& cp);



class ChromStrandStartEnd:public ChromStartEnd
	{
	public:
		ChromStrandStartEnd(const ChromStartEnd& seg,char strand);
		ChromStrandStartEnd(const std::string& s,int32_t start,int32_t end,char strand);
		ChromStrandStartEnd(const char* s,int32_t start,int32_t end,char strand);
		ChromStrandStartEnd(const ChromStrandStartEnd& cp);
		~ChromStrandStartEnd();
		ChromStrandStartEnd& operator=(const ChromStrandStartEnd& cp);
		bool operator==(const ChromStrandStartEnd& cp);
		bool operator<(const ChromStrandStartEnd& cp);
		bool isForward() const;
		bool isReverse() const;
		char strand() const;
	private:
		char orient;
	};

std::ostream& operator << (std::ostream& out,const ChromStrandStartEnd& cp);




#endif

