#ifndef XSTDIO_H_
#define XSTDIO_H_
#include <cstdio>
#include <memory>

std::FILE *safeFOpen (const char *filename,const char *modes);
std::FILE *safeTmpFile();
char* readLine(std::FILE* in,std::size_t* len);


std::size_t safeFRead(void *ptr, std::size_t size, std::size_t nmemb, std::FILE *stream);
std::size_t safeFWrite(const void *ptr, std::size_t size, std::size_t nmemb, std::FILE *stream);
int safeFSeek(std::FILE * stream, long int offset, int origin);
int safeFFlush(std::FILE * stream);
void safeRewind(std::FILE *stream);

#endif

