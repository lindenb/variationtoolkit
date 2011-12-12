/*
 * xstdlib.h
 *
 *  Created on: Aug 8, 2011
 *      Author: Pierre Lindenbaum PhD
 *              plindenbaum@yahoo.fr
 *              http://plindenbaum.blogspot.com
 *              
 */

#ifndef XSTDLIB_H_
#define XSTDLIB_H_
#include <cstdlib>

void *safeRealloc (void *ptr, size_t size);
void *safeMalloc (std::size_t size);


#endif /* XSTDLIB_H_ */
