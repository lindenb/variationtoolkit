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
#include <stdint.h>
#include "sam.h"
#include "fastareader.h"
#include "bam1sequence.h"
using namespace std;

typedef uint8_t strand_t;
static const strand_t PLUS=0;
static const strand_t MINUS=1;

class VirusAssembler
	{
	public:

		class Base
			{
			public:
				uint8_t chrom;
				strand_t strand;
				int32_t position;
			};

		std::vector<FastaSequence*> genome;
		std::vector<Base> bases;

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

		int charAt(const Base b,int32_t extend) const
			{
			const FastaSequence* chrom = this->genome.at(b.chrom);
			if(b.strand==PLUS)//forward
				{
				if(b.position+extend>=chrom->size()) return -1;
				return toupper(chrom->at(b.position+extend));
				}
			else
				{
				if(extend > b.position) return -1;
				return antiparallele(chrom->at(b.position-extend));
				}
			}

		int compare(const Base& b1,const Base& b2) const
			{
			int32_t extend=0;
			for(;;)
				{
				int c1=this->charAt(b1,extend);
				int c2=this->charAt(b2,extend);
				if(c1==-1 && c2==-1) return 0;
				if(c1==-1) return -1;
				if(c2==-1) return 1;
				int cmp=c1-c2;
				if(cmp!=0) return cmp;
				++extend;
				}
			}

		struct BaseCmp
			{
			VirusAssembler* owner;
  			bool operator() (const Base& b1,const Base& b2)
  				{
  				return owner->compare(b1,b2);
  				}
			};


		void build()
			{
			BaseCmp comparator;
			Base b;
			comparator.owner=this;
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
			std::sort(bases.begin(),bases.end(),comparator);
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
			  if(c1==-1) break;
			  int c2= seq->at(extend);
			  if(c1==c2) ++count;
			  extend++;
			  }
		     return 0;
		     }

		int compare(
			const Base* pos,
			const Bam1Sequence* seq,
			int max_extend
			) const
		     {
		     int32_t extend=0;

		     while(	extend < max_extend &&
		     		extend< seq->size())
			  {
			  int c1= charAt(*pos,extend);
			  if(c1==-1) return -1;
			  int c2= seq->at(extend);

			    int i=c1-c2;
			    if(i!=0)
				{
				return i;
				}
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
			for(;;)
				{
				std::auto_ptr<FastaSequence>  chrom=reader.next(in);
				if(chrom.get()==0) break;
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
			size_t i=lower_bound(&sequence,max_extend);
			while(i< bases.size())
				{
				const Base& b=bases[i];
				if(!startsWith(&b,&sequence,max_extend))
					{
					break;
					}

				++i;
				}
			}

		void scanPairedEnd(const char* filename)
			{
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
