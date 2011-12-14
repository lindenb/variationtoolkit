/*
 * xbam.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */

#ifndef XBAM_H
#define XBAM_H
#include <stdint.h>


class BamFile
    {

    public:
	BamFile(const char* file);
	virtual ~BamFile();
	const char* findNameByTid(int32_t tid);
	int32_t findTidByName(const char* seq_name);
    private:
	void* fp;
	void *header;
	void* index;
    friend class TTView;
    };


#endif
