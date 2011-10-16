/*
 * application.h
 *
 *  Created on: Oct 14, 2011
 *      Author: lindenb
 */

#ifndef ABSTRACT_APPLICATION_H
#define ABSTRACT_APPLICATION_H

#include <iostream>
#include <fstream>
#include <csignal>
#include "tokenizer.h"

class AbstractApplication
	{
	public:
		Tokenizer tokenizer;

		AbstractApplication();
		virtual ~AbstractApplication();
		virtual void usage(std::ostream& out,int argc,char** argv);
		virtual void usage(int argc,char** argv);
		virtual void redirectTo(const char* filename);
		virtual int argument(int optind,int argc,char** argv);
		static bool stopping();
	private:
		static volatile sig_atomic_t stop_called;
		static void catch_signal(int sig);
	};

#endif
