/*
 * shapes.h
 *
 *  Created on: Oct 25, 2011
 *      Author: lindenb
 */

#ifndef SHAPES_H_
#define SHAPES_H_
#include <iostream>
typedef double coord_t;

struct Dimension
	{
	coord_t width;
	coord_t height;
	Dimension();
	};

std::ostream& operator<<(std::ostream& out,const Dimension& d);

struct Rectangle
	{
	coord_t x;
	coord_t y;
	coord_t width;
	coord_t height;
	Rectangle();
	coord_t maxx() const;
	coord_t maxy() const;
	};

std::ostream& operator<<(std::ostream& out,const Rectangle& d);

struct Point
	{
	coord_t x;
	coord_t y;
	Point();
	};

std::ostream& operator<<(std::ostream& out,const Point& d);

struct Line
	{
	coord_t x1;
	coord_t y1;
	coord_t x2;
	coord_t y2;
	Line();
	};

std::ostream& operator<<(std::ostream& out,const Line& d);

struct Insets
	{
	coord_t top;
	coord_t bottom;
	coord_t right;
	coord_t left;
	Insets();
	};

std::ostream& operator<<(std::ostream& out,const Insets& d);

#endif /* SHAPES_H_ */
