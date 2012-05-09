/* 'Tee'-Counting program
 *  Author: Pierre Lindenbaum
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

int main(int argc,char** argv)
	{
	int c;
	uint64_t count=0UL;
	FILE* out=NULL;
	
	if(!(argc==2 || argc==1))
		{
		fprintf(stderr,"Usage: %s  (fileout).\n",argv[0]);
		return EXIT_FAILURE;
		}
	
	if(argc>1)
		{
		errno=0;
		out=fopen(argv[1],"w");
		if(out==NULL)
			{
			fprintf(stderr,"Cannot open %s %s\n",argv[1],strerror(errno));
			return EXIT_FAILURE;
			}
		}
	
	while((c=fgetc(stdin))!=EOF)
		{
		if(c=='\n') ++count;
		fputc(c,stdout);
		}
	
	if(out!=NULL)
		{
		fprintf(out,"%Ld\n",count);
		fflush(out);
		fclose(out);
		}
	
	return EXIT_SUCCESS;
	}
