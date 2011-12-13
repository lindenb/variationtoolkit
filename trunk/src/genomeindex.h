/*
 * genomeindex.h
 *
 *  Created on: Dec 12, 2011
 *      Author: lindenb
 */

#ifndef GENOMEINDEX_H_
#define GENOMEINDEX_H_
#include <string>
#include <vector>
#include <stdint.h>
#include "abstractcharsequence.h"
#include "auto_vector.h"

class GenomeIndex
    {

    public:
	class Chromosome:public AbstractCharSequence
		    {
		    public:
			uint8_t id;
			std::string name;
			int32_t non_N;
			int32_t heading_N;
			int32_t trailing_N;
			char* sequence;
			Chromosome();
			virtual ~Chromosome();
			virtual char at(int32_t index) const;
			virtual int32_t size() const;
		    };
		struct Reference
		    {
		    uint8_t chrom_id;
		    int32_t pos;
		    };


	GenomeIndex();
	~GenomeIndex();
	void readIndex(const char* filename);
	void writeIndex(const char* filename);
	bool lt(const Reference& o1,  const Reference& o2) const;
	const Chromosome* getChromsomeByIndex(uint8_t id) const;
	void readGenome(const char* fasta);
	void _createindex();

	int32_t short_read_max_size;
	auto_vector<Chromosome> chromosomes;
	std::vector<Reference> gIndex;
   private:
	std::FILE* merge(std::FILE* io,std::vector<GenomeIndex::Reference>& gIndex);
	uint32_t merge_sort_buff_size;
	void print(std::ostream& out,const Reference& ref) const;
    };

#endif /* GENOMEINDEX_H_ */
