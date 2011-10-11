/*
 * ttview.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */

#ifndef TTVIEW_H_
#define TTVIEW_H_
#include <iostream>
#include <string>
#include <memory>
#include <cstdarg>
#include <stdint.h>
#include "xbam.h"
#include "xfaidx.h"

class TTView
    {
    public:
	/* number of columns */
	int mcol;

	TTView();
	virtual ~TTView();
	void print(
		std::ostream& out,
		const char* chrom,
		int32_t pos,
		BamFile* bam,
		IndexedFasta* faidx
		);
    private:

	/* I put the characters in a structure. If one day I want to add colors... */
	struct CPixel
		{
		char c;
		};
	/* number of lines created so far */
	int nLines;
	CPixel** screen;
	/* param */
	void* lplbuf;

	int curr_tid, left_pos;
	void* bca;
	int ccol, last_pos, row_shift, base_for, color_for, is_dot, ins, no_skip, show_name;
	std::auto_ptr<std::string> ref;
	IndexedFasta* fai;
	BamFile* mybam;


	void clear();
	CPixel* getchxy(int y,int x);
	void putchxy(int y,int x,char c);
	void dump(std::ostream& out);
	int ttv_draw_aln(int tid, int pos);
	void printfyx(int y,int x,const char* fmt,...);
	int callback(uint32_t tid, uint32_t pos, int n, const void *bampileup);
	static int ttv_pl_func(uint32_t tid, uint32_t pos, int n, const void *pl, void *data);
	static int ttv_fetch_func(const void *b, void *data);
    };

#endif /* TTVIEW_H_ */
