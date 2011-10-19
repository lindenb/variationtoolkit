#ifndef RGB_COLOR_H
#define RGB_COLOR_H
#include <iostream>
#include <stdint.h>


struct Color
	{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	
	Color();
	Color(const char* s);
	Color(uint8_t r,uint8_t g,uint8_t b);
	Color(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
	Color(double r,double g,double b,double a);
	Color(double r,double g,double b);
	Color(double g);
	Color(uint32_t L);
	uint32_t asInt() const;
	};

std::ostream& operator<<(std::ostream& out,const Color& c);

#endif

