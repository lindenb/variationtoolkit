#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <unistd.h>

using namespace std;

/*
from http://stackoverflow.com/questions/3219393/
http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
*/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_BLACK   "\x1b[0m"
#define ANSI_COLOR_RESET   ANSI_COLOR_BLACK

static void fasta2term(FILE *in)
	{
	int prev_c=-1;
	int c;
	
	//not the terminal ? just 'cat'
	if(!isatty(fileno(stdout)))
		{
		char buff[BUFSIZ];
		int nRead;
		while((nRead=fread(buff,1,BUFSIZ,in))>0)
			{
			fwrite(buff,1,nRead,stdout);
			}
		return;
		}
	
	while((c=fgetc(in))!=EOF)
		{
		if(c=='>')
			{
			
			if(prev_c!=-1) fputs(ANSI_COLOR_RESET,stdout);
			fputc('>',stdout);
			while((c=fgetc(in))!=EOF)
				{
				fputc(c,stdout);
				if(c=='\n') break;
				}
			prev_c=-1;
			continue;
			}
		else if(c==prev_c || isspace(c))
			{
			fputc(c,stdout);
			continue;
			}
		
		switch(tolower(c))
			{
			case 'a': fputs(ANSI_COLOR_GREEN,stdout); break;
			case 't': fputs(ANSI_COLOR_RED,stdout); break;
			case 'g': fputs(ANSI_COLOR_BLACK,stdout); break;
			case 'c': fputs(ANSI_COLOR_BLUE,stdout); break;
			default: fputs(ANSI_COLOR_YELLOW,stdout); break;
			}
		fputc(c,stdout);
		prev_c=c;
		}
	if(prev_c!=-1) fputs(ANSI_COLOR_RESET,stdout);
	}

int main(int argc,char** argv)
	{
	int optind=1;
	  while(optind < argc)
                        {
                        if(strcmp(argv[optind],"-h")==0)
                                {
                                fprintf(stderr,"%s: Pierre Lindenbaum PHD. 2010.\n",argv[0]);
                         	fprintf(stderr,"Compilation: %s at %s.\n",__DATE__,__TIME__);
                                exit(EXIT_FAILURE);
                                }
                        else if(strcmp(argv[optind],"--")==0)
				{
				++optind;
                                break;
				}
			 else if(argv[optind][0]=='-')
                                {
                                fprintf(stderr,"unknown option '%s'\n",argv[optind]);
                               	exit(EXIT_FAILURE);
                                }
                        else
                                {
                                break;
                                }
                        ++optind;
                        }
	if(optind==argc)
		{
		fasta2term(stdin);
		}
	else
		{
		while(optind< argc)
			{
			FILE* in;
			errno=0;
			in=fopen(argv[optind],"r");
			if(in==NULL)
				{
				fprintf(stderr,"Cannot open %s : %s\n",argv[optind],strerror(errno));
				return EXIT_FAILURE;
				}
			fasta2term(in);
			fclose(in);
			++optind;
			}
		}
	return EXIT_SUCCESS;
	}
