/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Orginal source code:
 *	bam_tview.c in http://samtools.sourceforge.net/
 *	Authors: Heng Li, Bob Handsaker, Jue Ruan, Colin Hercus, Petr Danecek
 * Contact:
 *	plindenbaum@yahoo.fr
 * Reference:
 *	http://plindenbaum.blogspot.com/2011/07/text-alignment-viewer-using-samtools.html
 * WWW:
 *	http://plindenbaum.blogspot.com
 *	http://samtools.sourceforge.net/
 * Motivation:
 *	Text alignment viewer using the samtools API
 */
#include <cctype>
#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdarg>
#include <cerrno>
#include <limits>
#include "throw.h"
#include "bam.h"
#include "faidx.h"
#include "bam2bcf.h"
#include "ttview.h"

char bam_aux_getCEi(bam1_t *b, int i);
char bam_aux_getCSi(bam1_t *b, int i);
char bam_aux_getCQi(bam1_t *b, int i);

#define TV_MIN_ALNROW 2
#define TV_MAX_GOTO  40
#define TV_LOW_MAPQ  10

#define TV_COLOR_MAPQ   0
#define TV_COLOR_BASEQ  1
#define TV_COLOR_NUCL   2
#define TV_COLOR_COL    3
#define TV_COLOR_COLQ   4

#define TV_BASE_NUCL 0
#define TV_BASE_COLOR_SPACE 1




TTView::TTView():nLines(0),screen(NULL),lplbuf(NULL),bca(NULL)
    {
    this->is_dot = 1;
    this->lplbuf = ::bam_lplbuf_init((bam_pileup_f)TTView::ttv_pl_func, this);
    this->bca = ::bcf_call_init(0.83, 13);
    this->ins = 1;
    this->mcol = 80;
    this->color_for = TV_COLOR_MAPQ;
    }


TTView::CPixel* TTView::getchxy(int y,int x)
    {
    int i=0;
    assert(y>=0);
    if(y<0 || x<0 || x>= this->mcol ) return NULL;
    while(this->nLines<=y)
	    {
	    this->screen=(CPixel**)realloc(this->screen,sizeof(CPixel*)*(this->nLines+1));
	    if(this->screen==NULL)
		    {
		    THROW("Out of memory");
		    }
	    this->screen[this->nLines]=(CPixel*)malloc(this->mcol*sizeof(CPixel));
	    if(this->screen[this->nLines]==NULL)
		    {
		    THROW("Out of memory");
		    }
	    for(i=0;i< this->mcol;++i)
		    {
		    this->screen[this->nLines][i].c=' ';
		    }
	    this->nLines++;
	    }


    return &(this->screen[y][x]);
    }

void TTView::putchxy(int y,int x,char c)
    {
    CPixel* pixel=this->getchxy(y,x);
    if(pixel==NULL)
	    {
	    return;
	    }
    pixel->c=c;
    }

void TTView::printfyx(int y,int x,const char* fmt,...)
	{
	va_list ap;
	int i=0;
	char* buffer=NULL;

	buffer=(char*)malloc(this->mcol+1);
	if(buffer==NULL)
		{
		THROW("Out of memory");
		}
	memset(buffer,'\0',sizeof(char)*(this->mcol+1));

	va_start(ap,fmt);
	vsnprintf(buffer,this->mcol,fmt,ap);
	va_end(ap);
	while(x+i< this->mcol && buffer[i]!=0)
		{
		this->putchxy(y,x+i,buffer[i]);
		++i;
		}
	free(buffer);
	}

void TTView::dump(std::ostream& out)
	{
	int y;
	int x;
	for(y=0;y< this->nLines;++y)
		{
		for(x=0;x< this->mcol;++x)
			{
			out << this->screen[y][x].c;
			}
		out << std::endl;
		}
	}




