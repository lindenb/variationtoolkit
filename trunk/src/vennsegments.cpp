/*
 * vennsegments.cpp
 *
 *  Created on: Dec 16, 2011
 *      Author: lindenb
 */
#include <set>
#include <vector>
#include <cassert>
#include <memory>
#include <iostream>
#include <algorithm>
#include "tokenizer.h"
#include "segments.h"
#include "numeric_cast.h"
#include "throw.h"
#include "where.h"

using namespace std;


class VennSegment:public ChromStartEnd
    {
    public:
	std::set<std::string> samples;
	VennSegment(std::string s,int32_t b,int32_t e,const set<string>& samples):
	    ChromStartEnd(s,b,e),
	    samples(samples.begin(),samples.end())
	    {
	    }
	VennSegment(const VennSegment& cp):ChromStartEnd(cp),
		samples(cp.samples.begin(),cp.samples.end())
	    {
	    }

	~VennSegment()
	    {
	    }

	VennSegment& operator=(const VennSegment& cp)
	    {
	    if(this!=&cp)
		{
		ChromStartEnd::operator=(cp);
		samples.clear();
		samples.insert(cp.samples.begin(),cp.samples.end());
		}
	    return *this;
	    }

	auto_ptr<vector<VennSegment> > merge(const VennSegment& cp) const
	    {
	    auto_ptr<vector<VennSegment> > ret(NULL);
	    WHERE("");
	    if(chrom.compare(cp.chrom)!=0) return ret;
	    if(start > cp.start) return cp.merge(*this);

	    if(end <= cp.start) return ret;
	    if(start >= cp.end) return ret;

	    WHERE("");
	    set<string> union_samples;
	    union_samples.insert(samples.begin(),samples.end());
	    union_samples.insert(cp.samples.begin(),cp.samples.end());

	    WHERE("");

	    //same samples, overlaping regions: merge
	    /*
	    if(samples.size()==common_samples.size())
		{
		ret.reset(new vector<VennSegment>);
		ret->push_back(VennSegment(
		    this->chrom,
		    std::min(this->start,cp.start),
		    std::max(this->end,cp.end),
		    common_samples
		    ));
		return ret;
		}*/
	    WHERE("");
	    /**
	     *     ========
	     * =======
	     *
	     */
	    if(this->start <= cp.start &&
	       this->end <= cp.end )
		{
		ret.reset(new vector<VennSegment>);
		//left
		ret->push_back(VennSegment(
		    this->chrom,
		    this->start,
		    cp.start,
		    this->samples
		    ));
		//mid
		ret->push_back(VennSegment(
		    this->chrom,
		    cp.start,
		    this->end,
		    union_samples
		    ));
		//right
		ret->push_back(VennSegment(
		    this->chrom,
		    this->end,
		    cp.end,
		    cp.samples
		    ));
		return ret;
		}
	    /**
	     *     ========
	     * ================
	     *
	     */
	    if(this->start <= cp.start &&
	       this->end >= cp.end )
		{
		ret.reset(new vector<VennSegment>);
		//left
		ret->push_back(VennSegment(
		    this->chrom,
		    this->start,
		    cp.start,
		    this->samples
		    ));
		//mid
		ret->push_back(VennSegment(
		    this->chrom,
		    cp.start,
		    cp.end,
		    union_samples
		    ));
		//right
		ret->push_back(VennSegment(
		    this->chrom,
		    cp.end,
		    this->end,
		    cp.samples
		    ));
		return ret;
		}
	    WHERE("");
	    assert(0);
	    return ret;
	    }
    };

std::ostream& operator<<(ostream& out, const VennSegment& o)
    {
    out << o.chrom<<":" << o.start << "-"<< o.end << "(";
    for(set<string>::iterator rc=o.samples.begin();
	    rc!=o.samples.end();
	    ++rc)
	  {
	  if(rc!=o.samples.begin()) out << "|";
	  out << (*rc);
	  }
    out << ")";
    return out;
    }

class VennSegmentApp
    {
    public:
	 set<string> chromosomes;
	set<string> samples;
	vector<VennSegment> vennSegs;

	void process(vector<VennSegment>& data)
	    {
	    WHERE("");
	    bool done=false;
	    while(!done)
		{
		done=true;
		WHERE(done);
		std::size_t i=0;
		while(i< data.size())
		    {
		    WHERE("");
		    size_t j=i+1;
		    while(j< data.size())
			{
			WHERE("");
			auto_ptr<vector<VennSegment> >  buff= data[i].merge(data[j]);
			WHERE("");
			if(buff.get()==0)
			    {
			    ++j;
			    continue;
			    }
			WHERE("");
			 WHERE(j<<"/"<< data.size());
			for(size_t k=0;k<buff->size();)
			    {
			    if(buff->at(k).start >= buff->at(k).end)
				{
				buff->erase(buff->begin()+k);
				}
			    else
				{
				++k;
				}
			    }
			if(buff->empty())
			    {
			    ++j;
			    continue;
			    }
			WHERE("");
			cerr << "Change " << data[i]<< " vs "<< data[j] << " to ";
			data.erase(data.begin()+j);
			data.erase(data.begin()+i);
			for(size_t k=0;k<buff->size();++k)
			    {
			    cerr << buff->at(k) << " ";
			    data.push_back(buff->at(k));
			    }
			cerr << endl;
			WHERE("");
			done=false;
			break;
			}
		    if(!done) break;
		    ++i;
		    }
		}
	    WHERE("");
	    for(size_t i=0;i< data.size();++i)
		{
		cout	<< data[i].chrom
			<< "\t"
			<< data[i].start
			<< "\t"
			<< data[i].end
			<< "\t"
			<< data[i].samples.size()
			<< "\t"
			;
		  for(set<string>::iterator rc=data[i].samples.begin();
			rc!=data[i].samples.end();
			++rc)
		      {
		      if(rc!=data[i].samples.begin()) cout << "|";
		      cout << (*rc);
		      }
		cout << endl;
		}
	    WHERE("");
	    }

	void make()
	    {

	    for(set<string>::iterator rc=chromosomes.begin();
		rc!=chromosomes.end();
		++rc)
		{
		vector<VennSegment> data;
		for(vector<VennSegment>::iterator rv=vennSegs.begin();
		    rv!=vennSegs.end();
		    ++rv)
		    {
		    if(rc->compare(rv->chrom)!=0) continue;
		    data.push_back(VennSegment(*rv));
		    }

		process(data);
		}

	    }

	void run(std::istream& in)
	    {

	    Tokenizer tokenizer;
	    std::string line;
	    vector<string> tokens;
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		tokenizer.split(line,tokens);
		if(tokens.size()!=4) continue;
		int32_t start,end;
		chromosomes.insert(tokens[0]);
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&start)) continue;
		if(!numeric_cast<int32_t>(tokens[2].c_str(),&end)) continue;
		samples.insert(tokens[3]);
		set<string> one;
		one.insert(tokens[3]);
		vennSegs.push_back(VennSegment(tokens[0],start,end,one));
		}
	    }

	int main(int argc,char** argv)
	    {
	    run(cin);
	    make();
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    VennSegmentApp app;
    return app.main(argc,argv);
    }
