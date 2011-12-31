#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <iostream>
#include <cstdlib>



#include <string>
#include <vector>

#include <cstring>
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "numeric_cast.h"
#define NOWHERE
#include "where.h"

using namespace std;

#define SIMPLE_TAG(name,text) \
	::xmlTextWriterStartElement(writer,BAD_CAST name);\
	::xmlTextWriterWriteString(writer,BAD_CAST(text));\
	::xmlTextWriterEndElement(writer)

class VcfToXml:public AbstractApplication
    {
    private:
	xmlTextWriterPtr writer;
	class Meta
	    {
	    public:
		std::string id;
		std::string count;
		std::string type;
		std::string desc;
		void write(xmlTextWriterPtr writer,const char* tag)
		    {
		    ::xmlTextWriterStartElement(writer,BAD_CAST tag);
		    if(!id.empty()) {SIMPLE_TAG("id",id.c_str());}
		    if(!count.empty()) {SIMPLE_TAG("number",count.c_str());}
		    if(!type.empty()) {SIMPLE_TAG("type",type.c_str());}
		    if(!desc.empty()) {SIMPLE_TAG("description",desc.c_str());}
		    ::xmlTextWriterEndElement(writer);
		    }
	    };

	std::string parse_meta(const char* key,const std::string& line) const
	    {
	    std::string v;
	    std::string::size_type i=line.find(key,0);
	    if(i==string::npos) return v;
	    i+=strlen(key);
	    std::string::size_type j=line.find(",",i);
	    if(j==string::npos)
		{
		v.assign(line.substr(i));
		}
	    else
		{
		v.assign(line.substr(i,j-i));
		}
	    if(v.size()>0 && v[0]=='"')
		{
		v=v.substr(1);
		}
	    if(v.size()>0 && v[v.size()-1]=='\"')
		{
		v=v.substr(0,v.size()-1);
		}
	    return v;
	    }

	Meta* parse_meta(std::string line) const
	    {
	    Meta* meta=new Meta;
	    meta->id=parse_meta("ID=",line);
	    meta->count=parse_meta("Number=",line);
	    meta->type=parse_meta("Type=",line);
	    meta->desc=parse_meta("Description=",line);
	    return meta;
	    }
    public:

	VcfToXml():writer(0)
	    {

	    }


	void run(std::istream& in)
	    {
	    Tokenizer tab('\t');
	    int state=0;

	    vector<string> samples_ids;
	    vector<string> tokens;
	    vector<string> headers;
	    vector<Meta*> meta_infos;
	    vector<Meta*> meta_fmt;
	    vector<Meta*> meta_filter;
	    string line;
	    writer= xmlNewTextWriterFilename("-", 0);
	    xmlTextWriterStartDocument(writer, NULL,"UTF-8", NULL);
	    xmlTextWriterStartElement(writer,BAD_CAST "vcf");
	    xmlTextWriterStartElement(writer,BAD_CAST "head");

	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(state==0 &&
		    line.size()>1 &&
		    line[0]=='#' &&
		    line[1]=='#')
		    {
		    if(line.size()==2) continue;
		    if(line.size()>10 && line.compare(0,10,"##FORMAT=<")==0 && line.at(line.size()-1)=='>')
			{
			meta_fmt.push_back(parse_meta(line.substr(10,line.size()-11)));
			}
		    else if(line.size()>10 && line.compare(0,8,"##INFO=<")==0 && line.at(line.size()-1)=='>')
			{
			meta_infos.push_back(parse_meta(line.substr(8,line.size()-9)));
			}
		    else if(line.size()>10 && line.compare(0,10,"##FILTER=<")==0 && line.at(line.size()-1)=='>')
			{
			meta_filter.push_back(parse_meta(line.substr(10,line.size()-11)));
			}
		    else
			{
			headers.push_back(line.substr(2));
			}
		    }
		else if(state==0 &&
			line.size()>0 &&
			line[0]=='#'
			)
		    {
		    state=1;
		    tab.split(line,tokens);
		    if(tokens.size()<10) THROW("Expected at least 10 columns in " << line);
		    if(tokens[0].compare("#CHROM")!=0) THROW("Expected '#CHROM' in column 1 of " << line);
		    if(tokens[1].compare("POS")!=0) THROW("Expected 'POS' in column 2 of " << line);
		    if(tokens[2].compare("ID")!=0) THROW("Expected 'ID' in column 3 of " << line);
		    if(tokens[3].compare("REF")!=0) THROW("Expected 'REF' in column 4 of " << line);
		    if(tokens[4].compare("ALT")!=0) THROW("Expected 'ALT' in column 5 of " << line);
		    if(tokens[5].compare("QUAL")!=0) THROW("Expected 'QUAL' in column 6 of " << line);
		    if(tokens[6].compare("FILTER")!=0) THROW("Expected 'FILTER' in column 7 of " << line);
		    if(tokens[7].compare("INFO")!=0) THROW("Expected 'INFO' in column 8 of " << line);
		    if(tokens[8].compare("FORMAT")!=0) THROW("Expected 'FORMAT' in column 9 of " << line);

		    for(size_t i=0;i< headers.size();++i)
			{
			string::size_type j=headers[i].find("=");
			if(j!=string::npos)
			    {
			    string k=headers[i].substr(0,j);
			    string v=headers[i].substr(j+1);
			    ::xmlTextWriterStartElement(writer,BAD_CAST "meta");
			    ::xmlTextWriterWriteAttribute(writer,
				    BAD_CAST "key",
				    BAD_CAST k.c_str()
				);
			    ::xmlTextWriterWriteString(writer,BAD_CAST v.c_str());
			    ::xmlTextWriterEndElement(writer);
			    }
			else
			    {
			    SIMPLE_TAG("meta",headers[i].c_str());
			    }
			}
		    if(!meta_infos.empty())
			{
			xmlTextWriterStartElement(writer,BAD_CAST "infos");
			for(size_t i=0;i< meta_infos.size();++i)
			    {
			    meta_infos[i]->write(writer,"info");
			    delete meta_infos[i];
			    }
			::xmlTextWriterEndElement(writer);//meta
			}
		    if(!meta_fmt.empty())
			{
			xmlTextWriterStartElement(writer,BAD_CAST "formats");
			for(size_t i=0;i< meta_fmt.size();++i)
			    {
			    meta_fmt[i]->write(writer,"format");
			    delete meta_fmt[i];
			    }
			::xmlTextWriterEndElement(writer);//meta
			}

		    if(!meta_filter.empty())
			{
			xmlTextWriterStartElement(writer,BAD_CAST "filters");
			for(size_t i=0;i< meta_filter.size();++i)
			    {
			    meta_filter[i]->write(writer,"filter");
			    delete meta_filter[i];
			    }
			::xmlTextWriterEndElement(writer);//filters
			}


		    xmlTextWriterStartElement(writer,BAD_CAST "samples");
		    for(size_t i=9;i< tokens.size();++i)
			{
			samples_ids.push_back(tokens[i]);
			SIMPLE_TAG("sample",tokens[i].c_str());
			}
		    ::xmlTextWriterEndElement(writer);//samples

		    ::xmlTextWriterEndElement(writer);//head
		    ::xmlTextWriterStartElement(writer,BAD_CAST "body");
		    }
		else if(state==1 && !line.empty() && line[0]!='#')
		    {
		    ::xmlTextWriterStartElement(writer,BAD_CAST "variation");
		    tab.split(line,tokens);
		    if(tokens.size()!=9+samples_ids.size())
			{
			THROW("Expected " << (9+samples_ids.size()) << " in "<< line);
			}
		    //CHROM
		    SIMPLE_TAG("chrom",tokens[0].c_str());

		    //POS
		    SIMPLE_TAG("pos",tokens[1].c_str());

		    if(tokens[2].compare(".")!=0 && !tokens[2].empty())
			{
			SIMPLE_TAG("id",tokens[2].c_str());
			}
		    SIMPLE_TAG("ref",tokens[3].c_str());



		    if(tokens[4].compare(".")!=0 && !tokens[4].empty())
			{
			SIMPLE_TAG("alt",tokens[4].c_str());
			}
		    SIMPLE_TAG("qual",tokens[5].c_str());

		    if(tokens[6].compare(".")!=0 && !tokens[6].empty())
			{
			Tokenizer semicolon(';');
			vector<string> filters;
			semicolon.split(tokens[6],filters);

			::xmlTextWriterStartElement(writer,BAD_CAST"filters");
			for(size_t j=0;j< filters.size();++j)
			    {
			    if(filters[j].empty()) continue;
			    SIMPLE_TAG("filter",filters[j].c_str());
			    }
			::xmlTextWriterEndElement(writer);//filters
			}

		    ::xmlTextWriterStartElement(writer,BAD_CAST"infos");


		    //cerr << count_vcfrow << endl;

		    Tokenizer semicolon(';');
		    vector<string> infos;
		    semicolon.split(tokens[7],infos);
		    for(size_t j=0;j< infos.size();++j)
			{
			if(infos[j].empty()) continue;
			string::size_type eq=infos[j].find_first_of('=');
			 ::xmlTextWriterStartElement(writer,BAD_CAST"info");

			if(eq==std::string::npos)
			    {
			    ::xmlTextWriterWriteAttribute(writer,
				    BAD_CAST "prop",
				    BAD_CAST infos[j].c_str()
				);

			    }
			else
			    {
			    string k=infos[j].substr(0,eq);
			    string v=infos[j].substr(eq+1);
			    ::xmlTextWriterWriteAttribute(writer,
				BAD_CAST "key",
				BAD_CAST k.c_str()
				);
			    ::xmlTextWriterWriteString( writer,BAD_CAST v.c_str());
			    }
			::xmlTextWriterEndElement(writer);//INFO
			}

		    ::xmlTextWriterEndElement(writer);//INFOS

		    ::xmlTextWriterStartElement(writer,BAD_CAST "calls");
		    Tokenizer colon(':');
		    vector<string> format;

		    colon.split(tokens[8],format);
		    for(size_t j=0;j< samples_ids.size();++j)
			{
			vector<string> call;
			colon.split(tokens[9+j],call);
			if(call.size()!=format.size())
			    {
			    THROW("size(FORMAT)!=size(call) in "<< line);
			    }
			::xmlTextWriterStartElement(writer,BAD_CAST "call");
			::xmlTextWriterWriteAttribute(writer,
			    BAD_CAST "sample",
			    BAD_CAST samples_ids[j].c_str()
			    );
			for(size_t k=0;k< format.size();++k)
			    {
			    ::xmlTextWriterStartElement(writer,BAD_CAST "prop");
			    ::xmlTextWriterWriteAttribute(writer,
				    BAD_CAST "key",
				    BAD_CAST format[k].c_str()
				    );
			    ::xmlTextWriterWriteString(writer,BAD_CAST call[k].c_str());
			    ::xmlTextWriterEndElement(writer);//PROP
			    }
			 ::xmlTextWriterEndElement(writer);//CALL
			}
		    ::xmlTextWriterEndElement(writer);//CALLS
		    ::xmlTextWriterEndElement(writer);//variation
		    }
		else
		    {
		    THROW("BAD input "<< line);
		    }
		}

	    ::xmlTextWriterEndElement(writer);
	    ::xmlTextWriterEndDocument(writer);
	    ::xmlFreeTextWriter(writer);

	    }


	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Transforms a VCF to XML.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    //out << "Options:\n";
	    out << "(stdin|vcf|vcf.gz)\n\n";
	    }

	int main(int argc,char** argv)
		{
		LIBXML_TEST_VERSION
		int optind=1;

		while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			{
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind]<<"'\n";
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else
			{
			break;
			}
		    ++optind;
		    }

		if(optind==argc)
		    {
		    igzstreambuf buf;
		    istream in(&buf);
		    this->run(in);
		    }
		else if(optind+1==argc)
		    {
		    char* input=argv[optind++];
		    igzstreambuf buf(input);
		    istream in(&buf);
		    this->run(in);
		    buf.close();
		    }
		else
		    {
		    cerr << "Illegal number of arguments.\n";
		    return EXIT_FAILURE;
		    }
		xmlMemoryDump();
		return EXIT_SUCCESS;
		}
    };


int main(int argc,char** argv)
    {
    VcfToXml app;
    return app.main(argc,argv);
    }
