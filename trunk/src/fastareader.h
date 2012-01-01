#ifndef FASTAREADER_H
#define FASTAREADER_H
#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include "abstractcharsequence.h"

class FastaReader;

class FastaSequence:public AbstractCharSequence
	{
	private:
		std::string _name;
		std::string _seq;
		FastaSequence();
	public:
		static const int32_t DEFAULT_LINE_LENGTH;
		FastaSequence(const FastaSequence& cp);
		virtual ~ FastaSequence();
		virtual char at(int32_t index) const;
		virtual int32_t size() const;
		const char* name() const;
		const char* c_str() const;
		FastaSequence& operator=(const FastaSequence& cp);
		void printFasta(std::ostream& out,int32_t lineLength) const;
		void printFasta(std::ostream& out);
	friend class FastaReader;
	};

class FastaReader
	{
	private:
		std::size_t _reserve;
		bool to_upper;
	public:
		FastaReader();
		~FastaReader();
		FastaReader& reserve(int32_t len);
		FastaReader& toupper(bool choice);
		std::auto_ptr<FastaSequence> next(std::istream& in);
	};

#endif

