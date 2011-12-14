#include <sstream>
#include "knowngene.h"
#include "throw.h"
#include "tokenizer.h"
#include "numeric_cast.h"

using namespace std;

int32_t Segment::size() const
    {
    return end-start;
    }

bool Segment::contains(int32_t position) const
    {
    return start <=position && position< end;
    }

Segment::Segment():gene(NULL),index(-1),start(-1),end(-1)
    {

    }
Segment::~Segment()
    {
    }

bool Segment::isSplicing(int32_t position) const
    {
    return isSplicingAcceptor(position) || isSplicingDonor(position);
    }

Intron::Intron()
    {

    }

Intron::~Intron()
    {

    }

bool Intron::isSplicingAcceptor(int32_t position) const
    {
    if(!contains(position)) return false;
    if(gene->isForward())
	{
	return  (position==end-1) || (position==end-2);
	}
    else
	{
	return  position==start || position==start+1;
	}
    }

bool Intron::isSplicingDonor(int32_t position) const
    {
    if(!contains(position)) return false;
    if(gene->isForward())
	{
	return  position== start || position==start+1;
	}
    else
	{
	return  (position==end-1) ||  (position==end-2);
	}
    }

std::string Intron::name() const
    {
    ostringstream os;
    if(gene->isForward())
	    {
	    os << "Intron "<<(index+1);
	    }
    else
	    {
	    os << "Intron "<<(gene->countExons()-index);
	    }
    return os.str();
    }




Exon::Exon()
    {
    }

Exon::~Exon()
    {

    }

std::auto_ptr<Intron> Exon::getNextIntron() const
    {
    if(index+1 >= gene->countExons()) return std::auto_ptr<Intron>();
    return gene->intron(index);
    }

std::auto_ptr<Intron> Exon::getPrevIntron() const
    {
    if(index<=0) return std::auto_ptr<Intron>();
    return gene->intron(index-1);
    }

bool Exon::isSplicingAcceptor(int32_t position) const
    {
    if(!contains(position)) return false;
    if(gene->isForward())
	    {
	    if(index== 0) return false;
	    return position==start;
	    }
    else
	    {
	    if(index+1== gene->countExons()) return false;
	    return position==end-1;
	    }
    }

bool Exon::isSplicingDonor(int32_t position) const
    {
    if(!contains(position)) return false;
    if(gene->isForward())
	    {
	    if(index+1== gene->countExons()) return false;
	    return
		    (position==end-1) ||
		    (position==end-2) ||
		    (position==end-3)
		    ;
	    }
    else
	    {
	    if(index== 0) return false;
	    return  (position==start+0) ||
	    (position==start+1) ||
	    (position==start+2) ;
	    }

    }

std::string Exon::name() const
    {
    ostringstream os;
    if(gene->isForward())
	    {
	    os << "Exon "<<(index+1);
	    }
    else
	    {
	    os <<  "Exon "<<(gene->countExons()-index);
	    }
    return os.str();
    }


const Exon* KnownGene::exon(int32_t  idx) const
    {
    return &exons[idx];
    }

int32_t KnownGene::countExons() const
    {
    return (int32_t)exons.size();
    }

bool KnownGene::isForward() const
    {
    return strand=='+';
    }
std::auto_ptr<Intron> KnownGene::intron(int32_t  idx) const
    {
    Intron* i=new Intron;
    i->gene=this;
    i->index=idx;
    i->start = exon(idx)->end;
    i->end = exon(idx+1)->start;
    return std::auto_ptr<Intron>(i);
    }


int32_t KnownGene::getExonStart(int32_t  idx) const
	{
	return this->exon(idx)->start;
	}
int32_t KnownGene::getExonEnd(int32_t  idx) const
	{
	return this->exon(idx)->end;
	}

std::auto_ptr<std::string>
KnownGene::getExonNameFromGenomicIndex(int32_t genome) const
	{
	std::auto_ptr<std::string> p;
	for(int i=0;i< countExons();++i)
		{
		if(getExonStart(i)<=genome && genome< getExonEnd(i))
			{
			p.reset(new string(exon(i)->name()));
			break;
			}
		}
	return p;
	}


std::auto_ptr<KnownGene> KnownGene::parse(std::string line)
     {
     vector<string> tokens;
     Tokenizer tokenizer;

     tokenizer.split(line,tokens);
     if(tokens.size()<12)
	 {
	 THROW("Expected 12 columns in "<< line << " but got "<< tokens.size());
	 }
     KnownGene* g=new KnownGene;
     g->name=tokens[0];
     g->chrom=tokens[1];
     g->strand=tokens[2].at(0);
     if(!numeric_cast<int32_t>(tokens[3].c_str(),&(g->txStart)))
	 {
	 THROW("Bad txStart in "<< line);
	 }
     if(!numeric_cast<int32_t>(tokens[4].c_str(),&(g->txEnd)))
	 {
	 THROW("Bad txEnd in "<< line);
	 }
     if(!numeric_cast<int32_t>(tokens[5].c_str(),&(g->cdsStart)))
		 {
		 THROW("Bad cdsStart in "<< line);
		 }
     if(!numeric_cast<int32_t>(tokens[6].c_str(),&(g->cdsEnd)))
	 {
	 THROW("Bad cdsEnd in "<< line);
	 }
     int32_t exoncount;
     if(!numeric_cast<int32_t>(tokens[7].c_str(),&(exoncount)))
	 {
	 THROW("Bad exoncount in "<< line);
	 }
     Tokenizer comma(',');
     vector<string> exStart;
     vector<string> exEnd;
     comma.split(tokens[8],exStart);
     comma.split(tokens[9],exEnd);
     g->exons.reserve(exoncount);
     for(int32_t i=0;i< exoncount;++i)
	 {
	 Exon exon;
	 exon.gene=g;
	 exon.index=i;
	 if(!numeric_cast<int32_t>(exStart[i].c_str(),&(exon.start)))
	     {
	     THROW("Bad exon.start in "<< line);
	     }
	 if(!numeric_cast<int32_t>(exEnd[i].c_str(),&(exon.end)))
	     {
	     THROW("Bad exon.end in "<< line);
	     }
	 g->exons.push_back(exon);
	 }
     return std::auto_ptr<KnownGene>(g);
     }
