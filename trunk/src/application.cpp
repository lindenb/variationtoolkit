/*
 * application.h
 *
 *  Created on: Oct 14, 2011
 *      Author: lindenb
 */

#include "application.h"

using namespace std;

volatile sig_atomic_t AbstractApplication::stop_called=0;

void AbstractApplication::catch_signal(int sig)
    {
    stop_called=1;
    }

bool AbstractApplication::stopping()
    {
    return stop_called!=0;
    }

AbstractApplication::AbstractApplication()
    {
    signal (SIGPIPE, catch_signal);
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

int AbstractApplication::argument(int optind,int argc,char** argv)
    {
    return -1;
    }
