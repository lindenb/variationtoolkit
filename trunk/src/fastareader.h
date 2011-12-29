#ifndef FASTAREADER_H
#define FASTAREADER_H
#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include <cctype>
#include "throw.h"
#include "abstractcharsequence.h"

class FastaReader;

class FastaSequence:public AbstractCharSequence
	{
	private:
		std::string _name;
		std::string _seq;
		FastaSequence() {}
	public:
		
		FastaSequence(const FastaSequence& cp):_name(cp._name),_seq(cp._seq)
			{
			}
		virtual ~ FastaSequence()
			{
			}
		virtual char at(int32_t index) const
			{
			return _seq.at(index);
			}
		virtual int32_t size() const
			{
			return (int32_t)_seq.size();
			}
		const char* name() const
			{
			return _name.c_str();
			}
		const char* c_str() const
			{
			return _seq.c_str();
			}
		FastaSequence& operator=(const FastaSequence& cp)
			{
			if(this!=&cp)
				{
				_name.assign(cp._name);
				_seq.assign(cp._seq);
				}
			return *this;
			}
		void printFasta(std::ostream& out,int32_t lineLength) const
			{
			out << ">" << _name;
			for(int32_t i=0;i< size();++i)
				{
				if(i%lineLength==0) out << std::endl;
				out << at(i);
				}
			out << std::endl;
			}
		void printFasta(std::ostream& out)
			{
			printFasta(out,60);
			}
	friend class FastaReader;
	};

class FastaReader
	{
	private:
		std::size_t _reserve;
		bool to_upper;
	public:
		FastaReader():_reserve(BUFSIZ),to_upper(false)
			{
			}
		~FastaReader()
			{
			}

		FastaReader& reserve(int32_t len)
			{
			this->_reserve=(len<=0?BUFSIZ:len);
			return *this;
			}

		FastaReader& toupper(bool choice)
			{
			this->to_upper=choice;
			return *this;
			}
		
		std::auto_ptr<FastaSequence> next(std::istream& in)
			{
			std::auto_ptr<FastaSequence> ret(0);
			if(!in.good() || in.eof()) return ret;
			int c;

			while((c=in.get())!=EOF)
				{
				if(c=='>')
					{
					if(ret.get()!=0)
						{
						in.unget();
						return ret;
						}
					ret.reset(new FastaSequence);
					ret->_seq.reserve(_reserve);
					while((c=in.get())!=EOF && c!='\n')
						{
						if(c=='\r') continue;
						ret->_name+=(char)c;
						}
					continue;
					}
				if(std::isspace(c)) continue;
				if(!std::isalpha(c)) THROW("Bad char in sequence " << (char)c ) ;
				if(ret.get()==0) THROW("header missing");
				if(to_upper) c=std::toupper(c);
				ret->_seq+=(char)c;
				}
			return ret;
			}
	};

#endif

