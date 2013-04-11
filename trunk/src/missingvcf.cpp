#include<cstdio>
#include<vector>
#include<cstring>
#include<cstdlib>
#include<string>

using namespace std;

static void usage(int argc,char** argv)
	{
	fprintf(stderr," %s [-S SAMPLE]+ file\n",argv[0]);
	}

int main(int argc,char** argv)
	{
	vector<char*> samples;
	int optind=1;
	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    usage(argc,argv);
		    return(EXIT_FAILURE);
		    }
		else if(strcmp(argv[optind],"-S")==0 && optind+1< argc)
		    {
		    samples.push_back(argv[++optind]);
		    }
		else if(strcmp(argv[optind],"--")==0)
		    {
		    ++optind;
		    break;
		    }
		else if(argv[optind][0]=='-')
		    {
		    fprintf(stderr,"unknown option '%s'\n", argv[optind]);
		    usage(argc,argv);
		    return EXIT_FAILURE;
		    }
		else
		    {
		    break;
		    }
		++optind;
		}
	if(samples.empty())
		{
		fprintf(stderr,"[WARNING] no sample defined.\n");
		}

	if(optind!=argc)
		{
		fprintf(stderr,"expect input from stdin.\n");
		return EXIT_FAILURE;
		}
	
	int c=fgetc(stdin);
	if(c==EOF)
		{
		fputs("##fileformat=VCFv4.1\n",stdout);
		fputs("##EMPTY-VCF-GENERATED-TO-AVOID-PROGRAMS-FAILURE\n",stdout);
		fputs("#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT",stdout);
		for(size_t i=0;i< samples.size();++i)
			{
			fputc('\t',stdout);
			fputs(samples[i],stdout);
			}
		fputc('\n',stdout);
		}
	else if(c!='#')
		{
		fprintf(stderr,"[ERROR] input doesn't start with '#'.\n");
		return EXIT_FAILURE;
		}
	else
		{
		fputc(c,stdout);
		while((c=fgetc(stdin))!=EOF)
			{
			fputc(c,stdout);
			}
		}
		
	return EXIT_SUCCESS;
	}
