#include "color.h"

#define MUL(n) (n*255.0)
#define TRIM(n) (uint8_t)(MUL(n)<0?0:(MUL(n)>255?255:MUL(n)))


Color::Color(const char* s)
	{
	extern void parse_color_str(Color*,const char*);
	parse_color_str(this,s);
	}

Color::Color():r(0),g(0),b(0),a(0)
	{
	}

Color::Color(uint8_t r,uint8_t g,uint8_t b):r(r),g(g),b(b),a(0)
	{
	}

Color::Color(uint8_t r,uint8_t g,uint8_t b,uint8_t a):r(r),g(g),b(b),a(a)
	{
	}

Color::Color(double r,double g,double b,double a):r(TRIM(r)),g(TRIM(g)),b(TRIM(b)),a(TRIM(a))
	{
	
	}

Color::Color(double r,double g,double b):r(TRIM(r)),g(TRIM(g)),b(TRIM(b)),a(0)
	{
	}

Color::Color(double g):r(TRIM(g)),g(TRIM(g)),b(TRIM(g)),a(0)
	{
	}

Color::Color(uint32_t L)
	{
	uint8_t* p=(uint8_t*)&L;
	r=p[0];
	g=p[1];
	b=p[2];
	a=p[3];
	}

uint32_t Color::asInt() const
	{
	uint32_t  L=0L;
	uint8_t* p=(uint8_t*)L;
	p[0]=r;
	p[1]=g;
	p[2]=b;
	p[3]=a;
	return L;
	}

std::ostream& operator<<(std::ostream& out,const Color& c)
	{
	out << "rgb(" << (int)c.r <<"," << (int)c.g << "," << (int)c.b << ")";
	return out;
	}


