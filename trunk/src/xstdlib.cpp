#include <iostream>
#include "xstdio.h"
#include "xstdlib.h"
#include "throw.h"

using namespace std;

void *safeRealloc (void *ptr, size_t size)
    {
    ptr=std::realloc(ptr,size);
    if(ptr==NULL)
   	{
   	THROW("[Out of memory] Cannot re-allocate " << size << " bytes.");
   	}
    return ptr;
    }

void *safeMalloc (std::size_t size)
    {
    void* p=std::malloc(size);
    if(p==NULL)
	{
	THROW("[Out of memory] Cannot allocate " << size << " bytes.");
	}
    return p;
    }