int TTView::callback(uint32_t tid, uint32_t pos, int n, const void *bampileup)
	{
	extern unsigned char bam_nt16_table[256];
	const bam_pileup1_t *pl=(const bam_pileup1_t *)bampileup;
	int i, j, c, rb, max_ins = 0;
	uint32_t call = 0;
	if((int)pos < this->left_pos || this->ccol > this->mcol) return 0; // out of screen
	// print reference
	rb = (this->ref.get()!=NULL && pos - this->left_pos < this->ref->size())? this->ref->at(pos - this->left_pos) : 'N';
	for(i = this->last_pos + 1; i < (int)pos; ++i)
		{
		if (i%10 == 0 && this->mcol - this->ccol >= 10)
		    {
		    printfyx(0, this->ccol, "%-d", i+1);
		    }
		c = this->ref.get()!=NULL ? this->ref->at(i - this->left_pos) : 'N';
		putchxy(1, this->ccol++, c);
		}
	if (pos%10 == 0 && this->mcol - this->ccol >= 10) printfyx(0, this->ccol, "%-d", pos+1);
	{ // call consensus
		bcf_callret1_t bcr;
		int qsum[4], a1, a2, tmp;
		double p[3], prior = 30;
		::bcf_call_glfgen(n, pl, bam_nt16_table[rb], (bcf_callaux_t*)this->bca, &bcr);
		for (i = 0; i < 4; ++i) qsum[i] = bcr.qsum[i]<<2 | i;
		for (i = 1; i < 4; ++i) // insertion sort
			for (j = i; j > 0 && qsum[j] > qsum[j-1]; --j)
				tmp = qsum[j], qsum[j] = qsum[j-1], qsum[j-1] = tmp;
		a1 = qsum[0]&3; a2 = qsum[1]&3;
		p[0] = bcr.p[a1*5+a1]; p[1] = bcr.p[a1*5+a2] + prior; p[2] = bcr.p[a2*5+a2];
		if ("ACGT"[a1] != toupper(rb)) p[0] += prior + 3;
		if ("ACGT"[a2] != toupper(rb)) p[2] += prior + 3;
		if (p[0] < p[1] && p[0] < p[2]) call = (1<<a1)<<16 | (int)((p[1]<p[2]?p[1]:p[2]) - p[0] + .499);
		else if (p[2] < p[1] && p[2] < p[0]) call = (1<<a2)<<16 | (int)((p[0]<p[1]?p[0]:p[1]) - p[2] + .499);
		else call = (1<<a1|1<<a2)<<16 | (int)((p[0]<p[2]?p[0]:p[2]) - p[1] + .499);
	}
	
	c = ",ACMGRSVTWYHKDBN"[call>>16&0xf];
	i = (call&0xffff)/10+1;
	if (i > 4) i = 4;
	if (c == toupper(rb)) c = '.';
	putchxy(2, this->ccol, c);
	if(this->ins) {
		// calculate maximum insert
		for (i = 0; i < n; ++i) {
			const bam_pileup1_t *p = pl + i;
			if (p->indel > 0 && max_ins < p->indel) max_ins = p->indel;
		}
	}
	// core loop
	for (j = 0; j <= max_ins; ++j) {
		for (i = 0; i < n; ++i) {
			const bam_pileup1_t *p = pl + i;
			int row = TV_MIN_ALNROW + p->level - this->row_shift;
			if (j == 0) {
				if (!p->is_del) {
					if (this->base_for == TV_BASE_COLOR_SPACE &&
							(c = bam_aux_getCSi(p->b, p->qpos))) {
						c = bam_aux_getCSi(p->b, p->qpos);
						// assume that if we found one color, we will be able to get the color error
						if (this->is_dot && '-' == bam_aux_getCEi(p->b, p->qpos)) c = bam1_strand(p->b)? ',' : '.';
					} else {
						if (this->show_name) {
							char *name = bam1_qname(p->b);
							c = (p->qpos + 1 >= p->b->core.l_qname)? ' ' : name[p->qpos];
						} else {
							c = bam_nt16_rev_table[bam1_seqi(bam1_seq(p->b), p->qpos)];
							if (this->is_dot && toupper(c) == toupper(rb)) c = bam1_strand(p->b)? ',' : '.';
						}
					}
				} else c = p->is_refskip? (bam1_strand(p->b)? '<' : '>') : '*';
			} else { // padding
				if (j > p->indel) c = '*';
				else { // insertion
					if (this->base_for ==  TV_BASE_NUCL) {
						if (this->show_name) {
							char *name = bam1_qname(p->b);
							c = (p->qpos + j + 1 >= p->b->core.l_qname)? ' ' : name[p->qpos + j];
						} else {
							c = bam_nt16_rev_table[bam1_seqi(bam1_seq(p->b), p->qpos + j)];
							if (j == 0 && this->is_dot && toupper(c) == toupper(rb)) c = bam1_strand(p->b)? ',' : '.';
						}
					} else {
						c = bam_aux_getCSi(p->b, p->qpos + j);
						if (this->is_dot && '-' == bam_aux_getCEi(p->b, p->qpos + j)) c = bam1_strand(p->b)? ',' : '.';
					}
				}
			}
			if (row > TV_MIN_ALNROW
				/* && row < this->mrow */
				)
				{
				int x;
				
				
				if (this->color_for == TV_COLOR_BASEQ)
					{
					x = bam1_qual(p->b)[p->qpos]/10 + 1;
					if (x > 4) x = 4;	
					} 
				else if (this->color_for == TV_COLOR_MAPQ)
					{
					x = p->b->core.qual/10 + 1;
					if (x > 4) x = 4;
					}	
				else if (this->color_for == TV_COLOR_NUCL)
					{
					x = bam_nt16_nt4_table[bam1_seqi(bam1_seq(p->b), p->qpos)] + 5;
					}	
				else if(this->color_for == TV_COLOR_COL)
					{
					x = 0;
					switch(bam_aux_getCSi(p->b, p->qpos))
						{
						case '0': x = 0; break;
						case '1': x = 1; break;
						case '2': x = 2; break;
						case '3': x = 3; break;
						case '4': x = 4; break;
						default: x = bam_nt16_nt4_table[bam1_seqi(bam1_seq(p->b), p->qpos)]; break;
						}
					x+=5;
					}	
				else if(this->color_for == TV_COLOR_COLQ)
					{
					x = bam_aux_getCQi(p->b, p->qpos);
					if(0 == x) x = bam1_qual(p->b)[p->qpos];
					x = x/10 + 1;
					if (x > 4) x = 4;
					}
				
				putchxy(row, this->ccol, bam1_strand(p->b)? tolower(c) : toupper(c));
				
			}
		}
		c = j? '*' : rb;
		if (c == '*') {
			
			
			putchxy(1, this->ccol++, c);
			
		} else putchxy(1, this->ccol++, c);
	}
	this->last_pos = pos;
	return 0;
	}

