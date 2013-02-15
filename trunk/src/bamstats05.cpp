/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com

 * Motivation:
 *	Coverage per base.

 */
#include <sstream> 
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <limits>
#include <cassert>
#include <stdint.h>
#include "xbam2.h"
#include "sam.h"
#include "bam1sequence.h"
#include "throw.h"
#include "numeric_cast.h"
#include "tokenizer.h"
#include "where.h"

using namespace std;

class BamStats5
	{
	private:
		class Observed
			{
			public:
				std::string filename;
				auto_ptr<string> group;
			};
		
		class Base
			{
			public:
				int32_t chrom_idx;
				int32_t pos;
				vector<uint32_t> counts;
			};
		


		auto_ptr<uint32_t> max_qual;
		auto_ptr<uint32_t> min_qual;
		bool  discard_duplicates;
		vector<string> all_chromosomes;
		vector<Base*> bases_in_bed;
		vector<Observed*> observed;
		size_t base_index_begin;
		size_t base_index_end;
		bool split_by_group;

		int chromStart;
		int chromEnd;
		const char* current_group;
	public: 
		
		BamStats5():discard_duplicates(true),split_by_group(false)
			{
			}
		
		    
		 int  scan_bed_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl)
		  	{
		  	
		  	if((int32_t)pos< this->chromStart || (int32_t)pos>=this->chromEnd)
		  		{
		  		return 0;
		  		}
			uint32_t coverage=0;
			
			for(int d=0;d < depth;++d)
			    {
			   const bam_pileup1_t* curr=&pl[d];
			   const bam1_t* rec =curr->b;
			   Bam1Sequence b(rec);
			   
			   
			   if(this->min_qual.get()!=0 && b.quality() < (int)*(this->min_qual) )
				{
				continue;
				}
			   if(this->max_qual.get()!=0 && b.quality() > (int)*(this->max_qual) )
				{
				continue;
				}
			    if(this->discard_duplicates && b.is_duplicate()) continue;
				
                            if(current_group!=0)
                            	{
                            	const char* g=b.get_aux_RG();
                            	if(g==NULL || strcmp(current_group,g)!=0) continue;
                            	}
			    coverage++;
			    }
			
			this->bases_in_bed[this->base_index_begin + (pos-this->chromStart)]->counts.back()=coverage;
		  	return 0;
		  	}
		 
		 
		static int  static_scan_bed_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
		  	{
		  	return ((BamStats5*)data)->scan_bed_func(tid,pos,depth,pl);
		  	}
		 
		 
		static bool compare_base(const Base* g1,const Base* g2)
		    	{
		    	if(g1->chrom_idx !=  g2->chrom_idx)
		    		{
		    		return g1->chrom_idx <  g2->chrom_idx;
		    		}
		    	return g1->pos < g2->pos;
		    	}
		 
		void build_bed_array(const char* bedfile)
			{
			map<string,int32_t> chrom2idx;
			
			Tokenizer tab;
			ifstream in(bedfile,ios::in);
			 if(!in.is_open())
			 	{
			 	cerr << "Cannot open " << bedfile << endl;
			 	exit(EXIT_FAILURE);
			 	}
			 vector<string> tokens;
			 string line;
			 while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				if(line[0]=='#') continue;

				tab.split(line,tokens);
				if(tokens.size()<3)
				    {
				    cerr << "Bad bed line "<< line << endl;
				    continue;
				    }
				int32_t chrom_idx;
				map<string,int32_t>::iterator rc=chrom2idx.find(tokens[0]);
				if(rc==chrom2idx.end())
					{
					chrom_idx=all_chromosomes.size();
					all_chromosomes.push_back(tokens[0]);
					chrom2idx[tokens[0]]=chrom_idx;
					}
				else
					{
					chrom_idx=(*rc).second;
					}
				
				
				
				if(!(
					::numeric_cast<int>(tokens[1].c_str(),&chromStart) &&
					::numeric_cast<int>(tokens[2].c_str(),&chromEnd) &&
					 chromStart<=chromEnd ))
					{
					cerr << "Error, check the bed: "<< line << endl;
					exit(EXIT_FAILURE);
					}
				while(chromStart< chromEnd)
					{
					Base* base=new Base();
				
					base->chrom_idx=chrom_idx;
					base->pos = chromStart;
					bases_in_bed.push_back(base);
					++chromStart;
					}
				}
			
			in.close();

			::random_shuffle(bases_in_bed.begin(),bases_in_bed.end());

			::sort(bases_in_bed.begin(),bases_in_bed.end(),compare_base);

			} 
		void scan2(const char* bamFile, const char* groupid)
			{
			
			Observed* obs=new Observed;
			obs->filename.assign(bamFile);
			this->observed.push_back(obs);
			if(groupid==0)
				{
				obs->group.reset(0);
				this->current_group=0;
				}
			else
				{
				obs->group.reset(new string(groupid));
				this->current_group=groupid;
				}
			for(size_t i=0;i< bases_in_bed.size();++i)
				{
				bases_in_bed[i]->counts.push_back(0);
				}

			BamFile2* bf=new BamFile2(bamFile);
			bf->open();
			
			
			this->base_index_begin=0;
			
			
			while( this->base_index_begin < bases_in_bed.size())
				{
				int chromosome= bases_in_bed[this->base_index_begin]->chrom_idx ;
				this->chromStart = bases_in_bed[this->base_index_begin]->pos;
				this->chromEnd = this->chromStart+1;
				this->base_index_end = this->base_index_begin+1;
				while(this->base_index_end < bases_in_bed.size() )
					{
					
					Base* b=this->bases_in_bed[this->base_index_end];
					
					
					
					if(chromosome != b->chrom_idx )
						{
						break;
						}
					if(this->chromEnd != b->pos )
						{
						break;
						}
					this->chromEnd++;
					this->base_index_end++;
					}
				int tid = bf->findTidByName(this->all_chromosomes[chromosome].c_str());
				if(tid<0)
				    	{
				    	cerr << "Error, check the chromosomes names BED vs BAM: "<< chromosome << endl;
				    	exit(EXIT_FAILURE);
				    	}
				
				bam_plbuf_t *buf; buf = bam_plbuf_init(static_scan_bed_func,(void*)this); // initialize pileup
    				bam_fetch(bf->bamPtr(), bf->bamIndex(), tid,this->chromStart,this->chromEnd, buf, BamFile2::fetch_func);
    				bam_plbuf_push(0, buf); // finalize pileup
    				
				this->base_index_begin = this->base_index_end;
				}

			delete bf; 
			}
		 
		void scan(const char* bamFile)
			 {
			 if(this->split_by_group)
			 	{
			 	set<string> gids;
			 	 BamFile2* bf=new BamFile2(bamFile);
			 	bf->open();
				bam_header_t* header=bf->bamHeader();
				std::string query(header->text);
				std::istringstream in(query);
				string line;
				while(getline(in,line,'\n'))
				   {
				   if(line.compare(0, 4,"@RG\t")!=0) continue;
				   size_t j=line.find("\tID:");
				   if(j==string::npos) continue;
				   j+=4;
				   string group;
				   while(j< line.size() && line[j]!='\t')
				   	{
				   	group+=line[j++];
				   	}
				   if(group.empty()) continue;

				   gids.insert(group);
				   }
			
				delete bf;
				if(!gids.empty())
					{
					for(set<string>::iterator r=gids.begin();
						r!=gids.end();
						++r)
						{
						scan2(bamFile,(*r).c_str());
						}
					return;
					}
			 	}
			 scan2(bamFile,NULL);
			 }

 		void usage(ostream& out,int argc,char **argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2013.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    out << "Depth of each bases for muliple bam." << endl;
		    out << "Usage:\n\t"<< argv[0] << " [options]  -b file.bed file1.bam file2.bam ... fileN.bam\n";
		    out << "Options:\n";
		    out << " -m <min-qual uint32> (optional) min SAM record Quality.\n";
		    out << " -M <max-qual uint32> (optional) max SAM record Quality.\n";
		    out << " -b <bedfile> required.\n";
		    out << " -g split by group (optional).\n";
		    out << " -d do NOT dicard reads marked as duplicates." << endl;
		    }		
			
		int main(int argc, char *argv[])
			{
			
			char* bedfile=0;
			int optind=1;
			while(optind < argc)
				{
				if(strcmp(argv[optind],"-h")==0)
					{
					usage(cout,argc,argv);
					return EXIT_FAILURE;
					}
				else if(strcmp(argv[optind],"-b")==0 && optind+1<argc)
					{
					bedfile= argv[++optind];
					}
				else if(strcmp(argv[optind],"-d")==0)
					{
				        this->discard_duplicates=false;
		       			 }
				else if(strcmp(argv[optind],"-g")==0)
					{
					this->split_by_group=true;
					}
				else if(strcmp(argv[optind],"-m")==0 && optind+1<argc)
					{
					uint32_t q;
					if(!numeric_cast<uint32_t>(argv[++optind],&q))
					    {
					    cerr << "Bad value for -m "<< argv[optind] << endl;
					    usage(cerr,argc,argv);
					    return EXIT_FAILURE;
					    }
					this->min_qual.reset(new uint32_t(q));
					}
				else if(strcmp(argv[optind],"-M")==0 && optind+1<argc)
				    {
				    uint32_t q;
				    if(!numeric_cast<uint32_t>(argv[++optind],&q))
					{
					cerr << "Bad value for -M "<< argv[optind] << endl;
					usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				    this->max_qual.reset(new uint32_t(q));
				    }
				else if(strcmp(argv[optind],"--")==0)
					{
					++optind;
					break;
					}
				else if(argv[optind][0]=='-')
					{
					cerr << "unknown option '" << argv[optind]<< endl;
					usage(cerr,argc,argv);
					return(EXIT_FAILURE);
					}
				else
					{
					break;
					}
				++optind;
				}
			
			if(bedfile==0 || optind==argc)
				{
				cerr << "BED or BAM missing" << endl;
				usage(cerr,argc,argv);
				return(EXIT_FAILURE);
				}
			
			build_bed_array(bedfile);
			
			if(bases_in_bed.empty())
				{
				cerr << "Empty bed: "<< bedfile << endl;
				return(EXIT_FAILURE);
				}
			while(optind<argc)
				{
				char* filebam=argv[optind++];
				scan(filebam);
				}
			cout << "#CHROM\tPOS";
			for(size_t i=0;i< observed.size();++i)
				{
				cout << "\t" << observed[i]->filename;
				if(this->split_by_group)
					{
					cout << " : ";
					
					if(observed[i]->group.get()==0)
						{
						cout << "NULL";
						}
					else
						{
						cout << (*(observed[i]->group.get()));
						}
					}

				}
			cout << endl;
			for(size_t i=0;i< bases_in_bed.size();++i)
				{
				Base* b= bases_in_bed[i];
				cout << this->all_chromosomes[b->chrom_idx] << "\t" << b->pos;
				for(size_t j=0;j< b->counts.size();++j)
					{
					cout << "\t" << b->counts[j];
					}
				cout << endl;
				}			
			
			return EXIT_SUCCESS;
			}

	
	};
	
int main(int argc, char *argv[])
    {
    BamStats5 app;
    return app.main(argc,argv);
    }

