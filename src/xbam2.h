/*
 * xbam.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */

#ifndef XBAM2_H
#define XBAM2_H
#include <string>
#include <vector>
#include <memory>
#include <stdint.h>
#include "bam.h"

class BamFile2
    {
  private:
	bamFile fp;
	bam_header_t *header;
	bam_index_t* index;
	std::string filename;
    public:
	class Target
	    {
            private:
		int32_t _tid;
		std::string _name;
		uint32_t _length;
	    public:
		int32_t tid() const;
		const std::string& name() const;
		uint32_t length() const;
		Target(int32_t tid,const char* name,uint32_t length);
		Target(const Target& cp);
		~Target();
		Target& operator=(const Target& cp);
	    };
	BamFile2(const char* file);
	virtual ~BamFile2();
	void open(bool load_index=true);
	void close();
	const char* findNameByTid(int32_t tid);
	int32_t findTidByName(const char* seq_name);
	bamFile bamPtr();
	bam_index_t* bamIndex();
	bam_header_t* bamHeader();
	const char* path() const;
	bool is_open() const;
	std::auto_ptr<std::vector<BamFile2::Target> > targets();
	int32_t count_targets() const;
	const char* target_name(int32_t n) const;
	int32_t target_length(int32_t n) const;
	//
	static int fetch_func(const bam1_t *b, void *data);
    };


#endif
