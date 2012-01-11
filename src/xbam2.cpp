/*
 * xbam.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */
#include <cerrno>
#include <cassert>
#define NOWHERE
#include "where.h"

#include "throw.h"
#include "xbam2.h"

using namespace std;

#define CASTBAM(a) (a)
#define CASTIDX(a) (a)
#define CASTHEAD(a) (a)

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}

/***************************************************************************************/
/***************************************************************************************/

const CigarOp CigarOp::MATCH('M');
const CigarOp CigarOp::INSERT('I');

const CigarOp* CigarOp::find(char c)
    {
    switch(c)
	{
	case 'M': return &MATCH;
	case 'I': return &INSERT;
	}
    return 0;
    }

CigarOp::CigarOp(char c)
    {

    }


/***************************************************************************************/
/***************************************************************************************/

SAMRecord::SAMRecord()
    {
    }

SAMRecord::~SAMRecord()
    {
    }

bool SAMRecord::isReverseStrand() const
    {
    return !isForwardStrand();
    }

#define BAM_GETTER(fun,opcode) bool SAMRecord::fun() const \
    {\
    return (this->flag() & opcode)!=0;\
    }
BAM_GETTER(isRead1,BAM_FREAD1)
BAM_GETTER(isRead2,BAM_FREAD2)
BAM_GETTER(isProperPair,BAM_FPROPER_PAIR)
BAM_GETTER(isUnmapped,BAM_FUNMAP)
BAM_GETTER(isQCFail,BAM_FQCFAIL)
BAM_GETTER(isPaired,BAM_FPAIRED)
BAM_GETTER(isDuplicate,BAM_FDUP)
#undef BAM_GETTER

char SAMRecord::at(int32_t idx) const
    {
    return bam_nt16_rev_table[bam1_seqi(_seq(),idx)];
    }


/***************************************************************************************/
/***************************************************************************************/


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
    private:
	bam1_t* _ptr;
    };

DelegateSAMRecord::DelegateSAMRecord(bam1_t* ptr):_ptr(ptr)
    {
    }

DelegateSAMRecord::~DelegateSAMRecord()
    {
    }

const bam1_t* DelegateSAMRecord::ptr() const
    {
    return _ptr;
    }

const bam1_core_t* DelegateSAMRecord::core() const
    {
    return &(ptr()->core);
    }

const uint8_t* DelegateSAMRecord::_seq() const
    {
    return bam1_seq(ptr());
    }

const char* DelegateSAMRecord::readName() const
    {
    return bam1_qname(ptr());
    }

bool DelegateSAMRecord::isForwardStrand() const
    {
    return (bam1_strand(ptr())==0);//equal
    }


int32_t DelegateSAMRecord::flag() const
    {
    return core()->flag;
    }


int32_t DelegateSAMRecord::size() const
    {
    return core()->l_qseq;
    }

int32_t DelegateSAMRecord::tid() const
    {
    return core()->tid;
    }

int32_t DelegateSAMRecord::pos() const
    {
    return core()->pos;
    }

/***************************************************************************************/
/***************************************************************************************/


BamFile2::BamFile2(const char* file) :fp(NULL),header(NULL),index(NULL),filename(file)
    {

    }

// callback for bam_fetch()
int BamFile2::fetch_func(const bam1_t *b, void *data)
	{
	bam_plbuf_t *buf = (bam_plbuf_t*)data;
	bam_plbuf_push(b, buf);
	return 0;
	}

bool BamFile2::is_open() const
    {
    return fp!=0;
    }

void BamFile2::open(bool load_index)
    {
    close();
    bamFile f=::bam_open(filename.c_str(), "r");
    if(f==NULL) THROW("Cannot open bam file \""<< filename << "\" : " << strerror(errno));
    fp=f;

    bam_header_t *h= ::bam_header_read(f);
    if(h==NULL)
	{
	close();
	THROW("Cannot read header for "<< filename);
	}
    ::bam_init_header_hash(h);
    this->header=h;

    if(load_index)
	{
	bam_index_t *i= ::bam_index_load(filename.c_str());
	if(i==NULL)
	    {
	    close();
	    THROW("Cannot read index for "<< filename);
	    }
	this->index=i;
	}
    }
BamFile2::~BamFile2()
    {
    close();
    }

void BamFile2::close()
    {
    if(this->index!=NULL) ::bam_index_destroy(CASTIDX(index));
    this->index=0;
    if(this->header!=NULL) ::bam_header_destroy(CASTHEAD(header));
    this->header=0;
    if(this->fp!=NULL) ::bam_close(CASTBAM(fp));
    this->fp=0;
    }

const char* BamFile2::findNameByTid(int32_t tid)
    {
    assert(header!=NULL);
    assert(tid>=0);
    if(tid<0 || tid>= CASTHEAD(header)->n_targets) return NULL;
    return CASTHEAD(header)->target_name[tid];
    }

int32_t BamFile2::findTidByName(const char* seq_name)
    {
    assert(header!=NULL);
    assert(seq_name!=NULL);
    assert(CASTHEAD(header)->hash!=NULL);
    return ::bam_get_tid(CASTHEAD(header), seq_name);
    }

