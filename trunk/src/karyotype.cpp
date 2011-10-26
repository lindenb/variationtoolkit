/*
 * karyotype.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdint.h>
#include "auto_vector.h"
#include "smartcmp.h"
#include "tokenizer.h"
#include "shapes.h"
#include "color.h"

using namespace std;

class Genome;
class Chromosome;

typedef double pixel_t;

struct Config
	{
	std::ostream* out;
	int nLines;
	pixel_t x;
	pixel_t y;
	pixel_t width;
	pixel_t height;
	};


class Cytoband:public Rectangle
	{
	public:
		Chromosome* chrom;
		Rectangle bounds;
		std::string name;
		std::string stain;
		int32_t start;
		int32_t end;
		std::auto_ptr<Color> color()
				{
				Color* c=NULL;
				return std::auto_ptr<Color>(c);
				}
		void paint(Config* cfg)
			{
			std::ostream out=*(cfg->out);
			auto_ptr<Color> c=color();
			if(c.get()!=NULL)
				{
				out << (int)c->r << " "<< (int)c->g << " "<< (int)c->b << "\n";
				out << "1 1 1\n";
				out << bounds.x << " "<< bounds.y << " "
					<< bounds.width << " "<< bounds.height
					<< " "
					<< "vgradient\n";
				}

			}
	};

struct Data
	{
	int32_t start;
	int32_t end;
	uint32_t rgbcolor;
	};

class Chromosome
	{
	public:
		Genome* genome;
		std::string name;
		int32_t length;
		Rectangle bounds;
		std::auto_vector<Cytoband> bands;
		std::vector<Data> data;

		pixel_t convertToY(int pos)
			{
			return bounds.y+((double)pos/(double)(genome->longest->length))*bounds.height;
			}

		void paint(Config* cfg)
			{
			pixel_t chrom_width=10.0;
			std::ostream out=*(cfg->out);
			for(size_t i=0;i< bands.size();++i)
				{
				Cytoband* band=bands.at(i);
				band->bounds=0;
				band->paint(cfg);
				}
			for(size_t i=0;i< data.size();++i)
				{

				}
			}
	};

class Genome
	{
	public:
		std::string name;
		map<string,Chromosome*,SmartComparator> name2chrom;
		Chromosome* longest;
		Rectangle bounds;

		void paint(Config* cfg)
			{
			std::ostream out=*(cfg->out);
			int chromindex=0;
			int num_chrom_per_line=(int)ceil(name2chrom.size()/(float)cfg->nLines);
			int nLines=0;
			pixel_t one_chrom_width= cfg->width/num_chrom_per_line;
			pixel_t one_chrom_height;
			pixel_t y=cfg->y;
			for(map<string,Chromosome*,SmartComparator>::iterator r=name2chrom.begin();
				r!=name2chrom.end();++r)
				{
				Config cfg2(*cfg);

				chromindex++;
				if(chromindex%num_chrom_per_line==0)
					{
					y+=one_chrom_height;
					}
				}
			}


		void read(std::istream& in)
			{
			longest=NULL;
			vector<string> tokens;
			Tokenizer tokenizer;
			string line;
			while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				tokenizer.split(line,tokens);
				if(tokens.size()!=5) THROW("bad cytoband file "<< line);
				map<string,Chromosome*,SmartComparator>::iterator r=name2chrom.find(tokens[0]);
				Chromosome* chrom=NULL;
				if(r==name2chrom.end())
					{
					chrom=new Chromosome;
					chrom->genome=this;
					chrom->name.asssign(tokens[0]);
					chrom->length=0;
					name2chrom.insert(make_pair<string,Chromosome*>(chrom->name,chrom));
					}
				else
					{
					chrom=r->second;
					}
				Cytoband* cyto=new Cytoband;
				cyto->chrom=chrom;
				cyto->name.assign(tokens[0]);
				cyto->start=(int32_t)strtol(tokens[1].c_str(),0,10);
				cyto->end=(int32_t)strtol(tokens[2].c_str(),0,10);
				chrom->length=max(cyto->start,chrom->length);
				chrom->bands.push_back(cyto);
				if(longest==NULL || longest->length< chrom->length)
					{
					longest=chrom;
					}
				}
			}
	};

class Karyotype
	{
	public:

	};
