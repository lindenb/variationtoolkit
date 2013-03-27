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
#include <algorithm>
using namespace std;

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
				
				char at(size_t i,bool is_forward) const
					{
					if(is_forward)
						{
						return sequence.at(i);
						}
					else
						{
						return Methyl01::complementary(sequence.at(
							((sequence.size()-1)-i)
							));
						}
					}
			};
		struct Hit
			{
			size_t x;
			size_t y;
			int score;
			Reference* ref;
			bool is_forward;
			};
			
		vector<Reference*> references;
		size_t width;
		size_t height;
		vector<int> matrix;
	public:
		Methyl01()
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
		
		void lcs(const string& a,Reference* reference,bool is_forward,Hit* hit)
			    {
			 
			    this->width = a.size()+1;
			    this->height = reference->size()+1;
			 

			   
			   
			    if(matrix.size() <  (this->width)*(this->height) )
			    	{
			    	this->matrix.resize((this->width)*(this->height),0);
			    	}
			    std::fill(matrix.begin(),matrix.end(),0);
			     
			   
			    for(size_t x=0;x < a.size();++x)
			    	{
				for (size_t y=0; y < reference->size(); ++y )
				    {
				    if(equals(a[x], reference->at(y,is_forward)))
				    	{
				        MATRIX(x+1,y+1) = MATRIX(x,y) +1;
				        if(hit->score==-1 || hit->score <  MATRIX(x+1,y+1) )
				        	{
				        	hit->x=x;
				        	hit->y=y;
				        	hit->score= MATRIX(x+1,y+1);
				        	hit->ref=reference;
				        	hit->is_forward=is_forward;
				        	}
				    	}
				    else
				    	{
				        MATRIX(x+1,y+1) = -1; //std::max(MATRIX(x+1,y), MATRIX(x,y+1)); 
				    	}
				     }
			         }
			    
			    }
		
		struct MethylBase
			{
			Reference* ref;
			size_t pos;
			char readBase;
			};
		
		void scan(const string& name,const string& read)
			{
			Hit best;
			best.ref=NULL;
			best.score=-1;
			for(size_t j=0;j< references.size();++j)
				{
				lcs(read,references[j],true,&best);
				lcs(read,references[j],false,&best);
				}
			if(best.score>10 )
			    	{
			    	vector<MethylBase> candidates;
			    	
			    	cout << "%READ: "; 
			    	
			    	for(int i=0; i < best.score ; ++i)
			    		{
			    		cout << read[best.x-i];
			    		}
			    	cout << endl;
			    	cout << "%REF : "; 
			    	for(int i=0; i < best.score ; ++i)
			    		{
			    		char base= best.ref->at(best.y-i,  best.is_forward);
			    		cout << base;
			    		if(base=='Y' || base=='R')
			    			{
			    			size_t real_base_index=best.y-i;
			    			if( !best.is_forward)
			    				{
			    				real_base_index = (best.ref->size()-1)-real_base_index;
			    				}
			    			MethylBase mb;
			    			mb.ref = best.ref;
			    			mb.pos = real_base_index;
			    			mb.readBase = read[best.x-i];
			    			if( !best.is_forward)
			    				{
			    				mb.readBase = complementary(mb.readBase);
			    				}
			    			candidates.push_back(mb);
			    			}
			    		}
			    	cout << endl;

			    	for(size_t i=0;i< candidates.size();++i)
			    		{
			    		cout << name << "\t" ;
			    		cout << candidates[i].ref->ref_name << "\t"
			    			<< candidates[i].pos << "\t"
			    			<< candidates[i].readBase << "\t"
			    			<< endl;
			    		}
			    	cout << endl;
			    	
			    	}
			else
				{
				cout << "[NO-HIT]" << name << "\t" << read << endl;
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
			return EXIT_SUCCESS;
			}
	};

int main(int argc,char** argv)
	{
	Methyl01 app;
	return app.main(argc,argv);
	}
