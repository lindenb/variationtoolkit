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

class CigarOp
    {
    public:
	static const CigarOp MATCH;
	static const CigarOp INSERT;
	static const CigarOp* find(char c);
    private:
	CigarOp(char symbol);
    };

struct CigarOperation
    {
    char op;
    uint32_t count;
    };

class Cigar
    {
    public:
	Cigar() {}
	~Cigar() {}
    private:

    };

/**
 * Pure Abstract class for SAMRecord
 */
class SAMRecord
    {
    protected:
	SAMRecord();
	virtual const uint8_t * _seq() const=0;
    public:
	virtual ~SAMRecord();
	virtual int32_t flag() const=0;
	virtual const char* readName() const=0;
	/* the read is paired in sequencing, no matter whether it is mapped in a pair. */
	virtual bool isForwardStrand() const=0;
#define BAM_GETTER(fun) virtual bool fun() const

	BAM_GETTER(isReverseStrand);
	BAM_GETTER(isPaired);
	BAM_GETTER(isRead1);
	BAM_GETTER(isRead2);
	BAM_GETTER(isProperPair);
	BAM_GETTER(isUnmapped);
	BAM_GETTER(isQCFail);
	BAM_GETTER(isDuplicate);

#undef BAM_GETTER
	virtual int32_t size() const=0;
	virtual char at(int32_t idx) const;
	virtual int32_t tid() const=0;
	virtual int32_t pos() const=0;
    };

/**
 *  implementation of SAMRecord, only a wrapper around  bam1_t*
 * doesn't manage to manage/free memory
 */
class DelegateSAMRecord:public SAMRecord
    {
    public:
	DelegateSAMRecord(bam1_t* ptr);
	virtual ~DelegateSAMRecord();
	virtual const char* readName() const;
	virtual bool isForwardStrand() const;
	virtual int32_t flag() const;
	virtual int32_t size() const;
	virtual int32_t tid() const;
	virtual int32_t pos() const;
    protected:
	virtual const bam1_t* ptr() const;
	virtual const uint8_t* _seq() const;
	const bam1_core_t* core() const;
	bam1_t* _ptr;
    };

/**
 * DefaultSAMRecord: default implementation of a SAM record
 * copy the _ptr
 */
class DefaultSAMRecord:public DelegateSAMRecord
    {
    public:
	DefaultSAMRecord(bam1_t* ptr);
	DefaultSAMRecord(const DefaultSAMRecord& cp);
	virtual ~DefaultSAMRecord();
	DefaultSAMRecord& operator=(const DefaultSAMRecord& cp);
    };



class BamFile2
    {
  private:
	bamFile fp;
	bam_header_t *header;
	bam_index_t* index;
	std::string filename;
    public:
	class Iterator
	    {
	    public:
		~Iterator();
		const SAMRecord* next();
		void close();
	    private:
		Iterator(bam_iter_t iter,BamFile2* owner);
		bam_iter_t _iter;
		BamFile2* _owner;
		bam1_t *_rec;
		SAMRecord* _sam;
	    friend class BamFile2;
	    };

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
	//iterators
	std::auto_ptr<Iterator> query(int tid, int beg, int end);
	std::auto_ptr<Iterator> query(int tid);
	std::auto_ptr<Iterator> query();
    };


#endif
