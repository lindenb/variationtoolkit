/*
 * application.h
 *
 *  Created on: Oct 14, 2011
 *      Author: lindenb
 */

#include "application.h"

using namespace std;

AbstractApplication::AbstractApplication()
	{
	}

AbstractApplication::~AbstractApplication()
	{
	}

void AbstractApplication::usage(std::ostream& out,int argc,char** argv)
	{
	}

void AbstractApplication::usage(int argc,char** argv)
	{
	usage(cerr,argc,argv);
	}

void AbstractApplication::redirectTo(const char* filename)
	{

	}
