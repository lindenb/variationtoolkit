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
#include <zlib.h>
#include "zstreambuf.h"
#include "segments.h"
#include "application.h"
#include "throw.h"
#include "where.h"
#include "tarball.h"

using namespace std;

/* variation type */
enum VarType
    {
    SUBST,
    DEL,
    INS
    };

/* abstract mutation */
class Mutation
    {
    public:
	int32_t pos;
	Mutation() {}
	Mutation(int32_t pos):pos(pos) {}
	virtual ~Mutation() {}
	virtual VarType type() const=0;
	virtual std::string toString() const=0;
    };

/* substitution */
class Substitution:public Mutation
    {
    public:
	char base;
	Substitution() {}
	Substitution(int32_t pos,char base):Mutation(pos),base(base) {}
	virtual ~Substitution() {}
	virtual VarType type() const { return SUBST;}
	virtual std::string toString() const
	    {
	    ostringstream os;
	    os << "subst:("<< base << ") at " << pos;
	    return os.str();
	    }
    };

/* deletion */
class Deletion:public Mutation
    {
    public:
	int32_t length;
	Deletion():length(1) {}
	virtual ~Deletion() {}
	virtual VarType type() const { return DEL;}
	virtual std::string toString() const
	    {
	    ostringstream os;
	    os << "del:("<< length << ") at " << pos;
	    return os.str();
	    }
    };

/* insertion */
class Insertion:public Mutation
    {
    public:
	std::string sequence;
	Insertion() {}
	virtual ~Insertion() {}
	virtual VarType type() const { return INS;}
	virtual std::string toString() const
	    {
	    ostringstream os;
	    os << "ins:("<< sequence << ") at " << pos;
	    return os.str();
	    }
    };


/* substitution */
class UserDefinedSubstitution:public Substitution
    {
    public:
	char base2;
	std::string chrom;
	UserDefinedSubstitution() {}
	virtual ~UserDefinedSubstitution() {}
    };

