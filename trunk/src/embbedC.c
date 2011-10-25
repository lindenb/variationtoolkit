#include <stdio.h>

int main(int argc,char** argv)
	{
	int c;
	int n=0;
	if(argc!=2) return -1;
	printf("char* %s=\"",argv[1]);
	while((c=fgetc(stdin))!=EOF)
		{
		++n;
		switch(c)
			{
			case '\n': fputs("\\n",stdout);break;
			case '\b': fputs("\\b",stdout);break;
			case '\v': fputs("\\v",stdout);break;
			case '\r': fputs("\\r",stdout);break;
			case '\t': fputs("\\t",stdout);break;
			case '\\': fputs("\\\\",stdout);break;
			case '\"': fputs("\\\"",stdout);break;
			case '\'': fputs("\\\'",stdout);break;
			default: fputc(c,stdout);break;
			}
		}
	printf("\";\n");
	printf("unsigned long %s_length=%d;\n",argv[1],n);
	return 0;
	}
