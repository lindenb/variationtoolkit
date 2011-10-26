#include "segments.h"
#include "smartcmp.h"
using namespace std;

static SmartComparator _SMART;

ChromPosition::ChromPosition(const char* s,int32_t pos):chrom(s),pos(pos)
	{
	}

ChromPosition::ChromPosition(const string& s,int32_t pos):chrom(s),pos(pos)
	{
	}

ChromPosition::ChromPosition(const ChromPosition& cp):chrom(cp.chrom),pos(cp.pos)
	{
	}
ChromPosition::~ChromPosition()
	{
	}
	
ChromPosition& ChromPosition::operator=(const ChromPosition& cp)
	{
	if(this!=&cp)
		{
		chrom.assign(cp.chrom);
		pos=cp.pos;
		}
	return (*this);
	}
	
bool ChromPosition::operator==(const ChromPosition& cp)
	{
	return pos==cp.pos && _SMART(chrom,cp.chrom)==0;
	}
	
bool ChromPosition::operator<(const ChromPosition& cp)
	{
	int i=_SMART(chrom,cp.chrom);
	if(i!=0) return i;
	return pos-cp.pos;
	}



std::ostream& operator << (std::ostream& out,const ChromPosition& cp)
	{
	out << cp.chrom << ":" << cp.pos;
	return out;
	}




StartEnd::StartEnd(int32_t start,int32_t end):start(start),end(end)
	{
	}

StartEnd& StartEnd::operator=(const StartEnd& cp)
	{
	start=cp.start;
	end=cp.end;	
	return (*this);
	}
	
	
bool StartEnd::operator==(const StartEnd& cp)
	{
	return start==cp.start && cp.end==end;
	}
	
bool StartEnd::operator<(const StartEnd& cp)
	{
	int i= start-cp.start;
	if(i!=0) return i;
	return end-cp.end;
	}


std::ostream& operator << (std::ostream& out,const StartEnd& cp)
	{
	out << cp.start << "-" << cp.end;
	return out;
	}

ChromStartEnd::ChromStartEnd(const std::string& s,int32_t start,int32_t end):StartEnd(start,end),chrom(s)
	{
	}

ChromStartEnd::ChromStartEnd(const char* s,int32_t start,int32_t end):StartEnd(start,end),chrom(s)
	{
	}
	
ChromStartEnd::ChromStartEnd(const ChromStartEnd& cp):StartEnd(cp.start,cp.end),chrom(cp.chrom)
	{
	}
ChromStartEnd::~ChromStartEnd()
	{
	}
	
ChromStartEnd& ChromStartEnd::operator=(const ChromStartEnd& cp)
	{
	if(this!=&cp)
		{
		chrom.assign(cp.chrom);
		start=cp.start;
		end=cp.end;
		}
	return (*this);
	}
	
bool ChromStartEnd::operator==(const ChromStartEnd& cp)
	{
	return start==cp.start && cp.end==end && _SMART(chrom,cp.chrom)==0;
	}
	
bool ChromStartEnd::operator<(const ChromStartEnd& cp)
	{
	int i=_SMART(chrom,cp.chrom);
	if(i!=0) return i;
	i= start-cp.start;
	if(i!=0) return i;
	return end-cp.end;
	}


std::ostream& operator << (std::ostream& out,const ChromStartEnd& cp)
	{
	out << cp.chrom << ":" << cp.start << "-" << cp.end;
	return out;
	}


