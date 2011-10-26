#include "shapes.h"

typedef double coord_t;


typedef double coord_t;

Dimension::Dimension():width(0),height(0)
	{
	}

std::ostream& operator<<(std::ostream& out,const Dimension& d)
	{
	out << "("<< d.width<<","<< d.height <<")";
	return out;
	}

Rectangle::Rectangle():x(0),y(0),width(0),height(0)
	{

	}

coord_t Rectangle::maxx() const
	{
	return x+width;
	}

coord_t Rectangle::maxy() const
	{
	return y+height;
	}

std::ostream& operator<<(std::ostream& out,const Rectangle& o)
	{
	out << "("<< o.x<<","<< o.y <<","<< o.width<<","<< o.height <<")";
	return out;
	}


Point::Point():x(0),y(0)
	{
	}

std::ostream& operator<<(std::ostream& out,const Point& o)
	{
	out << "("<< o.x<<","<< o.y <<")";
	return out;
	}

Line::Line():x1(0),y1(0),x2(0),y2(0)
	{
	}

std::ostream& operator<<(std::ostream& out,const Line& o)
	{
	out << "("<< o.x1<<","<< o.y1 <<","<< o.x2<<","<< o.y2 <<")";
	return out;
	}


Insets::Insets():top(0),bottom(0),left(0),right(0)
	{
	}


std::ostream& operator<<(std::ostream& out,const Insets& o)
	{
	out << "("<< o.top<<","<< o.left <<","<< o.bottom<<","<< o.right <<")";
	return out;
	}
