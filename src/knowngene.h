#ifndef UCSC_KGENE_H
#define UCSC_KGENE_H
#include <vector>
#include <string>
#include <stdint.h>
#include <memory>
class KnownGene;

class Segment
    {
    protected:
	Segment();
    public:
	const KnownGene* gene;
	int32_t index;
	int32_t start;
	int32_t end;
	virtual ~Segment();
	int32_t size() const;
    	bool contains(int32_t position) const;
    	virtual bool isSplicingAcceptor(int32_t position) const=0;
    	virtual bool isSplicingDonor(int32_t position) const=0;
    	virtual bool isSplicing(int32_t position) const;
    	virtual std::string name() const=0;
    };
class Intron:public Segment
	{
	public:
		Intron();
		virtual ~Intron();
		virtual bool isSplicingAcceptor(int32_t position) const;
	    	virtual bool isSplicingDonor(int32_t position) const;
	    	virtual std::string name() const;
	};

class Exon:public Segment
	{
	public:
	    Exon();
	    virtual ~Exon();
	    std::auto_ptr<Intron> getNextIntron() const;
	    std::auto_ptr<Intron> getPrevIntron() const;
	    virtual bool isSplicingAcceptor(int32_t position) const;
	    virtual bool isSplicingDonor(int32_t position) const;
	    virtual std::string name() const;
	};



class KnownGene
	{
	public:
		std::string chrom;
		std::string name;
		std::string name2;
		char strand;
		int32_t txStart;
		int32_t txEnd;
		int32_t cdsStart;
		int32_t cdsEnd;
		std::vector<Exon> exons;
		const Exon* exon(int32_t  idx) const;
		std::auto_ptr<Intron> intron(int32_t  idx) const;
		int32_t countExons() const;
		bool isForward() const;
		int32_t getExonStart(int32_t  idx) const;
		int32_t getExonEnd(int32_t  idx) const;
		std::auto_ptr<std::string> getExonNameFromGenomicIndex(int32_t genome) const;
	};
#endif
