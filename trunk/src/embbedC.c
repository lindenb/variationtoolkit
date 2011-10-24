#include <stdio.h>

int main(int argc,char** argv)
	{
	int c;
	if(argc!=2) return -1;
	printf("char* %s=\"",argv[1]);
	while((c=fgetc(stdin))!=EOF)
		{
		switch(c)
			{
			case '\n': fputs("\\n",stdout);break;
			case '\r': fputs("\\r",stdout);break;
			case '\t': fputs("\\t",stdout);break;
			case '\\': fputs("\\\\",stdout);break;
			case '\"': fputs("\\\"",stdout);break;
			case '\'': fputs("\\\'",stdout);break;
			default: fputc(c,stdout);break;
			}
		}
	printf("\";\n");
	return 0;
	}
