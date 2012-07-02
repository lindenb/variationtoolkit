/*
* virusassembler.cpp
*
*  Created on: Jun 29, 2012
*      Author: Pierre Lindenbaum PhD
*              plindenbaum@yahoo.fr
*              http://plindenbaum.blogspot.com
*
*/


#include <vector>
#include <fstream>
#include <algorithm>
#include <limits>
#include <stdint.h>
#include <cassert>
#include "sam.h"
#include "fastareader.h"
#include "bam1sequence.h"
#include "where.h"
#include "throw.h"
using namespace std;

typedef uint8_t strand_t;
typedef uint8_t chrom_t;
static const strand_t PLUS=0;
static const strand_t MINUS=1;

class VirusAssembler
    {
    public:

        struct Base
            {
            chrom_t chrom;
            strand_t strand;
            int32_t position;
            };
        struct Hit
            {
	    Bam1Record* record;
	    Base base;
            };

        std::vector<FastaSequence*> genome;
        std::vector<Bam1Record*> bam1records;
        std::vector<Base> bases;
        std::vector<Hit> hits;



        char antiparallele(char c) const
            {
            switch(c)
                {
                case 'A':case 'a': return 'T';
                case 'T':case 't': return 'a';
                case 'G':case 'g': return 'C';
                case 'C':case 'c': return 'c';
                default: return 'N';
                }
            }

        char charAt(const Base b,int32_t extend) const
            {
            if(b.chrom<0 || (int)b.chrom>=this->genome.size())
            	{
            	THROW("size:"<< this->genome.size() << " b.com=" <<  (int)b.chrom);
            	}
            const FastaSequence* chrom = this->genome.at(b.chrom);
            if(b.strand==PLUS)//forward
                {

                if(b.position+extend>=chrom->size()) return '\0';
                return toupper(chrom->at(b.position+extend));
                }
            else
                {

                if(extend > b.position) return '\0';

                return antiparallele(chrom->at(b.position-extend));
                }
            }

        void print(std::ostream& out,const Base& b,int len) const
            {
            out << b.position << " strand:"<< (int)b.strand << " chrom:"<< (int)b.chrom << " ";
            for(int i=0;i< len;++i)
                {
                char c=charAt(b,i);
                if(c=='\0') break;
                out << c;
                }
            }

        int compare(const Base& b1,const Base& b2) const
            {
            int32_t extend=0;
            for(;;)
                {
                int c1=this->charAt(b1,extend);
                int c2=this->charAt(b2,extend);
                if(c1!=c2) return c1-c2;
                if(c1=='\0') return 0;
                ++extend;
                }
            }

        struct BaseCmp
            {
            VirusAssembler* owner;
            BaseCmp(VirusAssembler* owner):owner(owner) {}
            bool operator() (const Base& b1,const Base& b2) const
                  {

                  return owner->compare(b1,b2)<0;
                  }
            };


        void build()
            {
		Base b;
            for(b.strand=PLUS;b.strand<=MINUS;++b.strand)
                {
                for(b.chrom=0;b.chrom< genome.size();++b.chrom)
                    {
                    FastaSequence* chromosome=genome[b.chrom];
		    for(b.position=0;b.position<chromosome->size();++b.position)
                        {
                        bases.push_back(b);
                        }
                    }
                }


            std::sort(
            	bases.begin(),
            	bases.end(),
            	BaseCmp(this)
            	);


            }

        /** count the number of mismatches between pos and seq */
        int32_t mismatches(
            const Base* pos,
            const Bam1Sequence* seq,
            int32_t begin_extend,
            int32_t end_extend
            ) const
            {
            int32_t count=0;
            int32_t extend=begin_extend;
            while(extend < end_extend && extend< seq->size())
              {
              int c1= charAt(*pos,extend);
     	      if(c1==0) break;
              int c2= seq->at(extend);
              if(c2==0) break;
              if(c1!=c2) ++count;
              extend++;
              }
            return count;
            }

        int compare(
            const Base* pos,
            const Bam1Sequence* seq,
            int max_extend
            ) const
            {
            assert(max_extend>0);
            assert(seq->size()>0);
            int32_t extend=0;

            while(  extend < max_extend &&
                    extend < seq->size())
	       {
	       int c1= charAt(*pos,extend);
	       int c2= seq->at(extend);

                int i=c1-c2;
                if(i!=0)
		        {
		        return i;
		        }
		if(c1=='\0') break;
                extend++;
                }
            return 0;
            }

        size_t lower_bound(const Bam1Sequence* sequence,int32_t max_extend) const
            {
            size_t beg=0L;
            size_t end=bases.size();
            size_t len=end-beg;
            while(len>0)
                {
                long half = len/2;
                long middle=beg+half;

                const Base& position=bases.at(middle);

                if(compare(&position,sequence,max_extend)<0)
                    {
                    beg = middle;
                    ++beg;
                    len = len - half - 1;
                    }
                else
                    {
                    len = half;
                    }
                }
            return beg;
            }

        void loadViralGenome(const char* fasta)
            {
            FastaReader reader;
            ifstream in(fasta,ios::in);
            if(!in.is_open())
                {
                return;
                }
            for(;;)
                {
                WHERE(genome.size());
                std::auto_ptr<FastaSequence> chrom=reader.next(in);
                if(chrom.get()==0) break;

                if(genome.size()>= numeric_limits<chrom_t>::max()) THROW("Too many fasta sequences.");
                genome.push_back(chrom.release());
                }
            in.close();

            build();

            }

        bool startsWith(
            const Base* pos,
            const Bam1Sequence* seq,
            int32_t maxLen
            ) const
            {
            return mismatches(pos,seq,0,maxLen)==0;
            }

        void align(const bam1_t *b)
            {
            int32_t max_extend=10;
            Bam1Sequence sequence(b);
            Bam1Record* bam1record=0;
            if(sequence.is_mapped() || sequence.is_mate_mapped()) return;
            size_t i=lower_bound(&sequence,max_extend);
            while(i< bases.size())
                {
                const Base& b=bases[i];
                if(!startsWith(&b,&sequence,max_extend))
                    {
                    break;
                    }
                if(bam1record==0)
                    {
                    bam1record=new Bam1Record(b);
                    bam1records.push_back(bam1record);
                    }
		print(cerr,b,max_extend);cerr << endl;
		for(int32_t j=0;j< max_extend && j< sequence.size();++j) cerr << sequence[j]; cerr << endl;
		Hit hit;
		hit.record=bam1record;
		hit.base=b;
		hits.push_back(hit);
                ++i;
                }
            }

        void scanPairedEnd(const char* filename)
            {
            WHERE(filename);
            samfile_t *fp_in = NULL;
            bam1_t *b=NULL;
            fp_in = samopen(filename, "rb", 0);
            if(NULL == fp_in)
                  {
                  fprintf(stdout,"Could not open file.\n");
                    return;
                  }
            b = bam_init1();
            while(samread(fp_in, b) > 0)
                {
                align(b);
                }
            bam_destroy1(b);
            samclose(fp_in);
            }
        int main(int argc,char** argv)
            {
            if(argc!=3) return -1;
            WHERE("");
            loadViralGenome(argv[1]);
            scanPairedEnd(argv[2]);
            return 0;
            }
    };

int main(int argc,char** argv)
    {
    VirusAssembler app;
    return app.main(argc,argv);
    }
