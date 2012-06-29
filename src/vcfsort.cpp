/*
 * prediction.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <string>
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <memory>
#include <fstream>
#include <stdint.h>
#include "tokenizer.h"
#include "numeric_cast.h"
//#define NOWHERE
#include "throw.h"

using namespace std;

typedef int32_t chrom_id;
typedef int32_t bases_id;

class VcfSort
    {
    public:
	Tokenizer delim;
	int chromColumn;
	int posColumn;
	int refColumn;
	int altColumn;
	bool uniq_flag;
	vector<std::string > id2chrom;
	map<std::string,chrom_id> chrom2id;
	vector<std::string > id2bases;
	map<std::string,bases_id> bases2id;


	struct Row
	    {
		chrom_id tid;
		int32_t pos;
		bases_id ref;
		bases_id alt;
		uint32_t len;
		long offset;
		VcfSort*  owner;
		bool operator < (const Row& cp) const
			{
			return this->owner->compare(this,&cp)<0;
			}
		bool operator==(const Row& cp) const
			{
			return this->owner->compare(this,&cp)==0;
			}

	    };





	VcfSort():delim('\t'),
		chromColumn(0),
		posColumn(1),
		refColumn(-1),
		altColumn(-1),
		uniq_flag(false)
	    {
	    }
	virtual ~VcfSort()
	    {
	    }

	const char* getSeqById(bases_id id) const
	    {
	    //if(id<0 || id>= (int)id2bases.size()) return 0;
	    return id2bases[(size_t)id].c_str();
	    }

	const char* getChromById(bases_id id) const
	    {
	    //if(id<0 || id>= (int)id2chrom.size()) return 0;
	    return id2chrom[(size_t)id].c_str();
	    }

	int compareNoOffset(const Row* a,const  Row* b)
	    {
	    const char* ca= getChromById(a->tid);
	    const char* cb= getChromById(b->tid);
	    int32_t i= strcasecmp(ca,cb);
	    if(i!=0) return i;
	    i= ( a->pos - b->pos);
	    if(i!=0) return i;
	    if(refColumn!=-1)
		{
		const char* refa= getSeqById(a->ref);
		const char* refb= getSeqById(b->ref);
		i= strcasecmp(refa,refb);
		if(i!=0) return i;
		}
	    if(altColumn!=-1)
		{
		const char* alta= getSeqById(a->alt);
		const char* altb= getSeqById(b->alt);
		i= strcasecmp(alta,altb);
		return i;
		}
	    return 0;
	    }


	int compare(const Row* a,const  Row* b)
	    {
	    int32_t i= compareNoOffset(a,b);
	    if(i!=0) return i;
	    long j=a->offset - b->offset;
	    return (j<0?-1:j>0?1:0);
	    }


#define CHECKCOL(a) if(a>=(int)tokens.size()){\
		cerr << "Column "<< #a << " out of range in "<< line << endl;\
		continue;}



	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    FILE* tmpFile=0;
	    long offset=0L;
	    std::set<Row> rows;
	    tmpFile=tmpfile();
	    if(tmpFile==0)
		{
		THROW("Cannot create tmp File");
		}
	    while(getline(in,line,'\n'))
		    {
		    if(line.empty()) continue;
		    if(line[0]=='#')
			{
			cout << line << endl;
			continue;
			}
		    Row row;
		    row.owner=this;

		    delim.split(line,tokens);

		    if(chromColumn>=(int)tokens.size())
			{
			cerr << "CHROM out of range in "<< line << endl;
			cout << line << endl;
			continue;
			}
		    map<std::string,chrom_id>::iterator rc= chrom2id.find(tokens[chromColumn]);
		    if(rc==chrom2id.end())
			{
			chrom2id.insert(make_pair(tokens[chromColumn],id2chrom.size()));
			row.tid=(chrom_id)id2chrom.size();
			id2chrom.push_back(tokens[chromColumn]);

			}
		    else
			{
			row.tid=rc->second;
			}

		    if(posColumn>=(int)tokens.size())
			{
			cerr << "POS out of range in "<< line << endl;
			cout << line << endl;
			continue;
			}
		    if(!numeric_cast(tokens[posColumn].c_str(),&(row.pos)))
			{
			cerr << "bad POSin "<< line << endl;
			cout << line << endl;
			continue;
			}

		    if(refColumn!=-1)
			{
			if(refColumn>=(int)tokens.size())
			    {
			    cerr << "REF out of range in "<< line << endl;
			    cout << line << endl;
			    continue;
			    }
			string seq(tokens[refColumn]);
			std::transform(seq.begin(),seq.end(),seq.begin(),(int(*)(int)) toupper);
			map<std::string,bases_id>::iterator rc= bases2id.find(seq);
			if(rc==bases2id.end())
			    {
			    bases2id.insert(make_pair(seq,(bases_id)id2bases.size()));
			    row.ref=(bases_id)id2bases.size();
			    id2bases.push_back(seq);
			    }
			else
			    {
			    row.ref=rc->second;
			    }
			}
		    else
			{
			row.ref=0;
			}

		    if(altColumn!=-1)
			{
			if(altColumn>=(int)tokens.size())
			    {
			    cerr << "ALT out of range in "<< line << endl;
			    cout << line << endl;
			    continue;
			    }
			string seq(tokens[altColumn]);
			std::transform(seq.begin(),seq.end(),seq.begin(),(int(*)(int)) toupper);
			map<std::string,bases_id>::iterator rc= bases2id.find(seq);
			if(rc==bases2id.end())
			    {
			    bases2id.insert(make_pair(seq,(bases_id)id2bases.size()));
			    row.alt=(bases_id)id2bases.size();
			    id2bases.push_back(seq);
			    }
			else
			    {
			    row.alt=rc->second;
			    }
			}
		    else
			{
			row.alt=0;
			}
		    row.offset=offset;
		    row.len=line.size();
		    if(fwrite((const void*)line.data(),sizeof(char),row.len,tmpFile)!=row.len)
			{
			fclose(tmpFile);
			THROW("Cannot write "<< row.len << " bytes ");
			}
		    rows.insert(row);
		    offset+=(long)line.size();
		    }
	    char* buffer=0;
	    for(set<Row>::iterator r=rows.begin();
		    r!=rows.end();
		    ++r
		    )
		{
		if(uniq_flag)
		    {
		    set<Row>::iterator r_next(r);
		    ++r_next;
		    if(r_next!=rows.end() && compareNoOffset(&(*r),&(*r_next))==0)
			{
			continue;
			}
		    }
		if(fseek(tmpFile,r->offset,SEEK_SET)!=0)
		    {
		    fclose(tmpFile);
		    THROW("Cannot fseek");
		    }
		buffer=(char*)realloc(buffer,sizeof(char)*(r->len));
		if(buffer==0)
		    {
		    fclose(tmpFile);
		    THROW("Out of memory");
		    }
		if(fread((void*)buffer,sizeof(char),r->len,tmpFile)!=r->len)
		    {
		    fclose(tmpFile);
		    THROW("Cannot read "<< r->len << " bytes ");
		    }
		cout.write(buffer,r->len);
		cout << endl;
		}
	    free(buffer);

	    fclose(tmpFile);
	    }


	virtual void usage(ostream& out,int argc,char** argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  -c <CHROM col> (default:"<< (1+chromColumn) <<")" << endl;
		out << "  -p <POS col> (default:"<< (1+posColumn) <<")" << endl;
		out << "  -r <REF col> OPTIONAL" << endl;
		out << "  -a <ALT col> OPTIONAL" << endl;
		out << "  -u uniq" << endl;
		}
    };

#define ARGVCOL(flag,var) else if(std::strcmp(argv[optind],flag)==0 && optind+1<argc)\
	{\
	if(!numeric_cast<int>(argv[++optind],&(app.var)) || app.var < 1) \
		{cerr << "Bad column for "<< flag << ":" << argv[optind]<< ".\n";app.usage(cerr,argc,argv);return EXIT_FAILURE;}\
	app.var--;\
	}


int main(int argc,char** argv)
    {
    VcfSort app;
    int optind=1;
    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
			{
			app.usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		ARGVCOL("-c",chromColumn)
		ARGVCOL("-p",posColumn)
		ARGVCOL("-r",refColumn)
		ARGVCOL("-a",altColumn)
		else if(strcmp(argv[optind],"-u")==0)
		    {
		    app.uniq_flag=true;
		    }
		else if(strcmp(argv[optind],"--")==0)
			{
			++optind;
			break;
			}
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '" << argv[optind]<< "'" << endl;
			app.usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		else
			{
			break;
			}
		++optind;
		}


    if(optind==argc)
		{
		app.run(cin);
		}
    else if(optind+1==argc)
		{
		ifstream in(argv[optind],ios::in);
		if(!in.is_open())
		    {
		    cerr << "Cannot open "<< argv[optind] << endl;
		    return EXIT_FAILURE;
		    }
		app.run(in);
		in.close();
		++optind;
		}
    else
	{
	cerr << "Illegal number of arguments."<< endl;
	return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
    }
