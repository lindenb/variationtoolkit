/*
 * shortreadsim.cpp
 *
 *  Created on: Nov 21, 2011
 *      Author: lindenb
 */
#include <map>
#include <vector>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <cerrno>
#include <fstream>
#include <algorithm>
#include <zlib.h>
#include "zstreambuf.h"
#include "segments.h"
#include "application.h"
#include "throw.h"
//#define NOWHERE
#include "where.h"
#include "tarball.h"
#include "numeric_cast.h"

using namespace std;


/* substitution */
class UserDefinedSubstitution
    {
    public:
	int32_t pos;
	char base1;
	char base2;
	std::string chrom;
    };

class SeqOut
    {
    public:
	FILE*  out;
	int32_t skip;
	int32_t length;
	SeqOut():out(NULL),skip(0),length(0)
	    {

	    }
	void put(char c)
	    {
	    if(length%80==0 && length!=0) fputc('\n',out);
	    fputc(c,out);
	    length++;
	    }
	void put(const char* s)
	    {
	    char* p=(char*)s;
	    while(*p!=0) {put(*p);p++;}
	    }
    };

class GenomeSim:public AbstractApplication
    {
    public:
	std::map<string,map<int32_t,UserDefinedSubstitution*>* > user_defined_mutations;



	double proba_mutation;
	double indel_fraction;
	double proba_extend;
	SeqOut seqout1;
	SeqOut seqout2;
	SeqOut* seqout[2];
	FILE* outvcf;
	Tar* tarball;
	std::map<string,vector<StartEnd>* > capture;
	std::map<string,vector<StartEnd>* > no_mutation;

	GenomeSim():
	    proba_mutation(0.0010),
	    indel_fraction(0.1),
	    proba_extend(0.3),
	    outvcf(NULL),
	    tarball(NULL)
	    {
	    seqout[0]=&seqout1;
	    seqout[1]=&seqout2;
	    }

	~GenomeSim()
	    {

	    }



	char anyOf(const char* s,int length)const
	    {
	    return s[rand()%length];
	    }

	char notIn(char c) const
	    {
	    switch(toupper(c))
		{
		case 'A': return anyOf("TGC",3);break;
		case 'T': return anyOf("AGC",3);break;
		case 'G': return anyOf("TAC",3);break;
		case 'C': return anyOf("TGA",3);break;
		default:break;
		}
	    return anyOf("TGAC",4);
	    }

	void copyTo(FILE* in,FILE* out)
	    {
	    errno=0;
	    rewind(in);
	    if(errno!=0)
		{
		THROW("Cannot rewind FILE*"<< strerror(errno));
		}
	    size_t nread=0;
	    int c;
	    while((c=fgetc(in))!=EOF)
		{
		if(nread!=0 && (nread%60)==0)
		    {
		    fputc('\n',out);
		    }
		fputc(c,out);
		++nread;
		}
	    fputc('\n',out);
	    fflush(out);
	    }

	void run(
		const std::string& chrom,
		const std::string& sequence,
		int32_t chromStart,
		int32_t chromEnd
		)
	    {
	    chromEnd=std::min(chromEnd,(int32_t)sequence.size());
	    if(chromStart>=chromEnd) return;

	    map<int32_t,UserDefinedSubstitution*>* pos2mut=NULL;
	    vector<StartEnd>* ignore_ranges=NULL;
	    std::map<string,map<int32_t,UserDefinedSubstitution*>* >::iterator r_mut=user_defined_mutations.find(chrom);
	    if(r_mut!=user_defined_mutations.end())
		{
		pos2mut = r_mut->second;
		}

	    map<string,vector<StartEnd>* >::const_iterator ri=no_mutation.find(chrom);
	    if(ri!=no_mutation.end())
		{
		ignore_ranges = ri->second;
		}


	    for(int i=0;i< 2;++i)
		{
		seqout[i]->length=0;
		seqout[i]->skip=0;
		fprintf(seqout[i]->out,">%s:%d-%d\n",chrom.c_str(),chromStart,chromEnd);
		}

	   for(;chromStart<chromEnd;++chromStart)
	       {
	       char c=sequence[chromStart];

	       if(seqout[0]->skip>0 || seqout[1]->skip>0)
		    {
		    for(int side=0;side<2;++side)
			{
			if(seqout[side]->skip>0)
			    {
			    seqout[side]->skip--;
			    }
			else
			    {
			    seqout[side]->put(c);
			    }
			}
		    continue;
		    }
	        if(pos2mut!=NULL)
	            {
		    map<int32_t,UserDefinedSubstitution*>::iterator r_user_mut=pos2mut->find(chromStart);
		    if(r_user_mut!=pos2mut->end())
			{
			fprintf(this->outvcf,"%s\t%d\t%c\t%c/%c\t1/1\n",chrom.c_str(),(chromStart+1),c,r_user_mut->second->base1,r_user_mut->second->base2);
			seqout[0]->put(r_user_mut->second->base1!='.'?r_user_mut->second->base1:c);
			seqout[1]->put(r_user_mut->second->base2!='.'?r_user_mut->second->base2:c);
			continue;
			}
	            }

	        if(ignore_ranges!=NULL)
	            {
	            size_t i=0;
	            for(i= 0; i< ignore_ranges->size();++i)
	        	{
	        	StartEnd& range=ignore_ranges->at(i);
	        	if(range.start<= chromStart && chromStart < range.end)
	        	    {
	        	    seqout[0]->put(c);
	        	    seqout[1]->put(c);
	        	    break;
	        	    }
	        	}
	            if(i!=ignore_ranges->size()) continue;
	            }



		if(c=='N' )
		    {
		    seqout[0]->put(c);
		    seqout[1]->put(c);
		    continue;
		    }

		//no mutation
		if(drand48()>= proba_mutation)
		    {
		    for(int side=0;side<2;++side)
			{
			seqout[side]->put(c);
			}
		    continue;
		    }
		int homologue=rand()%3;

		//substitution
		if(drand48()>= indel_fraction)
		    {
		    char base=notIn(c);
		    fprintf(this->outvcf,"%s\t%d\t%c\t%c\t",chrom.c_str(),(chromStart+1),c,base);
		    switch(homologue)
			{
			case 0:
				seqout[0]->put(base);
				seqout[1]->put(c);
				fputs("1/0\n",this->outvcf);
				break;
			case 1:
				seqout[0]->put(c);
			    	seqout[1]->put(base);
			    	fputs("0/1\n",this->outvcf);
				break;
			case 2:
				seqout[0]->put(base);
				seqout[1]->put(base);
				fputs("1/1\n",this->outvcf);
				break;
			default:break;
			}
		    continue;
		    }

		//create deletion
		if (drand48() < 0.5)
		    {
		    int n_skip=0;
		    do { n_skip++; } while(drand48() < proba_extend );
		    fprintf(this->outvcf,"%s\t%d\t%c\tdel.%d\t",chrom.c_str(),(chromStart+1),c,n_skip);
		    switch(homologue)
			{
			case 0:
				seqout[0]->skip=n_skip;
				seqout[1]->put(c);
				fputs("1/0\n",this->outvcf);
				break;
			case 1:
				seqout[0]->put(c);
				seqout[1]->skip=n_skip;
				fputs("0/1\n",this->outvcf);
				break;
			case 2:
				seqout[0]->skip=n_skip;
				seqout[1]->skip=n_skip;
				fputs("1/1\n",this->outvcf);
				break;
			default:break;
			}
		    }
		else //create insertion
		    {
		    string mut;
		    mut+=notIn(c);
		    do {
			mut+=anyOf("ATGC",4);
			} while(drand48() < proba_extend );
		    fprintf(this->outvcf,"%s\t%d\t%c\t%s\t",chrom.c_str(),(chromStart+1),c,mut.c_str());
		    switch(homologue)
			{
			case 0:
				seqout[0]->put(mut.c_str());
				seqout[0]->put(c);
				seqout[1]->put(c);
				fputs("1/0\n",this->outvcf);
				break;
			case 1:
				seqout[0]->put(c);
				seqout[1]->put(mut.c_str());
				seqout[1]->put(c);
				fputs("0/1\n",this->outvcf);
				break;
			case 2:
				seqout[0]->put(mut.c_str());
				seqout[1]->put(mut.c_str());
				seqout[0]->put(c);
				seqout[1]->put(c);
				fputs("1/1\n",this->outvcf);
				break;
			default:break;
			}

		    }
		}
	    fputc('\n',seqout[0]->out);
	    fputc('\n',seqout[1]->out);
	    }

	void run(const char* hg)
	    {
	    gzFile in=gzopen(hg,"r");
	    if(in==NULL)
		{
		THROW("Cannot open "<< hg << " "<< strerror(errno));
		}

	    string chrom;
	    string sequence;
	    sequence.reserve(300000000);//chr1
	    for(;;)
		{
		int c=gzgetc(in);
		if(c==EOF || c=='>')
		    {
		    if(!sequence.empty())
			{
			if(capture.empty())
			    {
			    run(chrom,sequence,0,sequence.size());
			    }
			else
			    {
			    map<string,vector<StartEnd>* >::iterator r=capture.find(chrom);
			    if(r!=capture.end())
				{
				//loop over the anges for this capture
				vector<StartEnd>* ranges= r->second;
				//loop over the exon capture
				for(vector<StartEnd>::iterator r2=ranges->begin();
					r2!=ranges->end();
					++r2)
				    {
				    run(chrom,sequence,r2->start,r2->end);
				    }
				}
			    }
			}
		    if(c==EOF)break;
		    chrom.clear();
		    sequence.clear();

		    while((c=gzgetc(in))!=EOF && c!='\n')
			{
			if(c=='\r') continue;
			chrom+=(char)c;
			}
		    continue;
		    }
		if(!isalpha(c))
		    {
		    continue;
		    }
		sequence+=(char)c;
		}
	    gzclose(in);
	    }


	void _readRanges(const char* filename,map<string,vector<StartEnd>* >& chrom2ranges)
	    {
	    Tokenizer tokenizer('\t');
	    vector<string> tokens;
	    string line;
	    igzstreambuf buf(filename);
	    istream in(&buf);
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line.at(0)=='#') continue;
		tokenizer.split(line,tokens);
		if(tokens.size()<3)
		    {
		    cerr << "Bad number of tokens  in "
			    << filename
			    << " : "
			    << line
			    ;
		    continue;
		    }
		int32_t chromStart;
		int32_t chromEnd;
		if(
			!numeric_cast<int32_t>(tokens[1].c_str(),&chromStart) ||
			!numeric_cast<int32_t>(tokens[2].c_str(),&chromEnd) ||
			chromEnd <chromStart)
		    {
		    cerr << "Bad range in " << line << endl;
		    continue;
		    }
		vector<StartEnd>* v=NULL;
		map<string,vector<StartEnd>* >::iterator r= chrom2ranges.find(tokens[0]);
		if(r== chrom2ranges.end())
		    {
		    v=new vector<StartEnd>;
		    chrom2ranges.insert(make_pair<string,vector<StartEnd>* >(tokens[0],v));
		    }
		else
		    {
		    v=r->second;
		    }
		StartEnd range(chromStart,chromEnd);
		v->push_back(range);
		}
	    buf.close();
	    }

	void readCapture(const char* filename)
	    {
	    _readRanges(filename,capture);
	    }

	void readIgnore(const char* filename)
	    {
	    _readRanges(filename,no_mutation);
	    }

	void readUserDefinedMutations(const char* filename)
	    {
	    Tokenizer tokenizer('\t');
	    vector<string> tokens;
	    string line;
	    igzstreambuf buf(filename);
	    istream in(&buf);
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line.at(0)=='#') continue;
		tokenizer.split(line,tokens);
		if(tokens.size()<4)
		    {
		    cerr << "Bad number of tokens  in "
			    << filename
			    << " : "
			    << line
			    ;
		    continue;
		    }
		int32_t pos;
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&pos) || pos<=0)
		    {
		    cerr << "Bad range in " << line << endl;
		    continue;
		    }
		if(tokens[2].size()!=1 || (!isalpha(tokens[2].at(0)) && tokens[2].at(0)!='.'))
		    {
		    cerr << "Bad base 1 in " << line << endl;
		    continue;
		    }
		if(tokens[3].size()!=1 || (!isalpha(tokens[3].at(0)) && tokens[3].at(0)!='.'))
		    {
		    cerr << "Bad base 2 in " << line << endl;
		    continue;
		    }




		UserDefinedSubstitution* mut=new UserDefinedSubstitution;
		mut->chrom.assign(tokens[0]);
		mut->pos=pos-1;
		mut->base1=tokens[2].at(0);
		mut->base2=tokens[3].at(0);
		_insert(mut);
		}
	    buf.close();
	    }



	void _insert(UserDefinedSubstitution* mut)
	    {
	    map<int32_t,UserDefinedSubstitution*>* pos2mut=NULL;
	    map<string,map<int32_t,UserDefinedSubstitution*>* >::iterator r= user_defined_mutations.find(mut->chrom);
	    if(r== user_defined_mutations.end())
		{
		pos2mut=new map<int32_t,UserDefinedSubstitution*>;
		user_defined_mutations.insert(make_pair<string,map<int32_t,UserDefinedSubstitution*>* >(mut->chrom,pos2mut));
		}
	    else
		{
		pos2mut=r->second;
		}


	    if(pos2mut->find(mut->pos)!=pos2mut->end())
		{
		THROW("error mutation at "<< mut->chrom<< ":"<< (mut->pos+1)<< "defined twice");
		}
	    pos2mut->insert(make_pair<int32_t,UserDefinedSubstitution*>(mut->pos,mut));
	    }

	void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] genome.fa"<< endl;
	    out << "Options:\n";
	    out << "  -o (file.tar) output tar file. contains chromosomes and mutations.\n";
	    out << "  -f (file) limit by genomic region (optional) read file:chrom(TAB)start(TAB)end\n";
	    out << "  -i (file) no mutation in those genomic regions (optional) read file:chrom(TAB)start(TAB)end\n";
	    out << "  -r (float) rate of mutations. default: "<< this->proba_mutation << "\n";
	    out << "  -R (float) fraction of indels default: "<< this->indel_fraction << "\n";
	    out << "  -X (float)  probability an indel is extended default: "<< this->indel_fraction << "\n";
	    out << " ** Inserting  User defined substitutions (optional):\n";
	    out << "    -u (filename) read a file containing user-defined mutations (optional). Format: (CHROM)\\t(POS+1)\\t(BASE1)\\t(BASE2)\n";
	    out << "    -m (chrom) (POS+1) (BASE1) (BASE2) insert user defined substitution. use dot('.') to not change the base.\n";
	    out << endl;

	    }

	int main(int argc,char** argv)
	    {
	    int optind=1;
	    const char* outfile=NULL;
	    srand(time(0));
	    srand48(time(0));
	    string tarbase("gsim");

	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-f")==0 && optind+1< argc)
		    {
		    readCapture(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-r")==0 && optind+1< argc)
		    {
		    this->proba_mutation=atof(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-R")==0 && optind+1< argc)
		    {
		    this->indel_fraction=atof(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-X")==0 && optind+1< argc)
		    {
		    this->proba_extend=atof(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-i")==0 && optind+1< argc)
		    {
		    readIgnore(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-u")==0 && optind+1< argc)
		    {
		    readUserDefinedMutations(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-m")==0 && optind+4< argc)
		    {
		    string chrom(argv[++optind]);
		    int32_t pos=atoi(argv[++optind]);
		    if(pos<1)
			{
			usage(cerr,argc,argv);
			cerr << "bad position in " << argv[optind] << endl;
			return EXIT_FAILURE;
			}
		    char* base1=argv[++optind];
		    if(strlen(base1)!=1 || (base1[0]!='.' && !isalpha(base1[0])))
			{
			usage(cerr,argc,argv);
			cerr << "bad base-1 in " << base1 << endl;
			return EXIT_FAILURE;
			}
		    char* base2=argv[++optind];
		    if(strlen(base2)!=1 || (base2[0]!='.' && !isalpha(base2[0])))
			{
			usage(cerr,argc,argv);
			cerr << "bad base-2 in " << base2 << endl;
			return EXIT_FAILURE;
			}
		    UserDefinedSubstitution* mut=new UserDefinedSubstitution;
		    mut->chrom.assign(chrom);
		    mut->pos=pos-1;
		    mut->base1=base1[0];
		    mut->base2=base2[0];
		    _insert(mut);
		    }
		else if(std::strcmp(argv[optind],"-o")==0 && optind+1< argc)
		    {
		    outfile=argv[++optind];
		    size_t slen=strlen(outfile);
		    if(slen<5 || strcmp(".tar",&outfile[slen-4])!=0)
			{
			cerr << "output file should end with tar."<< endl;
			return EXIT_FAILURE;
			}
		    const char* p=strrchr(outfile,'/');
		    if(p==NULL)
			{
			tarbase.assign(outfile);
			}
		    else
			{
			tarbase.assign(p+1);
			}
		    string::size_type d=tarbase.find_last_of('.');
		    if(d!=string::npos)
			{
			tarbase=tarbase.substr(0,d);
			}
		    }
		else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else
		    {
		    break;
		    }
		++optind;
		}

	    if(optind+1!=argc)
		{
		cerr << "Illegal number of arguments."<< endl;
		usage(cerr,argc,argv);
		return (EXIT_FAILURE);
		}


	    fstream* fout=NULL;
	    ostream* out=&cout;
	    if(outfile!=NULL)
		{
		fout=new fstream(outfile,ios::out);
		if(!fout->is_open())
		    {
		    cerr << "Cannot open outfile "<< outfile << ":" << strerror(errno)<< endl;
		    return EXIT_FAILURE;
		    }
		out=fout;
		}

	    this->tarball=new Tar(*out);

	    for(int side=0;side<2;++side)
		{
		this->seqout[side]->out=tmpfile();
		if(this->seqout[side]->out==NULL)
		    {
		    cerr << "Cannot open tmpFile for homologous " << (side+1)<< "/2" << strerror(errno)<< endl;
		    return EXIT_FAILURE;
		    }
		}

	    this->outvcf=tmpfile();
	    if(outvcf==NULL)
		{
		cerr << "Cannot open tmpFile for VCF:" << strerror(errno)<< endl;
		return EXIT_FAILURE;
		}

	    this->run(argv[optind]);


	    std::fflush(this->seqout[0]->out);
	    std::fflush(this->seqout[1]->out);
	    std::fflush(this->outvcf);

	    string tarpath;
	    tarpath.assign(tarbase).append("/homologous1.fa");
	    this->tarball->putFile(this->seqout[0]->out,tarpath.c_str());
	    tarpath.assign(tarbase).append("/homologous2.fa");
	    this->tarball->putFile(this->seqout[1]->out,tarpath.c_str());
	    tarpath.assign(tarbase).append("/mutations.txt");
	    this->tarball->putFile(this->outvcf,tarpath.c_str());
	    this->tarball->finish();

	    std::fclose(this->seqout[0]->out);
	    std::fclose(this->seqout[1]->out);
	    std::fclose(this->outvcf);

	    out->flush();
	    if(fout!=NULL)
		{
		fout->close();
		}

	    return EXIT_SUCCESS;
	    }
    };


int main(int argc, char** argv)
    {
    GenomeSim app;
    return app.main(argc,argv);
    }
