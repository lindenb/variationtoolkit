/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com

 * Motivation:
 *	Fraction of covered bases 

 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <set>
#include <limits>
#include <cassert>
#include <stdint.h>
#include "xbam2.h"
#include "sam.h"
#include "bam1sequence.h"
#include "throw.h"
#include "numeric_cast.h"
#include "tokenizer.h"

using namespace std;

class BamStats4
	{
	private:
		int chromStart;
		int chromEnd;
		vector<uint32_t> bases_per_bin;
		auto_ptr<uint32_t> max_qual;
		auto_ptr<uint32_t> min_qual;
		set<string> groupids;
		bool  discard_duplicates;
		uint32_t bin_step;
		uint32_t bin_max;
		bool cumulative;
		bool print_percentage;
		uint32_t previous_pos;
	public: 
		
		BamStats4():discard_duplicates(true),bin_step(10),bin_max(200),cumulative(false),print_percentage(false)
			{
			}
		
		 void fill_empty( uint32_t pos)
		 	{

		 	while(previous_pos<pos)
		 		{
		 		this->bases_per_bin[0]++;
		 		previous_pos++;
		 		}
		 	}
		    
		 int  scan_bed_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl)
		  	{
		  	if((int32_t)pos< this->chromStart || (int32_t)pos>=this->chromEnd) return 0;
			uint32_t coverage=0;
			fill_empty(pos);
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
				
                            if(!groupids.empty())
                            	{
                            	const char* g=b.get_aux_RG();
                            	if(g==NULL) continue;
                            	if(groupids.find(g)==groupids.end()) continue;
                            	}
			    coverage++;
			    }
			uint32_t cat=(uint32_t)coverage/this->bin_step;
			if(cat>=bases_per_bin.size()) cat=bases_per_bin.size()-1;
			if(cumulative)
				{
				for(uint32_t i=0;i<=cat;++i) this->bases_per_bin[i]++;
				}
			else
				{
				this->bases_per_bin[cat]++;
				}
			this->previous_pos=pos+1;
		  	return 0;
		  	}
		 
		 
		static int  static_scan_bed_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
		  	{
		  	return ((BamStats4*)data)->scan_bed_func(tid,pos,depth,pl);
		  	}
		    
		void scan(const char* bamFile,const char* bedfile)
			 {
			 Tokenizer tab;
			 vector<string> tokens;
			 uint32_t total_bases=0;
			 for(size_t i=0;i< bases_per_bin.size();++i) bases_per_bin[i]=0;
			 BamFile2* bf=new BamFile2(bamFile);
			 bf->open();
			 ifstream in(bedfile,ios::in);
			 if(!in.is_open())
			 	{
			 	cerr << "Cannot open " << bedfile << endl;
			 	exit(EXIT_FAILURE);
			 	}
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

				if(!(
				::numeric_cast<int>(tokens[1].c_str(),&chromStart) &&
				::numeric_cast<int>(tokens[2].c_str(),&chromEnd) &&
				 chromStart<=chromEnd ))
					{
					cerr << "Error, check the bed: "<< line << endl;
					exit(EXIT_FAILURE);
					}
				 int tid = bf->findTidByName(tokens[0].c_str());
				 if(tid<0)
				    	{
				    	cerr << "Error, check the chromosomes names "<< line << endl;
				    	exit(EXIT_FAILURE);
				    	}

				total_bases+=(chromEnd-chromStart);
				previous_pos=chromStart;
				bam_plbuf_t *buf; buf = bam_plbuf_init(static_scan_bed_func,(void*)this); // initialize pileup
    				bam_fetch(bf->bamPtr(), bf->bamIndex(), tid,chromStart,chromEnd, buf, BamFile2::fetch_func);
    				bam_plbuf_push(0, buf); // finalize pileup

    				fill_empty(chromEnd);
				}
			in.close();
			delete bf;
			

			
			cout << bamFile << "\t" << total_bases;
			 if(!groupids.empty())
			 	{
			 	cout << "\t";
			 	for(set<string>::iterator r=groupids.begin();r!=groupids.end();++r)
			 		{
			 		if(r!=groupids.begin()) cout << "|";
			 		cout << (*r);
			 		}
			 	}
			
			for(size_t i=0;i< this->bases_per_bin.size();++i)
				{
				uint32_t nbases=this->bases_per_bin[i];
				cout << "\t";
				if(print_percentage)
					{
					cout << (nbases/(double)total_bases)*100.0;
					}
				else
					{
					cout << nbases;
					}
				}
				
			cout << endl;
			for(size_t i=0;i< bases_per_bin.size();++i) bases_per_bin[i]=0;
			
			}
			
 		void usage(ostream& out,int argc,char **argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2013.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    out << "Histogram of bases covered." << endl;
		    out << "Usage:\n\t"<< argv[0] << " [options]  -b file.bed file1.bam file2.bam ... fileN.bam\n";
		    out << "Options:\n";
		    out << " -m <min-qual uint32> (optional) min SAM record Quality.\n";
		    out << " -M <max-qual uint32> (optional) max SAM record Quality.\n";
		    out << " -b <bedfile> required.\n";
		    out << " -g <groupid> optional.\n";
		    out << " -d do NOT dicard reads marked as duplicates." << endl;
		    out << " --bin-max (int) max observed depth default:" << bin_max << endl;
		    out << " --bin-step (int) category/depth step:" << bin_step  << endl;
		    out << " --cumulative (if coverage=100, then increase each bin from 0 to 100)" << endl;
		    out << " -p print percentage instead of count" << endl;
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
		       		else if(strcmp(argv[optind],"-p")==0)
					{
				        this->print_percentage=true;
		       			}
				else if(strcmp(argv[optind],"-g")==0 && optind+1<argc)
					{
					groupids.insert(argv[++optind]);
					}
				else if(strcmp(argv[optind],"--bin-step")==0 && optind+1<argc)
					{
					if(!numeric_cast<uint32_t>(argv[++optind],&bin_step) || bin_step<1)
					    {
					    cerr << "Bad value for --bin-step "<< argv[optind] << endl;
					    usage(cerr,argc,argv);
					    return EXIT_FAILURE;
					    }
					}
				else if(strcmp(argv[optind],"--bin-max")==0 && optind+1<argc)
					{
					if(!numeric_cast<uint32_t>(argv[++optind],&bin_step) || bin_max<1)
					    {
					    cerr << "Bad value for --bin-max "<< argv[optind] << endl;
					    usage(cerr,argc,argv);
					    return EXIT_FAILURE;
					    }
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
			
			for(uint32_t i=0; i <=bin_max;i+=bin_step)
				{
				bases_per_bin.push_back(0);
				}
				
			cout << "#filename\ttotal-bases";
			if(!groupids.empty())
				{
				cout << "\tgroup-id";
				}
			for(size_t i=0;i< bases_per_bin.size();++i)
				{
				cout << "\t[" << (i*bin_step);
				if(i+1==bases_per_bin.size())
					{
					cout << "-all[";
					}
				else
					{
					cout << "-" << ((i+1)*bin_step) << "[";
					}
				}
			cout << endl;
			
			while(optind<argc)
				{
				char* filebam=argv[optind++];
				scan(filebam,bedfile);
				}
				
			return EXIT_SUCCESS;
			}

	
	};
	
int main(int argc, char *argv[])
    {
    BamStats4 app;
    return app.main(argc,argv);
    }