int TTView::ttv_pl_func(uint32_t tid, uint32_t pos, int n, const void *pl, void *data)
	{
	TTView *tv = (TTView*)data;
	return tv->callback(tid,pos,n,pl);
	}


void TTView::clear()
	{
	int i=0;
	for(i=0;i< this->nLines;++i)
		{
		free(this->screen[i]);
		}

	free(this->screen);
	this->screen=NULL;
	this->nLines=0;
	ref.reset();
	}

TTView::~TTView()
	    {
	    clear();
	    if(this->lplbuf!=NULL) ::bam_lplbuf_destroy((bam_lplbuf_t*)this->lplbuf);
	    if(this->bca!=NULL) bcf_call_destroy((bcf_callaux_t*)this->bca);

	    }

int TTView::ttv_fetch_func(const void *b1, void *data)
	{
	const  bam1_t *b=(bam1_t*)b1;
	TTView *tv = (TTView*)data;
	if (tv->no_skip)
		{
		uint32_t *cigar = bam1_cigar(b); // this is cheating...
		int i;
		for (i = 0; i <b->core.n_cigar; ++i) {
			if ((cigar[i]&0xf) == BAM_CREF_SKIP)
				cigar[i] = cigar[i]>>4<<4 | BAM_CDEL;
		}
		}
	::bam_lplbuf_push(b, (bam_lplbuf_t*)(tv->lplbuf));
	return 0;
	}

int TTView::ttv_draw_aln(int tid, int pos)
	{
	// reset
	clear();
	this->curr_tid = tid;
	this->left_pos = pos;
	this->last_pos = this->left_pos - 1;
	this->ccol = 0;
	// print ref and consensus
	if (this->fai!=NULL)
		{
		this->ref.reset();
		this->ref = this->fai->fetch(
			this->mybam->findNameByTid(this->curr_tid),
			this->left_pos + 1,
			this->left_pos + this->mcol
			);
		}
	// draw aln
	::bam_lplbuf_reset((bam_lplbuf_t*)this->lplbuf);
	::bam_fetch(
		(bamFile)(this->mybam->fp),
		(const bam_index_t*)(this->mybam->index),
		this->curr_tid,
		this->left_pos,
		this->left_pos + this->mcol,
		this,
		(bam_fetch_f)TTView::ttv_fetch_func
		);
	::bam_lplbuf_push(0, (bam_lplbuf_t*)this->lplbuf);

	while (this->ccol < this->mcol)
		{
		int pos = this->last_pos + 1;
		if (pos%10 == 0 && this->mcol - this->ccol >= 10)
		    {
		    printfyx(0, this->ccol, "%-d", pos+1);
		    }
		putchxy(1,
			this->ccol++,
			(this->ref.get()!=NULL && pos < (int)this->ref->size())? this->ref->at(pos - this->left_pos) : 'N');
		++this->last_pos;
		}
	return 0;
	}

void TTView::print(
	std::ostream& out,
	const char* chrom,
	int32_t pos,
	BamFile* bam,
	IndexedFasta* faidx
	)
    {
    this->mybam=bam;
    this->fai=faidx;
    int32_t idx=mybam->findTidByName(chrom);
    if(idx!=-1)
	{
	this->ttv_draw_aln(idx,pos);
	}
    dump(out);
    clear();
    this->mybam=NULL;
    this->fai=NULL;
    }

