#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <set>
#include <cerrno>
#include <vector>
#include <fstream>
#include <cassert>
#include <map>
#include <algorithm>
using namespace std;

#define DEBUG(a) cerr << "[" << __LINE__ << "]" << a << endl;

class Methyl01
	{
	public:
		static char complementary(char c)
			{
			switch(c)
				{
				case 'A': return 'T';
				case 'T': return 'A';
				case 'G': return 'C';
				case 'C': return 'G';
				case 'Y': return 'R';
				case 'R': return 'Y';
				default:
					cerr << "Bad base \'" << c << "'" << endl;
					exit(EXIT_FAILURE);
					break;
				}
			}
	private:	
		class Reference
			{
			public:
				std::string ref_name;
				std::string sequence;
				
				size_t size() const
					{
					return sequence.size();
					}
				
				char at(size_t i) const
					{
					return sequence.at(i);
					}
			};
		
		
		class OrientedSequence
			{

			
			public:
				Reference* ref;
				OrientedSequence(Reference* ref):ref(ref)
					{
				
					}
				size_t size() const
					{
					return ref->size();
					}
				virtual char at(size_t i) const =0;
				virtual bool is_forward() const =0;
			};
		
		class ForwardSequence:public OrientedSequence
			{
			public:
				ForwardSequence(Reference* ref):OrientedSequence(ref)
					{
					}
				virtual char at(size_t i) const
					{
					return ref->at(i);
					}
				virtual bool is_forward() const
					{
					return true;
					}
			};
		
		class ReverseSequence:public OrientedSequence
			{
			public:
				ReverseSequence(Reference* ref):OrientedSequence(ref)
					{
					}
				virtual char at(size_t i) const
					{
					return Methyl01::complementary(ref->at(
						((ref->size()-1)-i)
						));
					}
				virtual bool is_forward() const
					{
					return false;
					}
			};
		
		
		struct Hit
			{
			size_t x;
			size_t y;
			int score;
			OrientedSequence* ref;
			};
		
		struct MethylBase
			{
			Reference* ref;
			size_t pos;
			char readBase;
			
			
			
			bool operator < (const MethylBase& other) const
				{
				if(ref!=other.ref) return ref < other.ref;
				if(pos!=other.pos) return pos< other.pos;
				return readBase < other.readBase;
				}
			bool operator == (const MethylBase& other) const
				{
				return	ref==other.ref &&
					pos==other.pos &&
					readBase==other.readBase
					;
				}
			};
		std::map<MethylBase,long> base2count;
		int min_score;
		bool show_align;
		vector<Reference*> references;
		vector<OrientedSequence*> orientedrefs;
		size_t width;
		size_t height;
		vector<int> matrix;
	public:
		Methyl01():min_score(10),show_align(true)
			{
			
			}
		~Methyl01()
			{
			
			}
		
		bool equals(char read,char ref)
			{
			switch(ref)
				{
				case 'G':
				case 'C':  
				case 'A': 
				case 'T': return read==ref;
				case 'Y': return read=='C'  || read=='T';
				case 'R': return read=='A'  || read=='G';
				default: return false;
				}
			}
#define MATRIX_INDEX(x,y) ((x)*this->height + y )
#define MATRIX(x,y)   this->matrix[MATRIX_INDEX(x,y)]
		
		void lcs(const string& a,OrientedSequence* seq,Hit* hit)
			    {
			 	
			    this->width = a.size()+1;
			    this->height = seq->size()+1;
			 

			   
			   
			    if(matrix.size() <  (this->width)*(this->height) )
			    	{
			    	this->matrix.resize((this->width)*(this->height),0);
			    	}
			    std::fill(matrix.begin(),matrix.end(),0);
			     
			   
			    for(size_t x=0;x < a.size();++x)
			    	{
				for (size_t y=0; y < seq->size(); ++y )
				    {
				    if(equals(a[x], seq->at(y)))
				    	{
				        MATRIX(x+1,y+1) = MATRIX(x,y) +1;
				        if(hit->score==-1 || hit->score <  MATRIX(x+1,y+1) )
				        	{
				        	hit->x=x;
				        	hit->y=y;
				        	hit->score= MATRIX(x+1,y+1);
				        	hit->ref=seq;
				        	}
				    	}
				    else
				    	{
				        MATRIX(x+1,y+1) = -1; //std::max(MATRIX(x+1,y), MATRIX(x,y+1)); 
				    	}
				     }
			         }
			    
			    }
		
#define DISPLAY(a) if(this->show_align) cout << a;		
		
		void scan(const string& name,const string& read)
			{
			Hit best;
			best.ref=NULL;
			best.score=-1;
			
			for(size_t j=0;j< orientedrefs.size();++j)
				{
				lcs(read,orientedrefs[j],&best);
				}
			if(best.score> this->min_score )
			    	{
			    	vector<MethylBase> candidates;
			    	
			    	DISPLAY("%READ: "); 
			    	
			    	for(int i=0; this->show_align &&  i < best.score ; ++i)
			    		{
			    		DISPLAY(read[best.x-i]);
			    		}
			    	DISPLAY(endl);
			    	DISPLAY("%REF : ");
			    	 
			    	for(int i=0; i < best.score ; ++i)
			    		{
			    		char base= best.ref->at(best.y-i);
			    		DISPLAY(base);
			    		if(base=='Y' || base=='R')
			    			{
			    			size_t real_base_index=best.y-i;
			    			if( !best.ref->is_forward())
			    				{
			    				real_base_index = (best.ref->size()-1)-real_base_index;
			    				}
			    			MethylBase mb;
			    			mb.ref = best.ref->ref;
			    			mb.pos = real_base_index;
			    			mb.readBase = read[best.x-i];
			    			if( !best.ref->is_forward())
			    				{
			    				mb.readBase = complementary(mb.readBase);
			    				}
			    			candidates.push_back(mb);
			    			}
			    		}
			    	DISPLAY(endl);

			    	for(size_t i=0;i< candidates.size();++i)
			    		{
			    		std::map<MethylBase,long>::iterator r= base2count.find(candidates[i]);
			    		if(r==base2count.end())
			    			{
			    			base2count.insert(make_pair<MethylBase,long>(candidates[i],1));
			    			}
			    		else
			    			{
			    			(*r).second++;
			    			}
			    		
			    		DISPLAY( name << "\t" );
			    		DISPLAY( candidates[i].ref->ref_name << "\t"
			    			<< candidates[i].pos << "\t"
			    			<< candidates[i].readBase
			    			<< endl );
			    		}
			    	DISPLAY(endl);
			    	
			    	}
			else
				{
				DISPLAY("[NO-HIT]" << name << "\t" << read << endl);
				}
			}
		
		void dump_count()
			{
			cout << "Sequence\tPOS\tBASE\tCOUNT\n";
			std::map<MethylBase,long>::iterator r= base2count.begin();
			while(r!=base2count.end())
				{
				cout << (*r).first.ref->ref_name << "\t"
					<< (*r).first.pos << "\t"
					<< (*r).first.readBase << "\t"
			    		<< (*r).second
			    		<< endl;
				++r;
				}
			}
		
		void load_reference(const char* ref)
			{
			Reference* curr=NULL;
			FILE* in=fopen(ref,"r");
			if(in==NULL)
				{
				cerr << "Cannot open "<< ref << " " <<strerror(errno) << endl;
				exit(EXIT_FAILURE);
				}
			int c;
			while((c=fgetc(in))!=EOF)
				{
				if(c=='\r') continue;
				
				
				if(c=='>')
					{
					curr=new Reference;
					references.push_back(curr);
					orientedrefs.push_back(new ForwardSequence(curr));
					orientedrefs.push_back(new ReverseSequence(curr));
					while((c=fgetc(in))!=EOF && c!='\n')
						{
						if(c=='\r') continue;
						curr->ref_name+=(char)c;
						}
					}
				else if(!isspace(c) && curr!=0)
					{
					curr->sequence+=(char)toupper(c);
					}
				}
			fclose(in);
			}
		void run(std::istream& in)
			{
			string name1;
			string read;
			string name2;
			string qual;
			while(getline(in,name1))
				{
				if(!getline(in,read)) break;
				if(!getline(in,name2)) break;
				if(!getline(in,qual)) break;
				scan(name1,read);
				}
			}
 		void usage(ostream& out,int argc,char **argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2013.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		
		    out << "Options:\n";
		    out << " -R <refernce> REQUIRED.\n";
		    out << " -n no display.\n";
		    }
		int main(int argc,char** argv)
			{
			int optind=1;
			while(optind < argc)
				{
				if(strcmp(argv[optind],"-h")==0)
					{
					usage(cout,argc,argv);
					return EXIT_FAILURE;
					}
				else if(strcmp(argv[optind],"-R")==0 && optind+1<argc)
					{
					load_reference(argv[++optind]);
					}
				else if(strcmp(argv[optind],"-n")==0)
					{
					this->show_align=!this->show_align;
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
			if(references.empty())
				{
				cerr << "Reference missing."<< endl;
				usage(cerr,argc,argv);
				return(EXIT_FAILURE);
				}
			if(optind==argc)
				{
				run(cin);
				}
			else while(optind < argc)
				{
				ifstream in(argv[optind++],ios::in);
				if(!in.is_open())
					{
					cerr << "Cannot open file:" <<strerror(errno) << endl;
					exit(EXIT_FAILURE);
					}
				run(in);
				in.close();
				}
			dump_count();
			return EXIT_SUCCESS;
			}
	};

int main(int argc,char** argv)
	{
	Methyl01 app;
	return app.main(argc,argv);
	}
