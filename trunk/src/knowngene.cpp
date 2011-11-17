#include <sstream>
#include "knowngene.h"

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