bamFile BamFile2::bamPtr() { return this->fp;}
bam_index_t* BamFile2::bamIndex() { return this->index;}
bam_header_t* BamFile2::bamHeader() { return this->header;}

int32_t BamFile2::count_targets() const
    {
    return header->n_targets;
    }
const char* BamFile2::target_name(int32_t n) const
    {
    assert(n< header->n_targets);
    return header->target_name[n];
    }
int32_t BamFile2::target_length(int32_t n) const
    {
    assert(n< header->n_targets);
    return (int32_t) header->target_len[n];
    }

const char* BamFile2::path() const
    {
    return this->filename.c_str();
    }

std::auto_ptr<vector<BamFile2::Target> > BamFile2::targets()
    {
    std::vector<BamFile2::Target>* v=new vector<BamFile2::Target>;
    v->reserve(header->n_targets);
    for(int32_t n=0;n< count_targets();++n)
	{
	v->push_back(Target(n,target_name(n),target_length(n)));
	}
    return std::auto_ptr<vector<BamFile2::Target> >(v);
    }

BamFile2::Target::Target(int32_t tid,const char* name,uint32_t length):
	_tid(-1),_name(name),_length(length)
    {

    }

BamFile2::Target::Target(const Target& cp):
	_tid(cp._tid),_name(cp._name),_length(cp._length)
    {

    }
BamFile2::Target::~Target()
    {

    }
BamFile2::Target& BamFile2::Target::operator=(const BamFile2::Target& cp)
    {
    if(this!=&cp)
	{
	_tid=cp._tid;
	_name.assign(cp._name);
	_length=cp._length;
	}
    return *this;
    }

int32_t BamFile2::Target::tid() const
    {
    return _tid;
    }
const std::string& BamFile2::Target::name() const
    {
    return _name;
    }
uint32_t BamFile2::Target::length() const
    {
    return _length;
    }


std::auto_ptr<BamFile2::Iterator>
BamFile2::query(int tid, int beg0, int end0)
    {
    return std::auto_ptr<BamFile2::Iterator>(new  BamFile2::Iterator(
	 ::bam_iter_query(CASTIDX(index), tid, beg0,end0),
	  this
	));
    }


std::auto_ptr<BamFile2::Iterator>
BamFile2::query(int tid)
    {
    if(tid<0 || tid>=count_targets()) THROW("BOUM");
    return query(tid,0,target_length(tid));
    }

std::auto_ptr<BamFile2::Iterator>
BamFile2::query()
    {
    return std::auto_ptr<BamFile2::Iterator>(new  BamFile2::Iterator(
   	 0,
   	  this
   	));
    }


/*****************************************************************************/
BamFile2::Iterator::Iterator(bam_iter_t iter,BamFile2* owner):
	_iter(iter),_owner(owner),_rec(0),_sam(0)
    {
    _rec=bam_init1();
    if(_rec==0) THROW("bam_init1 failed");
    }

void BamFile2::Iterator::close()
    {
    WHERE("");
    if(_iter!=0) ::bam_iter_destroy(_iter);
    _iter=0;
    if(_rec!=0) bam_destroy1(_rec);
    _rec=0;
    if(_sam!=0) delete _sam;
    _sam=0;
    }

const SAMRecord*  BamFile2::Iterator::next()
    {
    if(_rec==0) return false;
    bool ok=false;
    if (_iter!=0)
	{
	WHERE("_iter!=0");
	ok= bam_iter_read(
	    CASTBAM(_owner->fp),
	    _iter,
	    _rec)>=0;
	}
    else
	{
	WHERE("_iter==0");
	ok= bam_read1(CASTBAM(_owner->fp),_rec)>=0;
	}

    if(!ok)
	{
	close();
	return 0;
	}
    if(_sam!=0) delete _sam;
    _sam=new DelegateSAMRecord(_rec);
    return _sam;
    }

BamFile2::Iterator::~Iterator()
    {
    this->close();
    }

#ifdef TEST_THIS_CODE
int main(int argc,char** argv)
    {
    for(int i=1;i< argc;++i)
	{
	BamFile2 f(argv[i]);
	f.open();
	for(int j=0;j< f.count_targets();++j)
	    {
	    if(j==0) continue;
	    int n=0;
	    auto_ptr<BamFile2::Iterator> iter=f.query(j,0,99999999);
	    const SAMRecord* rec;
	    while(++n<10 && (rec=iter->next())!=0)
		{
		cout << argv[i] << " "<< n << " " << rec->tid() << ":"<< rec->pos() << " " << rec->readName() << endl;
		}

	    n=0;
	    iter->close();
	    iter=f.query(23);

	    while(++n<10 && (rec=iter->next())!=0)
		{
		cout << "#2" << argv[i] << " "<< n << " " << rec->tid() << ":"<< rec->pos() << " " << rec->readName() << endl;
		}
	    break;
	    }
	f.close();
	}
    return 0;
    }

#endif
