/*
 * bam1sequence.h
 *
 *  Created on: Jun 29, 2012
 *      Author: Pierre Lindenbaum PhD
 *              plindenbaum@yahoo.fr
 *              http://plindenbaum.blogspot.com
 *              
 */

#ifndef BAM1SEQUENCE_H_
#define BAM1SEQUENCE_H_

#include "bam.h"
#include "sam.h"
#include "abstractcharsequence.h"


/**
 * Bam1Sequence
 */
class AbstractBam1Sequence:public AbstractCharSequence
    {
    protected:
    	AbstractBam1Sequence()
		{
		}
    public:
	virtual const bam1_t* ptr() const=0;

	virtual ~AbstractBam1Sequence()
		{
		}
	virtual char at(int32_t index) const
		{
		return bam_nt16_rev_table[bam1_seqi(bam1_seq(ptr()),index)];
		}
	virtual int32_t size() const
		{
		return ptr()->core.l_qseq;
		}

	virtual const char* name() const
	    {
	    return bam1_qname(ptr());
	    }

	virtual int flag() const
	    {
	    return ptr()->core.flag;
	    }

	bool is_flag_set(int flag_name) const
	    {
	    return ((flag() & (flag_name))!=0);
	    }

	bool is_flag_unset(int flag_name) const
	    {
	    return !is_flag_set(flag_name);
	    }

	bool is_paired() const
	    {
	    return is_flag_set(BAM_FPAIRED);
	    }

	bool is_mapped() const
	    {
	    return is_flag_unset(BAM_FUNMAP);
	    }
	bool is_mate_mapped() const
	    {
	    return is_flag_unset(BAM_FMUNMAP);
	    }

	bool is_reverse_strand()
	    {
	    return is_flag_set(BAM_FREVERSE);
	    }

	bool is_mate_reverse_strand()
	    {
	    return is_flag_set(BAM_FMREVERSE);
	    }

	bool is_proper_pair() const
	    {
	    return is_flag_set(BAM_FPAIRED);
	    }
	bool is_qc_fail() const
	    {
	    return is_flag_set(BAM_FQCFAIL);
	    }
	bool is_duplicate() const
	    {
	    return is_flag_set(BAM_FDUP);
	    }

	bool is_read1() const
	    {
	    return is_flag_set(BAM_FREAD1);
	    }
	bool is_read2() const
	    {
	    return is_flag_set(BAM_FREAD2);
	    }

    };
/**
 * Bam1Sequence
 */
class Bam1Sequence:public AbstractBam1Sequence
    {
    private:
    	 const bam1_t* _ptr;
    public:
	Bam1Sequence(const bam1_t* ptr):_ptr(ptr)
		{
		}
	virtual ~Bam1Sequence()
		{
		}
	virtual const bam1_t* ptr() const
	    {
	    return _ptr;
	    }

    };

class Bam1Record:public AbstractBam1Sequence
    {
    private:
       	 bam1_t* _ptr;
    public:
       	Bam1Record(const bam1_t* ptr):_ptr(::bam_dup1(ptr))
	    {
	    }
       	Bam1Record(const AbstractBam1Sequence* b):_ptr(::bam_dup1(b->ptr()))
       	    {

       	    }
       	Bam1Record(const Bam1Record& cp):_ptr(::bam_dup1(cp._ptr))
       	    {

       	    }

	virtual ~Bam1Record()
	    {
	    bam_destroy1(_ptr);
	    }

	Bam1Record& operator=(const Bam1Record& cp)
	    {
	    if(this!=&cp)
		{
		bam_destroy1(_ptr);
		_ptr=bam_dup1(cp._ptr);
		}
	    return *this;
	    }




	virtual const bam1_t* ptr() const
	    {
	    return _ptr;
	    }
	virtual bam1_t* ptr()
	    {
	    return _ptr;
	    }
    };

#endif /* BAM1SEQUENCE_H_ */
