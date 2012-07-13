/* 'Tee+head' program
 *  Author: Pierre Lindenbaum
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

static void usage(int argc,char** argv)
	{
	fprintf(stderr," %s [-n num-lines] [fileout] stdin.\n",argv[0]);
	}

int main(int argc,char** argv)
	{
	uint64_t nLines=10;
	int c;
	uint64_t count=0UL;
	FILE* out=NULL;
	int optind=1;
	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    usage(argc,argv);
		    return(EXIT_FAILURE);
		    }
		else if(strcmp(argv[optind],"-n")==0)
		    {
		    nLines=atoi(argv[++optind]);
		    if(nLines<0)
			{
			fprintf(stderr," Bad number of lines %s.\n",argv[optind]);
			return EXIT_FAILURE;
			}
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


	if(optind==argc)
		{
		/* nothing */
		}
	else if(optind+1==argc)
		{
		errno=0;
		out=fopen(argv[optind],"w");
		if(out==NULL)
			{
			fprintf(stderr,"Cannot open %s %s\n",argv[optind],strerror(errno));
			return EXIT_FAILURE;
			}
		}
	else
		{
		usage(argc,argv);
		return EXIT_FAILURE;
		}
	
	while((c=fgetc(stdin))!=EOF)
		{
		if(count< nLines && out!=NULL) fputc(c,out);
		if(c=='\n')
			{
			++count;
			}
		fputc(c,stdout);
		
		}
	
	if(out!=NULL)
		{
		fflush(out);
		fclose(out);
		}
	
	return EXIT_SUCCESS;
	}
