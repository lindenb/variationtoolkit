/*
 * Original code from wikipedia: http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm
 *
 */


#ifndef BOYERMOORE_H
#define BOYERMOORE_H
#include <climits>
#include <cstring>
#include <memory>
#include <stdint.h>
#include "abstractcharsequence.h"




class BoyerMoore
	{
	public:
		typedef int (*comparator_t)(int,int);
	private:
		 comparator_t comparator;
		 int32_t delta1[UCHAR_MAX];
		 int32_t* delta2;
		 std::string pat;
		 BoyerMoore():comparator(0),delta2(0)
			{
			}
		 int32_t patlen() const
			{
			return (int32_t)pat.size();
			}
		bool equals(int a,int b) const
			{
			return comparator==0?a==b:comparator(a,b)==0;
			}
	public:
		~BoyerMoore()
			{
			if(delta2!=0) delete delta2;
			}

		const char* pattern() const
			{
			return pat.c_str();
			}

		int32_t find(const AbstractCharSequence* sequence,int32_t begin) const
			{
			if(begin<0 || sequence->size()< this->patlen()) return -1;
			int32_t i = this->patlen()-1 + begin;
			const int32_t sequence_length=sequence->size();
			    while (i < sequence_length ) {
				int32_t j = this->patlen()-1;
				while (j >= 0 && equals(sequence->at(i),pat[j]))
					{
				    	--i;
				    	--j;
					}
				if (j < 0)
					{
				    	return i+1;
					}
					 
				i += std::max(delta1[(int32_t)sequence->at(i)], delta2[j]);
			    }
			    return -1;
			}
		int32_t find(const AbstractCharSequence* sequence) const
			{
			return find(sequence,0);
			}
	friend class BoyerMooreFactory;
	
	};

class BoyerMooreFactory
	{
	BoyerMoore::comparator_t comparator;	

	bool equals(int a,int b) const
		{
		return comparator==0?a==b:comparator(a,b)==0;
		}
// delta1 table: delta1[c] contains the distance between the last
// character of pat and the rightmost occurence of c in pat.
// If c does not occur in pat, then delta1[c] = patlen.
// If c is at string[i] and c != pat[patlen-1], we can
// safely shift i over by delta1[c], which is the minimum distance
// needed to shift pat forward to get string[i] lined up 
// with some character in pat.
// this algorithm runs in alphabet_len+patlen time.
void make_delta1(BoyerMoore* bm,const char* pat, int32_t patlen)
    {
    const int32_t NOT_FOUND=patlen;
    for (int32_t i=0; i < UCHAR_MAX; i++) {
        bm->delta1[i] = NOT_FOUND;
    }
    for (int32_t i=0; i < patlen-1; i++) {
        bm->delta1[(int32_t)pat[i]] = patlen-1 - i;
    }
}
 
// true if the suffix of word starting from word[pos] is a prefix 
// of word
bool is_prefix(const char *word, int32_t wordlen, int32_t pos) {
    int32_t suffixlen = wordlen - pos;

    for (int32_t i = 0; i < suffixlen; i++) {
        if (!equals(word[i],word[pos+i])) {
            return false;
        }
    }
    return true;
}
 
// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2
int32_t suffix_length(const char *word, int32_t wordlen, int32_t pos) {
    int32_t i=0;
    // increment suffix length i to the first mismatch or beginning
    // of the word
    for (i = 0; equals(word[pos-i],word[wordlen-1-i]) && (i < pos); i++);
    return i;
}
 
// delta2 table: given a mismatch at pat[pos], we want to align 
// with the next possible full match could be based on what we
// know about pat[pos+1] to pat[patlen-1].
//
// In case 1:
// pat[pos+1] to pat[patlen-1] does not occur elsewhere in pat,
// the next plausible match starts at or after the mismatch.
// If, within the substring pat[pos+1 .. patlen-1], lies a prefix
// of pat, the next plausible match is here (if there are multiple
// prefixes in the substring, pick the longest). Otherwise, the
// next plausible match starts past the character aligned with 
// pat[patlen-1].
// 
// In case 2:
// pat[pos+1] to pat[patlen-1] does occur elsewhere in pat. The
// mismatch tells us that we are not looking at the end of a match.
// We may, however, be looking at the middle of a match.
// 
// The first loop, which takes care of case 1, is analogous to
// the KMP table, adapted for a 'backwards' scan order with the
// additional restriction that the substrings it considers as 
// potential prefixes are all suffixes. In the worst case scenario
// pat consists of the same letter repeated, so every suffix is
// a prefix. This loop alone is not sufficient, however:
// Suppose that pat is "ABYXCDEYX", and text is ".....ABYXCDEYX".
// We will match X, Y, and find B != E. There is no prefix of pat
// in the suffix "YX", so the first loop tells us to skip forward
// by 9 characters.
// Although superficially similar to the KMP table, the KMP table
// relies on information about the beginning of the partial match
// that the BM algorithm does not have.
//
// The second loop addresses case 2. Since suffix_length may not be
// unique, we want to take the minimum value, which will tell us
// how far away the closest potential match is.
void make_delta2(BoyerMoore* bm, const char *pat, int32_t patlen)
	{
    int32_t last_prefix_index = patlen-1;
 
    // first loop
    for (int32_t p=patlen-1; p>=0; p--) {
        if (is_prefix(pat, patlen, p+1)) {
            last_prefix_index = p+1;
        }
        bm->delta2[p] = last_prefix_index + (patlen-1 - p);
    }
 
    // second loop
    for (int32_t p=0; p < patlen-1; p++) {
        int32_t slen = suffix_length(pat, patlen, p);
        if (!equals(pat[p - slen], pat[patlen-1 - slen])) {
            bm->delta2[patlen-1 - slen] = patlen-1 - p + slen;
        }
    }
}

public:
	BoyerMooreFactory():comparator(0)
		{
		}
	void set_comparator(BoyerMoore::comparator_t cmp)
		{
		this->comparator=cmp;
		}
	std::auto_ptr<BoyerMoore> compile(const char *pat, uint32_t patlen)
		{
		BoyerMoore* bm=new BoyerMoore(); 
		bm->pat.assign(pat,patlen);
		bm->comparator=this->comparator;
		bm->delta2 = new int[patlen];
		make_delta1(bm, pat,(int32_t)patlen);
		make_delta2(bm, pat,(int32_t)patlen);
		return std::auto_ptr<BoyerMoore>(bm);
		}
	std::auto_ptr<BoyerMoore> compile(const char *pat)
		{
		return compile(pat,std::strlen(pat));
		}
	};


#endif