class ShortReadSim:public AbstractApplication
    {
    public:
	std::map<string,vector<StartEnd> > capture;
	std::map<string,map<int32_t,UserDefinedSubstitution*> > user_defined_mutations;
	uint64_t total_num_pairs;
	int32_t paired_end_size;
	int32_t paired_end_stddev;
	int32_t short_read_length;
	double base_error_rate;
	double proba_mutation;
	double indel_fraction;
	double proba_extend;
	FILE* outfq1;
	FILE* outfq2;
	FILE* outvcf;
	char default_quality;
	uint64_t pair_id_generator;
	Tar* tarball;


	ShortReadSim():
	    total_num_pairs(10),
	    paired_end_size(500),
	    paired_end_stddev(50),
	    short_read_length(70),
	    base_error_rate(0.020),
	    proba_mutation(0.0010),
	    indel_fraction(0.1),
	    proba_extend(0.3),
	    outfq1(NULL),
	    outfq2(NULL),
	    outvcf(NULL),
	    default_quality('2'),
	    pair_id_generator(0),
	    tarball(NULL)
	    {

	    }

	~ShortReadSim()
	    {

	    }

	char complement(char c) const
	    {
	    switch(c)
		    {
		    case 'a': case 'A': return 'T';
		    case 't': case 'T': return 'A';
		    case 'c': case 'C': return 'G';
		    case 'g': case 'G': return 'C';
		    case 'n': case 'N': return 'N';
		    default: cerr << "??" << c << (int)c<< endl;return 'N';
		    }
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

	void emit(const string& chrom,const string& amplicon)
	    {
	    ++pair_id_generator;
	    //first read
	    fputc('@',outfq1);
	    fputs(chrom.c_str(),outfq1);
	    fprintf(outfq1,":%ld:1\n",pair_id_generator);

	    for(int32_t i=0;i< short_read_length;++i)
		{
		fputc(amplicon[i],outfq1);
		}
	    fputs("\n+\n",outfq1);
	    for(int32_t i=0;i< short_read_length;++i)
		{
		fputc(default_quality,outfq1);
		}
	    fputs("\n",outfq1);

	    //second read

	    fputs("@",outfq2);
	    fputs(chrom.c_str(),outfq2);
	    fprintf(outfq2,":%ld:2\n",pair_id_generator);
	    for(int32_t i=0;i< short_read_length;++i)
		{
		fputc(amplicon[amplicon.size()-short_read_length+i],outfq2);
		}
	    fputs("\n+\n",outfq2);
	    for(int32_t i=0;i< short_read_length;++i)
		{
		fputc(default_quality,outfq2);
		}
	    fputs("\n",outfq2);
	    }

	void run(const char* hg)
	    {
	    long genome_size=0L;
	    string chrom;
	    int32_t length=0;
	    map<string,int32_t> chrom2size;
	    gzFile in=gzopen(hg,"r");
	    if(in==NULL)
		{
		THROW("Cannot open "<< hg << " "
			<< strerror(errno)
		);
		}

	    for(;;)
		{
		int c=gzgetc(in);
		if(c==EOF || c=='>')
		    {
		    if(length>0)
			{
			WHERE("Found "<< chrom << " size:"<< length);
			chrom2size.insert(make_pair<string,int32_t>(chrom,length));
			genome_size+=length;
			}
		    chrom.clear();
		    length=0;
		    if(c==EOF) break;

		    while((c=gzgetc(in))!=-1 && c!='\n')
			{
			if(c!='\r') chrom+=(char)c;
			}
		    continue;
		    }
		else if(std::isalpha(c))
		    {
		    length++;
		    }
		}
	    if(gzrewind(in)==-1)
		{
		THROW("Cannot rewind "<< hg);
		}


	    chrom.clear();
	    string sequence;
	    for(;;)
		{
		int c=gzgetc(in);
		if(c==EOF || c=='>')
		    {
		    if(!sequence.empty())
			{
			WHERE("found "<< chrom << " " << sequence.size());
			vector<bool> mask;
			//resize the mask: if no capture, allow everywhere
			mask.resize(sequence.size(),capture.empty());
			//get the capture for this chromosome
			map<string,vector<StartEnd> >::iterator r=capture.find(chrom);
			if(r!=capture.end())
			    {
			    //loop over the anges for this capture
			    vector<StartEnd>& ranges= r->second;
			    //loop over the exon capture
			    for(vector<StartEnd>::iterator r2=ranges.begin();
				    r2!=ranges.end();
				    ++r2)
				{
				int32_t chromStart= r2->start;
				while(chromStart<= r2->end && chromStart< (int32_t)sequence.size())
				    {
				    mask[chromStart]=true;
				    chromStart++;
				    }
				}
			    }
			//generate the mutations
			vector<Mutation*> all_mutations;
			map<int32_t,UserDefinedSubstitution*>& chrom_user_def_mut=this->user_defined_mutations[chrom];
			map<int32_t,Mutation*> chrom1;
			map<int32_t,Mutation*> chrom2;
			for(int32_t i=0;i< (int32_t)sequence.size();++i)
			    {
			    /* is there a user-defined mutation here ? */
			    map<int32_t,UserDefinedSubstitution*>::iterator r2=chrom_user_def_mut.find(i);
			    if(r2!=chrom_user_def_mut.end())
				{
				WHERE("Inserting custom mutation");
				fprintf(this->outvcf,"%s\t%d\t%c\t%c\n",chrom.c_str(),(i+1),r2->second->base,r2->second->base2);
				all_mutations.push_back(new Substitution(i,r2->second->base));
				chrom1.insert(make_pair<int32_t,Mutation*>(i,all_mutations.back()));
				all_mutations.push_back(new Substitution(i,r2->second->base2));
				chrom2.insert(make_pair<int32_t,Mutation*>(i,all_mutations.back()));
				continue;
				}
			    /* random : is there a mutation here ? */
			    if(drand48()>= proba_mutation) continue;
			    if(!mask[i]) continue;
			    Mutation* mutation= NULL;
			    if(drand48()>= indel_fraction)
				{
				Substitution* subst=new Substitution;
				subst->base=anyOf("ATGC",4);
				fprintf(this->outvcf,
					"%s\t%d\t%c\t%c\n",
					chrom.c_str(),
					(i+1),
					sequence.at(i),
					r2->second->base
					);
				mutation=subst;
				}
			    //create deletion
			    else if (drand48() < 0.5)
				{
				Deletion* deletion=new Deletion;
				while(drand48() < proba_extend) deletion->length++;
				fprintf(this->outvcf,
				    "%s\t%d\t%c\td%d\n",
				    chrom.c_str(),
				    (i+1),
				    sequence.at(i),
				    deletion->length
				    );
				mutation=deletion;
				}
			    //create insertion
			    else
				{
				Insertion* insertion=new Insertion;
				insertion->sequence+=notIn(sequence.at(i));
				while(drand48() < proba_extend) insertion->sequence+=anyOf("ATGC",4);
				insertion->sequence+=sequence.at(i);
				fprintf(this->outvcf,
				    "%s\t%d\t%c\t%s\n",
				    chrom.c_str(),
				    (i+1),
				    sequence.at(i),
				    insertion->sequence.c_str()
				    );
				mutation=insertion;
				}
			    mutation->pos=i;
			    all_mutations.push_back(mutation);
			    int where=rand()%3;
			    if(where==0 || where==2)
				{
				chrom1.insert(make_pair<int32_t,Mutation*>(i,mutation));
				}
			    if(where==1 || where==2)
				{
				chrom2.insert(make_pair<int32_t,Mutation*>(i,mutation));
				}
			    }
			WHERE("mutations=" << all_mutations.size());
			//loop over the non-masked ranges
			int32_t chromStart=0;
			int32_t chromEnd=0;
			while(chromStart< (int32_t)sequence.size())
			    {
			    if(!mask[chromStart])
				{
				chromStart++;
				continue;
				}
			    //ok we found chromStart, now incr chromEnd while mask==true
			    chromEnd=chromStart+1;
			    while(chromEnd < (int32_t)sequence.size() && mask[chromEnd])
				{
				chromEnd++;
				}
			    WHERE(chromStart << "-" << chromEnd);
			    //process the unmasked segment
			    int32_t segLength=chromEnd-chromStart;
			    if(paired_end_size<=segLength)
				{
				uint64_t n_pairs = (uint64_t)((((double)segLength/(double)genome_size)+0.5) * total_num_pairs);
				WHERE(n_pairs);
				/* generate all the pairs */
				for(uint64_t i=0;i< n_pairs;++i)
				    {
				    int seq_index=chromStart+(int32_t)lrand48()%(segLength-paired_end_size);
				    int paired_len= paired_end_size+(lrand48()%paired_end_stddev)*(::drand48()<0.5?1:-1);
				    map<int32_t,Mutation*>* chrom2mut=(rand()%2==0?&chrom1:&chrom2);
				    string amplicon;
				    while(seq_index < (int32_t)sequence.size() &&
					    (int32_t)amplicon.size()< paired_len
				    )
					{
					map<int32_t,Mutation*>::iterator r=chrom2mut->find(seq_index);
					if(r==chrom2mut->end())
					    {
					    if(drand48()< this->base_error_rate)
						{
						amplicon+= notIn(sequence.at(seq_index));
						}
					    else
						{
						amplicon+= sequence.at(seq_index);
						}
					    seq_index++;
					    continue;
					    }
					Mutation* mutation=r->second;
					WHERE("mutation "<< mutation->toString());
					switch(mutation->type())
					    {
					    case SUBST:
						{
						Substitution* sub=(Substitution*)mutation;
						amplicon+= sub->base;
						seq_index++;
						break;
						}
					    case INS:
						{
						Insertion* sub=(Insertion*)mutation;
						amplicon.append(sub->sequence);
						amplicon+= sequence.at(seq_index);
						seq_index++;
						break;
						}
					    case DEL:
						{
						Deletion* sub=(Deletion*)mutation;
						seq_index+=sub->length;
						break;
						}
					    }
					}

				    WHERE(amplicon);
				    if(rand()%2==0)//use reverse complement
					{
					std::reverse(amplicon.begin(),amplicon.end());
					for(size_t i=0;i< amplicon.size();++i)
					    {
					    amplicon[i]=complement(amplicon[i]);
					    }
					}

				    emit(chrom,amplicon);
				    }
				}
			    chromStart=chromEnd;
			    }
			//free all mutations
			WHERE("mutations=" << all_mutations.size());
			for(size_t i=0;i< all_mutations.size();++i)
			    {
			    delete all_mutations[i];
			    }

			}
		    if(c==EOF) break;
		    chrom.clear();
		    sequence.clear();
		    while((c=gzgetc(in))!=-1 && c!='\n')
			{
			if(c!='\r') chrom+=(char)c;
			}
		    int32_t expect=chrom2size[chrom];
		    sequence.reserve(expect);
		    continue;
		    }
		else if(isalpha(c))
		    {
		    sequence+=(char)c;
		    }
		}
	    gzclose(in);
	    }


	void readCapture(const char* filename)
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
		int32_t chromStart=atoi(tokens[1].c_str());
		int32_t chromEnd=atoi(tokens[2].c_str());
		if(chromStart<0 || chromEnd <chromStart)
		    {
		    cerr << "Bad range in " << line << endl;
		    continue;
		    }
		StartEnd range(chromStart,chromEnd);
		capture[ tokens[0] ].push_back(range);
		}
	    buf.close();
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
		int32_t pos=atoi(tokens[1].c_str());
		if(pos<=0)
		    {
		    cerr << "Bad range in " << line << endl;
		    continue;
		    }
		if(tokens[2].size()!=1 || !isalpha(tokens[2].at(0)))
		    {
		    cerr << "Bad base 1 in " << line << endl;
		    continue;
		    }
		if(tokens[3].size()!=1 || !isalpha(tokens[3].at(0)))
		    {
		    cerr << "Bad base 2 in " << line << endl;
		    continue;
		    }
		UserDefinedSubstitution* mut=new UserDefinedSubstitution;
		mut->chrom.assign(tokens[0]);
		mut->pos=pos-1;
		mut->base=tokens[2].at(0);
		mut->base2=tokens[3].at(0);
		if(user_defined_mutations[mut->chrom][mut->pos]!=NULL)
		    {
		    THROW("error mutation at "<< mut->chrom<< ":"<< (mut->pos+1)<< "defined twice");
		    }
		user_defined_mutations[mut->chrom][mut->pos]=mut;
		}
	    buf.close();
	    }

	void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] genome.fa"<< endl;
	    out << "Options:\n";
	    out << "  -f (file) limit by genomic region (optional) read file:chrom(TAB)start(TAB)end\n";
	    out << "  -o (file.tar) save as TAR file\n";
	    out << "  -N (int) approximate number of reads. default: "<< this->total_num_pairs << "\n";
	    out << "  -e(float) base error rate. default: "<< this->base_error_rate << "\n";
	    out << "  -r (float) rate of mutations. default: "<< this->proba_mutation << "\n";
	    out << "  -d (int) distance between the two pairs default: "<< this->paired_end_size << "\n";
	    out << "  -s (int) pair length std deviation default: "<< this->paired_end_stddev << "\n";
	    out << "  -L (int) read length default: "<< this->short_read_length << "\n";
	    out << "  -R (float) fraction of indels default: "<< this->indel_fraction << "\n";
	    out << "  -X (float)  probability an indel is extended default: "<< this->indel_fraction << "\n";
	    out << " ** Inserting  User defined substitutions (optional):\n";
	    out << "    -u (filename) read a file containing user-defined mutations (optional). Format: (CHROM)\\t(POS+1)\\t(BASE1)\\t(BASE2)\n";
	    out << "    -m (chrom) (POS+1) (BASE1) (BASE2) insert user defined substitution.\n";
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    const char* outfile=NULL;
	    int optind=1;
	    srand(time(0));
	    srand48(time(0));

	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-N")==0 && optind+1< argc)
		    {
		    this->total_num_pairs=atol(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-e")==0 && optind+1< argc)
		    {
		    this->base_error_rate=atof(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-r")==0 && optind+1< argc)
		    {
		    this->proba_mutation=atof(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    this->paired_end_size=atoi(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-s")==0 && optind+1< argc)
		    {
		    this->paired_end_stddev=atoi(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-L")==0 && optind+1< argc)
		    {
		    this->short_read_length=atoi(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-R")==0 && optind+1< argc)
		    {
		    this->indel_fraction=atof(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-X")==0 && optind+1< argc)
		    {
		    this->proba_extend=atof(argv[++optind]);
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
		    }
		else if(std::strcmp(argv[optind],"-f")==0 && optind+1< argc)
		    {
		    readCapture(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-u")==0 && optind+1< argc)
		    {
		    readUserDefinedMutations(argv[++optind]);
		    }
		else if(std::strcmp(argv[optind],"-u")==0 && optind+4< argc)
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
		    if(strlen(base1)!=1 || !isalpha(base1[0]))
			{
			usage(cerr,argc,argv);
			cerr << "bad base-1 in " << base1 << endl;
			return EXIT_FAILURE;
			}
		    char* base2=argv[++optind];
		    if(strlen(base2)!=1 || !isalpha(base2[0]))
			{
			usage(cerr,argc,argv);
			cerr << "bad base-2 in " << base2 << endl;
			return EXIT_FAILURE;
			}
		    UserDefinedSubstitution* mut=new UserDefinedSubstitution;
		    mut->chrom.assign(chrom);
		    mut->pos=pos-1;
		    mut->base=base1[0];
		    mut->base2=base1[1];
		    if(user_defined_mutations[mut->chrom][mut->pos]!=NULL)
			{
			cerr << "error mutation at "<< mut->chrom<< ":"<< (mut->pos+1)<< "defined twice.\n";
			return EXIT_FAILURE;
			}
		    user_defined_mutations[mut->chrom][mut->pos]=mut;
		    }
		/*
		else if(std::strcmp(argv[optind],"-p")==0 && optind+1< argc)
		    {
		    char* p=argv[++optind];
		    char* c=strchr(p,':');
		    if(c==NULL || !isdigit(*(c+1)))
			{
			usage(cerr,argc,argv);
			cerr << "bad position in "<< p << endl;
			return EXIT_FAILURE;
			}
		    *c=0;
		    c++;
		    this->inject_mutation_pos=new ChromPosition(p,atoi(c));
		    if(this->inject_mutation_pos->pos<=0)
			{
			usage(cerr,argc,argv);
			cerr << "bad user position in "<< (*this->inject_mutation_pos) << endl;
			return EXIT_FAILURE;
			}
		    }
		else if(std::strcmp(argv[optind],"-b")==0 && optind+1< argc)
		    {
		    char* base=argv[++optind];
		    char b=toupper(base[0]);
		    if(strlen(base)!=1 || !(b=='A' || b=='T' || b=='G' || b=='C'))
			{
			usage(cerr,argc,argv);
			cerr << "bad base in "<< base << endl;
			return EXIT_FAILURE;
			}
		    this->inject_mutation_base=b;
		    }
		else if(std::strcmp(argv[optind],"-b")==0)
		    {
		    this->inject_mutation_homozygous=false;
		    }*/
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


	    this->outfq1=tmpfile();
	    if(outfq1==NULL)
		{
		cerr << "Cannot open tmpFile for fastq-1:" << strerror(errno)<< endl;
		return EXIT_FAILURE;
		}
	    this->outfq2=tmpfile();
	    if(outfq2==NULL)
		{
		cerr << "Cannot open tmpFile for fastq-2:" << strerror(errno)<< endl;
		return EXIT_FAILURE;
		}
	    this->outvcf=tmpfile();
	    if(outvcf==NULL)
		{
		cerr << "Cannot open tmpFile for VCF:" << strerror(errno)<< endl;
		return EXIT_FAILURE;
		}

	    this->run(argv[optind]);

	    std::fflush(this->outfq1);
	    std::fflush(this->outfq2);
	    std::fflush(this->outvcf);

	    this->tarball->putFile(this->outfq1,"sim/reads1.fastq");
	    this->tarball->putFile(this->outfq2,"sim/reads2.fastq");
	    this->tarball->putFile(this->outvcf,"sim/mutations.txt");
	    this->tarball->finish();

	    std::fclose(this->outfq1);
	    std::fclose(this->outfq2);
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
    ShortReadSim app;
    return app.main(argc,argv);
    }
