/*
 * vcfreader.cpp
 *
 *  Created on: Jan 1, 2012
 *      Author: lindenb
 */
#include "tokenizer.h"
#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <stdint.h>

class VCFReader
    {
    private:
	std::istream& in;
	Tokenizer tab;
	Tokenizer colon;
	Tokenizer semicolon;
    public:
	class AbstractMeta
	    {
	    protected:
		std::string id;
		std::string desc;
		AbstractMeta();
		AbstractMeta(const AbstractMeta& cp);
	    public:
		~AbstractMeta();
	    friend class VCFReader;
	    };

	class MetaFilter:public AbstractMeta
	    {
	    public:

		MetaFilter();
		MetaFilter(const MetaFilter& cp);
		~MetaFilter();
		MetaFilter& operator=(const MetaFilter& cp);
		bool operator==(const MetaFilter& cp) const;
		bool operator<(const MetaFilter& cp) const;
		friend class VCFReader;
	    };

	class Meta:public AbstractMeta
	    {
	    public:
		std::string count;
		std::string type;
		Meta();
		Meta(const Meta& cp);
		~Meta();
		Meta& operator=(const Meta& cp);
		bool operator==(const Meta& cp) const;
		bool operator<(const Meta& cp) const;
		friend class VCFReader;
	    };

	typedef Meta MetaInfo;
	typedef Meta MetaFormat;

	class Header
	    {
	    public:
		std::vector<std::string> meta;
		std::map<std::string,MetaFilter> filters;
		std::map<std::string,MetaInfo> info;
		std::map<std::string,MetaFormat> format;
		std::vector<std::string> samples;
	    };

	class Call
	    {
	    public:
		std::string sample;
		std::map<std::string,std::string> call;
		std::set<std::string> formats() const;
	    };

	class Row
	    {
	    public:
		std::string chrom;
		int32_t pos;
		std::string id;
		std::string ref;
		std::string alt;
		std::string qual;
		std::set<std::string> filter;
		std::multimap<std::string,std::string> info;
		std::vector<Call> calls;
		std::size_t nLine;
		std::set<std::string> formats() const;

	    };
	VCFReader(std::istream& in);
	~VCFReader();
	const Header* header();
	const std::vector<std::string>& samples() const;
	std::auto_ptr<Row> next();
    private:
	Header* _header;
	std::size_t nLine;
	Meta parse_meta(std::string line) const;
	MetaFilter parse_filter(std::string line) const;
	std::string parse_meta(const char* key,const std::string& line) const;
    };

std::ostream& operator << (std::ostream& out,const VCFReader::MetaFilter& o);
std::ostream& operator << (std::ostream& out,const VCFReader::Meta& o);
std::ostream& operator << (std::ostream& out,const VCFReader::Row& o);
