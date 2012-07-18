/**
 * Author:
 *	Pierre Lindenbaum PhD
 *	This program was later edited by Andre Blavier ( interactive-biosoftware.com ) but I didn't test it.
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
#include <boost/tokenizer.hpp>
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

	string parseChrom(const string& src)
	{
		// Try to extract the chrom name out of e.g. "chr17.fa".
		// Return empty string if not found.
		// (A poor man's implementation, without regex)
		if (src.empty()) return string();
		string prefix = src.substr(0, 3);
		string src2 = src;
		if (prefix == "chr" || prefix == "Chr" || prefix == "CHR")
			src2.erase(0, 3);

		if (src2.empty()) return string();
		if (src2.at(0) == 'X' || src2.at(0) == 'x') return "X";
		if (src2.at(0) == 'x' || src2.at(0) == 'y') return "Y";
		if (src2.at(0) == 'M' || src2.at(0) == 'm') return "MT";  // a bit risky?

		string chrom;
		for (size_t i = 0; i < src2.size(); ++i)
		{
			char c = src2.at(i);
			if (isdigit(c)) chrom += c;
			else break;
		}
		return chrom;
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

		cout << parseChrom(tokens[0]) << "\t" ; //CHROM
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
//		cout << ";bcalls_used="<< tokens[2];
		cout << ";Q_snp="<< tokens[5];
		cout << ";max_gt="<< tokens[6];
		cout << ";Q_max_gt="<< tokens[7];
		cout << ";max_gt_poly_site="<< tokens[8];
		cout << ";Q_max_gt_poly_site="<< tokens[9];
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

		int depth=atoi(tokens[9].c_str());
		if(depth<1)
		    {
		    continue;
		    }


		string::size_type slash= tokens[4].find('/');
		if(slash==string::npos) THROW("no slash in "<< tokens[4]);
		if(slash==0 || slash+1==tokens[4].size()) THROW("nothing before/after slash ?? in "<< tokens[4]);
		string left( tokens[4].substr(0,slash));
		string right( tokens[4].substr(slash+1));
		if(left.size()!=right.size()) THROW("Bad indel "<<line);


		cout << parseChrom(tokens[0]) << "\t" ; //CHROM
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
		cout << indel<<"=" << indel_size << ";DP="<<depth;
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
		cout << depth;

		cout << endl;
		}
	    }

#define CHECK_NEXT_COL(k) \
	++tok_it; if (tok_it == tokens.end() || *tok_it != k) THROW(col_err_msg);

	void run(std::istream& in)
	    {
	    string line;
	    vector<string> tokens;
	    while(next(in,line))
		{
		if(line.empty()) continue;
		if (line.substr(0, 2) == "#$")		// header line
		{
			typedef boost::tokenizer<boost::char_separator<char> > boost_tokenizer;
			boost::char_separator<char> sep(" ");
			boost_tokenizer tokens(line, sep);
			boost_tokenizer::iterator tok_it = tokens.begin();
			++tok_it;  // skip "#$"
			if (tok_it == tokens.end()) continue;
			else if (*tok_it == "COLUMNS")
			{
				cout << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT\t__SAMPLE__" << endl;
				static const char* col_err_msg = "Invalid COLUMNS header line";
				CHECK_NEXT_COL("seq_name");
				CHECK_NEXT_COL("pos");
				++tok_it; if (tok_it == tokens.end()) THROW(col_err_msg);
				if (*tok_it == "bcalls_used")
				{
					CHECK_NEXT_COL("bcalls_filt");
					CHECK_NEXT_COL("ref");
					CHECK_NEXT_COL("Q(snp)");
					CHECK_NEXT_COL("max_gt");
					CHECK_NEXT_COL("Q(max_gt)");
					CHECK_NEXT_COL("max_gt|poly_site");
					CHECK_NEXT_COL("Q(max_gt|poly_site)");
					CHECK_NEXT_COL("A_used");
					CHECK_NEXT_COL("C_used");
					CHECK_NEXT_COL("G_used");
					CHECK_NEXT_COL("T_used");
					buffer.release();
					run_snp(in);
				}
				else if (*tok_it == "type")
				{
					CHECK_NEXT_COL("ref_upstream");
					CHECK_NEXT_COL("ref/indel");
					CHECK_NEXT_COL("ref_downstream");
					CHECK_NEXT_COL("Q(indel)");
					CHECK_NEXT_COL("max_gtype");
					CHECK_NEXT_COL("Q(max_gtype)");
					CHECK_NEXT_COL("depth");
					CHECK_NEXT_COL("alt_reads");
					CHECK_NEXT_COL("indel_reads");
					CHECK_NEXT_COL("other_reads");
					CHECK_NEXT_COL("repeat_unit");
					CHECK_NEXT_COL("ref_repeat_count");
					CHECK_NEXT_COL("indel_repeat_count");
					buffer.release();
					run_indel(in);
				}
				else THROW(col_err_msg);
			}
			else	// key-value pairs
			{
				cout << "##" << *tok_it << "=";
				++tok_it;
				for ( ; tok_it != tokens.end(); ++tok_it)
					cout << *tok_it << " ";
				cout << endl;
			}
		}
		else if (line.substr(0, 1) == "#") continue;
		else THROW("Invalid line \""<< line << "\"");
		}
	    }
	void usage(ostream& out,int argc,char **argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD, Andre Blavier 2012.\n";
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
	    cout << "##VCFConverter=casava2vcf" << endl;
	    cout << "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"Basecalls used to make the genotype call for this site\">" << endl;
//	    cout << "##INFO=<ID=bcalls_used,Number=1,Type=Integer,Description=\"Basecalls used to make the genotype call for this site\">" << endl;
	    cout << "##INFO=<ID=bcalls_filt,Number=1,Type=Integer,Description=\"Basecalls mapped to the site but filtered out before genotype calling\">" << endl;
	    cout << "##INFO=<ID=Q_snp,Number=1,Type=String,Description=\"A Q-value expressing the probability of the homozygous reference genotype\">" << endl;
	    cout << "##INFO=<ID=max_gt,Number=1,Type=String,Description=\"The most likely genotype\">" << endl;
	    cout << "##INFO=<ID=Q_max_gt,Number=1,Type=Integer,Description=\"Q-value expressing the probability that the genotype is not the most likely genotype\">" << endl;
	    cout << "##INFO=<ID=max_gt_poly_site,Number=1,Type=String,Description=\"The most likely genotype assuming this site is polymorphic with an expected allele frequency of 0.5\">" << endl;
	    cout << "##INFO=<ID=Q_max_gt_poly_site,Number=1,Type=Integer,Description=\"A Q-value expressing the probability that the genotype is not the most likely genotype above assuming this site is polymorphic\">" << endl;
	    cout << "##INFO=<ID=A_used,Number=1,Type=Integer,Description=\"'A' basecalls used\">" << endl;
	    cout << "##INFO=<ID=C_used,Number=1,Type=Integer,Description=\"'C' basecalls used\">" << endl;
	    cout << "##INFO=<ID=G_used,Number=1,Type=Integer,Description=\"'G' basecalls used\">" << endl;
	    cout << "##INFO=<ID=T_used,Number=1,Type=Integer,Description=\"'T' basecalls used\">" << endl;

	    cout << "##INFO=<ID=INSERTION,Number=1,Type=Integer,Description=\"Number of inserted bases\">" << endl;
	    cout << "##INFO=<ID=DELETION,Number=1,Type=Integer,Description=\"Number of deleted bases\">" << endl;
	    cout << "##INFO=<ID=alt_reads,Number=1,Type=Integer,Description=\"Number of reads strongly supporting either the reference path or an alternate indelpath\">" << endl;
	    cout << "##INFO=<ID=indel_reads,Number=1,Type=Integer,Description=\"Number of reads strongly supporting the indel path\">" << endl;
	    cout << "##INFO=<ID=other_reads,Number=1,Type=Integer,Description=\"Number of reads intersecting the indel, but not strongly supporting either the reference or any one indelpath\">" << endl;
	    cout << "##INFO=<ID=repeat_unit,Number=1,Type=String,Description=\"The smallest repeating sequence unit within the inserted or deleted sequence. For breakpoints this field is set to the value 'N/A'\">" << endl;
	    cout << "##INFO=<ID=ref_repeat_count,Number=1,Type=Integer,Description=\"Number of times the repeat_unit sequence is contiguously repeated starting from the indel start position in the reference case\">" << endl;
	    cout << "##INFO=<ID=indel_repeat_count,Number=1,Type=Integer,Description=\"Number of times the repeat_unit sequence is contiguously repeated starting from the indel start position in the indel case\">" << endl;

cout << "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">" << endl;
cout << "##FORMAT=<ID=GQ,Number=1,Type=Integer,Description=\"Genotype Quality\">" << endl;
cout << "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"Basecalls mapped to the site but filtered out before genotype calling\">" << endl;

/*
cout << "##FORMAT=<ID=GL,Number=3,Type=Float,Description=\"Likelihoods for RR,RA,AA genotypes (R=ref,A=alt)\">" << endl;
cout << "##FORMAT=<ID=DP,Number=1,Type=Integer,Description=\"# high-quality bases\">" << endl;
cout << "##FORMAT=<ID=SP,Number=1,Type=Integer,Description=\"Phred-scaled strand bias P-value\">" << endl;
cout << "##FORMAT=<ID=PL,Number=-1,Type=Integer,Description=\"List of Phred-scaled genotype likelihoods, number of values is (#ALT+1)*(#ALT+2)/2">\" << endl;
*/

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
