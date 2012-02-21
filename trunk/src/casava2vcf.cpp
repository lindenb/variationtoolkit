/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	transforms Casava SNP/Indel files to VCF
 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <set>
#include <cerrno>
#include <iostream>
#include <limits>
#include <string>
#include <stdint.h>
#include "throw.h"
#include "tokenizer.h"
#include "numeric_cast.h"
#include "zstreambuf.h"
#include "xfaidx.h"

using namespace std;

class CasavaToVCF
    {
    private:
	Tokenizer tokenizer;
	std::auto_ptr<string> buffer;

	bool next(std::istream& in,string& line)
	    {
	    if(buffer.get()!=0)
		{
		line.assign(*buffer);
		buffer.release();
		return true;
		}
	    else
		{
		return getline(in,line,'\n');
		}
	    }

    public:
	CasavaToVCF()
	    {

	    }

	virtual ~CasavaToVCF()
	    {

	    }

	void run_snp(std::istream& in)
	    {
	    string line;
	    vector<string> tokens;
	    while(next(in,line))
		{
		if(line.empty()) continue;
		tokenizer.split(line,tokens);
		if(tokens[0].compare("seq_name")==0)
		    {
		    buffer.reset(new string(line));
		    return;
		    }
		buffer.release();
		if(tokens.size()<14) THROW("Expected at least 14 columns in "<< line << "  but got "<< tokens.size());

		if(tokens[4].size()!=1) THROW("Expected REF=one base in "<< line);

		set<char> genotypes;
		for(size_t i=0;i< tokens[6].size();++i)
		    {
		    genotypes.insert(tokens[6].at(i));
		    }

		set<char> genotypes_save(genotypes.begin(),genotypes.end());


		genotypes.erase(tokens[4].at(0));

		if(genotypes.empty()) continue;

		cout << tokens[0] << "\t" ; //CHROM
		cout << tokens[1] << "\t" ; //POS
		cout << "." << "\t" ; //ID
		cout << tokens[4] << "\t" ; //REF

		for(set<char>::iterator r=genotypes.begin();r!=genotypes.end();++r)
		    {
		    if(r!=genotypes.begin()) cout <<",";
		    cout << (*r);
		    }
		cout << "\t" ; //ALT
		cout << tokens[5]  << "\t" ; //QUAL
		cout << ".\t" ; //FILTER
		cout << "DP="<< tokens[2];
		cout << ";bcalls_used="<< tokens[2];
		cout << ";Q_snp="<< tokens[5];
		cout << ";max_gt="<< tokens[6];
		cout << ";Q_max_gt="<< tokens[7];
		cout << ";max_gt_poly_site="<< tokens[8];
		cout << ";Q_max_gt_poly_site="<< tokens[8];
		cout << ";A_used="<< tokens[10];
		cout << ";C_used="<< tokens[11];
		cout << ";G_used="<< tokens[12];
		cout << ";T_used="<< tokens[13];

		cout << "\tGT:GQ:DP\t";
		if(genotypes_save.size()==2)
		    {
		    cout << "0/1";
		    }
		else if(genotypes_save.size()==1)
		    {
		    cout << "1/1";
		    }
		else
		    {
		    THROW("genotypes ? "<< line);
		    }

		cout << ":";
		cout << tokens[7];
		cout << ":";
		cout << tokens[2];

		cout << endl;
		}
	    }

	void run_indel(std::istream& in)
	    {
	    string line;
	    vector<string> tokens;
	    while(next(in,line))
		{
		if(line.empty()) continue;
		tokenizer.split(line,tokens);
		if(tokens[0].compare("seq_name")==0)
		    {
		    buffer.reset(new string(line));
		    return;
		    }
		buffer.release();
		if(tokens.size()<16) THROW("Expected at least 16 columns in "<< line << "  but got "<< tokens.size());
		if(tokens[7].compare("ref")==0) continue;

		string::size_type slash= tokens[4].find('/');
		if(slash==string::npos) THROW("no slash in "<< tokens[4]);
		if(slash==0 || slash+1==tokens[4].size()) THROW("nothing before/after slash ?? in "<< tokens[4]);
		string left( tokens[4].substr(0,slash));
		string right( tokens[4].substr(slash+1));
		if(left.size()!=right.size()) THROW("Bad indel "<<line);


		cout << tokens[0] << "\t" ; //CHROM
		int position1;
		if(!numeric_cast<int>(tokens[1].c_str(),&position1) || position1<1)
		    {
		    THROW("Bad position in "<< line);
		    }
		const string& aval=tokens[3];
		if(aval.empty()) THROW("upstream sequence ??"<< aval);
		string indel;
		size_t indel_size;
		if(left[0]=='-') /* -----/CCTCT insertion */
		    {
		    if(left.find_first_not_of('-')!=string::npos) THROW("BAD indel "<< tokens[4]);
		    if(right.find('-')!=string::npos) THROW("BAD indel "<< tokens[4]);
		    if(right.find_first_not_of("ATGCNatgcn")!=string::npos) THROW("BAD indel "<< tokens[4]);

		    indel.assign("INSERTION");
		    indel_size=right.size();
		    cout << position1-1 << "\t" ; //yes VCF use the base before
		    cout << "." << "\t" ; //ID
		    cout << aval.at(aval.size()-1) << "\t";
		    cout << aval.at(aval.size()-1) << right << "\t";
		    }
		else /* CCTCT/-----  deletion */
		    {

		    if(left.find('-')!=string::npos) THROW("BAD indel "<< tokens[4]);
		    if(left.find_first_not_of("ATGCNatgcn")!=string::npos) THROW("BAD indel "<< tokens[4]);
		    if(right.find_first_not_of('-')!=string::npos) THROW("BAD indel "<< tokens[4]);

		    indel.assign("DELETION");
		    indel_size=left.size();

		    cout << position1-1 << "\t" ; //before yes VCF use the base before
		    cout << "." << "\t" ; //ID
		    cout << aval.at(aval.size()-1) << left << "\t";
		    cout << aval.at(aval.size()-1) << "\t";
		    }

		cout << tokens[6]  << "\t" ; //QUAL
		cout << ".\t" ; //FILTER
		cout << indel<<"=" << indel_size << ";DP="<< tokens[10];
		cout << ";alt_reads="<< tokens[10];
		cout << ";indel_reads="<< tokens[11];
		cout << ";other_reads="<< tokens[12];
		cout << ";repeat_unit="<< tokens[13];
		cout << ";ref_repeat_count="<< tokens[14];
		cout << ";indel_repeat_count="<< tokens[15];
		cout << "\tGT:GQ:DP\t";
		if(tokens[7].compare("het")==0)
		    {
		    cout << "0/1";
		    }
		else if(tokens[7].compare("hom")==0)
		    {
		    cout << "1/1";
		    }
		else
		    {
		    THROW("genotypes ? "<< line);
		    }

		cout << ":";
		cout << tokens[8];
		cout << ":";
		cout << tokens[10];

		cout << endl;
		}
	    }

#define VERIFYCOL(k,i) \
	if((int)tokens.size()<=i) THROW("Expected at least "<< (i+1) << "columns in "<< line << "  but got "<< tokens.size());\
	if(tokens[i].compare(k)!=0) THROW("Expected column "<< i << " to be "<< k << " but got "<< tokens[i]<< " in "<< line)

	void run(std::istream& in)
	    {
	    string line;
	    vector<string> tokens;
	    while(next(in,line))
		{
		if(line.empty()) continue;
		tokenizer.split(line,tokens);
		if(tokens.size()>2 && tokens[2].compare("bcalls_used")==0)
		    {
		    VERIFYCOL("seq_name",0);
		    VERIFYCOL("pos",1);
		    VERIFYCOL("bcalls_used",2);
		    VERIFYCOL("bcalls_filt",3);
		    VERIFYCOL("ref",4);
		    VERIFYCOL("Q(snp)",5);
		    VERIFYCOL("max_gt",6);
		    VERIFYCOL("Q(max_gt)",7);
		    VERIFYCOL("max_gt|poly_site",8);
		    VERIFYCOL("Q(max_gt|poly_site)",9);
		    VERIFYCOL("A_used",10);
		    VERIFYCOL("C_used",11);
		    VERIFYCOL("G_used",12);
		    VERIFYCOL("T_used",13);
		    buffer.release();
		    run_snp(in);
		    }
		else if(tokens.size()>2 && tokens[2].compare("type")==0)
		    {
		    VERIFYCOL("seq_name",0);
		    VERIFYCOL("pos",1);
		    VERIFYCOL("type",2);
		    VERIFYCOL("ref_upstream",3);
		    VERIFYCOL("ref/indel",4);
		    VERIFYCOL("ref_downstream",5);
		    VERIFYCOL("Q(indel)",6);
		    VERIFYCOL("max_gtype",7);
		    VERIFYCOL("Q(max_gtype)",8);
		    VERIFYCOL("depth",9);
		    VERIFYCOL("alt_reads",10);
		    VERIFYCOL("indel_reads",11);
		    VERIFYCOL("other_reads",12);
		    VERIFYCOL("repeat_unit",13);
		    VERIFYCOL("ref_repeat_count",14);
		    VERIFYCOL("indel_repeat_count",15);
		    buffer.release();
		    run_indel(in);
		    }
		else
		    {
		    THROW("bad header in \""<< line << "\"");
		    }

		}
	    }
	void usage(ostream& out,int argc,char **argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Usage:\n\t"<< argv[0] << " [options] files.txt\n";
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
	    cout << "##fileformat=VCFv4.1"<< endl;
	    cout << "##" << argv[0] << endl;
	    cout << "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Basecalls used to make the genotype call for this site\">" << endl;
	    cout << "##INFO=<ID=bcalls_filt,Number=1,Type=Integer,Description=\" Basecalls mapped to the site but filtered out before genotype calling\">" << endl;
	    cout << "##INFO=<ID=Q_snp,Number=1,Type=String,Description=\"A Q-value expressing the probability of the homozygous reference genotype\">" << endl;
	    cout << "##INFO=<ID=max_gt,Number=1,Type=Integer,Description=\"The most likely genotype\">" << endl;
	    cout << "##INFO=<ID=Q_max_gt,Number=1,Type=Integer,Description=\"Q-value expressing the probability that the genotype is not the most likely genotype\">" << endl;
	    cout << "##INFO=<ID=max_gt_poly_site,Number=1,Type=Integer,Description=\"The most likely genotype assuming this site is polymorphic with an expected allele frequency of 0.5\">" << endl;
	    cout << "##INFO=<ID=Q_max_gt_poly_site,Number=1,Type=Integer,Description=\"Q(max_gt|poly_site)\">" << endl;
	    cout << "##INFO=<ID=A_used,Number=1,Type=Integer,Description=\"A_used\">" << endl;
	    cout << "##INFO=<ID=C_used,Number=1,Type=Integer,Description=\"C_used\">" << endl;
	    cout << "##INFO=<ID=G_used,Number=1,Type=Integer,Description=\"G_used\">" << endl;
	    cout << "##INFO=<ID=T_used,Number=1,Type=Integer,Description=\"T_used\">" << endl;

cout << "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">" << endl;
cout << "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">" << endl;
cout << "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Basecalls mapped to the site but filtered out before genotype calling\">" << endl;

/*
cout << "##FORMAT=<ID=GL,Number=3,Type=Float,Description=\"Likelihoods for RR,RA,AA genotypes (R=ref,A=alt)\">" << endl;
cout << "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"# high-quality bases\">" << endl;
cout << "##FORMAT=<ID=SP,Number=1,Type=Integer,Description=\"Phred-scaled strand bias P-value\">" << endl;
cout << "##FORMAT=<ID=PL,Number=-1,Type=Integer,Description=\"List of Phred-scaled genotype likelihoods, number of values is (#ALT+1)*(#ALT+2)/2">\" << endl;
*/
	    cout << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\t__SAMPLE__" << endl;

	    if(optind==argc)
		    {
		    igzstreambuf buf;
		    istream in(&buf);
		    run(in);
		    }
	    else
		    {
		    while(optind< argc)
			    {
			    igzstreambuf buf(argv[optind++]);
			    istream in(&buf);
			    this->run(in);
			    buf.close();
			    ++optind;
			    }
		    }
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    CasavaToVCF app;
    return app.main(argc,argv);
    }
