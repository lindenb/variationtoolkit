/*
 * ucscgenesps.cpp
 *
 *  Created on: Oct 18, 2011
 *      Author: lindenb
 */
#include <cstdlib>
#include <map>
#include <set>
#include <cerrno>
#include <ctime>
#include <string>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdint.h>
#include "mysqlapplication.h"
#include "geneticcode.h"
#include "zstreambuf.h"
#include "xfaidx.h"
#include "tokenizer.h"
#include "knowngene.h"
#define NOWHERE
#include "where.h"
#include "auto_vector.h"
#include "smartcmp.h"

using namespace std;

class GenePostcript:public MysqlApplication
	{
	public:
	class Cluster
			{
			public:
					auto_vector<KnownGene> genes;
					set<int32_t> positions;
					map<string,set<int32_t>,SmartComparator > sample2positions;
					int32_t chromStart;
					int32_t chromEnd;
					void clear()
						{
						genes.clear();
						chromStart=-1;
						chromEnd=-1;
						positions.clear();
						sample2positions.clear();
						}
			};

		map<string,auto_vector<KnownGene> > chrom2genes;
		int chromCol;
		int posCol;
		int sampleCol;
		Cluster cluster;
		float margin_left;
		float margin_right;
		float margin_top;
		float margin_bottom;
		float page_width;
		float page_height;
		float fHeight;
		int count_pages_printed;


		GenePostcript():MysqlApplication()
			{
			chromCol=0;
			posCol=1;
			sampleCol=-1;
			page_width=900;
			page_height=700;
			margin_left=200;
			margin_right=50;
			margin_top=100;
			margin_bottom=50;
			fHeight=20;
			count_pages_printed=0;
			}

		float pageWidth()
			{
			return page_width;
			}

		float pageHeight()
			{
			return page_height;
			}

		float width()
			{
			return pageWidth()-(margin_left+margin_right);
			}

		float height()
			{
			return pageHeight()-(margin_top+margin_bottom);
			}

		double toPixel(int32_t pos)
			{
			return (double)margin_left+((pos-(double)cluster.chromStart)/((double)cluster.chromEnd-(double)cluster.chromStart))*(double)width();
			}
#define MOVETO(x,y) cout << x << " " << y << " moveto\n"
#define LINETO(x,y) cout << x << " " << y << " lineto\n"
#define STROKE cout << "stroke\n"
#define NEWPATH cout << "newpath\n"
		void print()
			{
			if(cluster.positions.empty())
				{
				cluster.clear();
				return;
				}
			++count_pages_printed;
			cout << "\n%%Page: " << count_pages_printed << " "<<count_pages_printed << "\n";
			WHERE(cluster.genes.size());
			double fHeight=20;

			float midy=fHeight/2.0f;

			double cdsHeight=fHeight*0.4;
			double exonHeight=fHeight*0.9;

			cout << "2 " << (pageHeight()-10 )<<" moveto (" << cluster.genes.back()->chrom << ":"
					<< cluster.chromStart << "-"
					<< cluster.chromEnd << ") show\n"
					;


			cout << "1 0 0 setrgbcolor\n";
			cout << "0.3 setlinewidth\n";
			for(set<int32_t>::iterator r=cluster.positions.begin();
					r!=cluster.positions.end();
					++r)
				{
				cout << "newpath "<< toPixel(*r) << " 0 moveto 0 "<< pageHeight() << " rlineto stroke\n";
				cout << toPixel(*r) << " " << (pageHeight()-5) <<" moveto -90 rotate (" << (*r) << ") show 90 rotate\n";
				}



			for(size_t i=0;i< cluster.genes.size();++i)
				{
				KnownGene* g=cluster.genes.at(i);
				cout << "gsave\n";
				cout <<"0 "<< pageHeight()-margin_top-(fHeight*i)<<" translate\n";


				double x1=toPixel(g->txStart);
				double x2=toPixel(g->txEnd);
				cout << "0 0 0 setrgbcolor\n";
				NEWPATH;
				MOVETO(x1,midy);
				LINETO(x2,midy);
				STROKE;
				//draw ticks

				cout << "0.2 setlinewidth\n";
				cout << "newpath\n";
				cout << x1 <<" " << midy << " moveto\n";
				cout << x1 << " " << x2 << (g->isForward()?" forticksF":" forticksR")<< endl;
				cout << "closepath stroke\n";


				cout << "0.5 setlinewidth\n";
				//draw txStart/txEnd
				cout << "0.1 0.1 0.5 setrgbcolor\n"
						"newpath\n"
						<< toPixel(g->cdsStart) << " "
						<< (midy-cdsHeight/2.0) <<" "
						<< (toPixel(g->cdsEnd)-toPixel(g->cdsStart)) << " "
						<< cdsHeight << " box closepath fill\n"
						;
				//draw each exon
				for(int32_t j=0;j< g->countExons();++j)
					{
					cout << toPixel(g->exon(j)->start) << " "
						 << (midy-exonHeight/2.0)<< " "
						 << (toPixel(g->exon(j)->end)-toPixel(g->exon(j)->start)) << " "
						 << exonHeight << " gradient\n"
						 ;
					}
				//draw name
				cout << "0 0 0 setrgbcolor\n";
				cout << "10 " << midy << " moveto (" << g->name << ") show\n";
				cout << "grestore\n";
				}
			if(sampleCol!=-1)
				{
				double y= pageHeight()-margin_top-(fHeight*(cluster.genes.size()+1));
				for(map<string,set<int32_t>,SmartComparator >::iterator r=cluster.sample2positions.begin();
						r!=cluster.sample2positions.end();
						++r)
					{
					cout << "0.2 setlinewidth\n";
					cout << "0 0 0 setrgbcolor\n";
					cout << "10 " << (y-midy+5) << " moveto (" << r->first << ") show\n";
					cout << "newpath "<< margin_left << " "<< y << " moveto\n"
							<< width()<< " " << 0 << " rlineto stroke\n";
						for(set<int32_t>::iterator r2= r->second.begin();
											r2!=r->second.end();
											++r2)
						{
						cout << "0.8 setlinewidth\n";
						cout << "newpath "<<toPixel(*r2)<<" "<< y << " circle closepath stroke\n";
						}
					y-=fHeight;
					}
				}
			cout << "showpage\n";

			cluster.clear();
			}

		void run(std::istream& in)
			    {
			    vector<string> tokens;
			    string line;
			    map<string,auto_vector<KnownGene> >::iterator rchrom;
			    while(getline(in,line,'\n'))
				    {
				    if(AbstractApplication::stopping()) break;
				    if(line.empty() || line[0]=='#') continue;
				    WHERE(line);
				    tokenizer.split(line,tokens);

				    if(chromCol>=(int)tokens.size())
				    	{
						cerr << "Column CHROM out of range in "<< line << endl;
						continue;
						}
				    if((rchrom=chrom2genes.find(tokens[chromCol]))==chrom2genes.end())
				    	{
				    	continue;
				    	}
				    if(posCol>=(int)tokens.size())
						{
						cerr << "Column POS out of range in "<< line << endl;
						continue;
						}
				    if(sampleCol!=-1 && sampleCol>=(int)tokens.size())
						{
						cerr << "Column SAMPLE out of range in "<< line << endl;
						continue;
						}
				    char* p2;
					int pos=(int)strtol(tokens[posCol].c_str(),&p2,10);
					if(pos <0 || *p2!=0)
						{
						cerr << "Bad POS "<< tokens[posCol] << " in "<<line << endl;
						continue;
						}
					 WHERE(line);
					if(cluster.chromEnd <=pos)
						{
						print();
						WHERE("");
						cluster.clear();
						auto_vector<KnownGene>&  v= rchrom->second;

						size_t i=0;
						while(i< v.size())
							{
							WHERE("");
							KnownGene* g=v.at(i);
							WHERE((g==NULL));
							if(cluster.genes.empty())
								{
								WHERE("");
								if(g->txEnd <=pos || g->txStart> pos )
									{
									i++;
									continue;
									}
								WHERE("");
								cluster.chromStart= g->txStart;
								cluster.chromEnd= g->txEnd;
								WHERE("");
								cluster.genes.push_back(v.release(i));
								WHERE("");
								}
							else
								{
								WHERE("");
								if(!(g->txStart>cluster.chromEnd || g->txEnd<= cluster.chromStart))
									{
									cluster.chromStart=min(cluster.chromStart,g->txStart);
									cluster.chromEnd=max(cluster.chromEnd,g->txEnd);
									cluster.genes.push_back(v.release(i));
									}
								else
									{
									++i;
									}
								}
							WHERE("");
							}
						}
					WHERE("");
					if(!(cluster.chromStart<=pos && pos<cluster.chromEnd))
						{
						continue;
						}
					cluster.positions.insert(pos);
					if(sampleCol!=-1)
						{
						cluster.sample2positions[tokens[sampleCol]].insert(pos);
						}
				    }
			    WHERE("");
			    print();
			    }


		void loadGenes()
			{
		    Tokenizer comma;
		    comma.delim=',';
		    vector<string> exonStarts;
		    vector<string> exonEnds;

		    string query="select "
				    " chrom,name,strand,txStart,txEnd,cdsStart,cdsEnd,exonCount,exonStarts,exonEnds "
				    " from knownGene "
		    		" order by chrom,txStart,txEnd "
		    		;
		    MYSQL_ROW row;
		    if(mysql_real_query( mysql, query.c_str(),query.size())!=0)
				 {
				 THROW("Failure for "<< query << "\n" << mysql_error(mysql));
				 }
		    MYSQL_RES* res=mysql_use_result( mysql );
		    //int ncols=mysql_field_count(mysql);
		    while(( row = mysql_fetch_row( res ))!=NULL )
				{
		    	if(AbstractApplication::stopping()) break;
				KnownGene* g=new KnownGene;

				g->chrom.assign(row[0]);

				g->name.assign(row[1]);
				g->strand=row[2][0];
				g->txStart=atoi(row[3]);
				g->txEnd=atoi(row[4]);
				g->cdsStart=atoi(row[5]);
				g->cdsEnd=atoi(row[6]);
				int exonCount=atoi(row[7]);
				comma.split(row[8],exonStarts);
				comma.split(row[9],exonEnds);
				for(int i=0;i< exonCount;++i)
					{
					Exon exon;
					exon.index=i;
					exon.gene=g;
					exon.start=atoi(exonStarts[i].c_str());
					exon.end=atoi(exonEnds[i].c_str());
					g->exons.push_back(exon);
					}
				chrom2genes[g->chrom].push_back(g);
				}
		    ::mysql_free_result( res );
			}

		void usage(ostream& out,int argc,char** argv)
			{
			out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			out << "Options:\n";
			out << " -c <int> CHROM column default:"<< chromCol << endl;
			out << " -p <int> POS column default:"<< posCol << endl;
			out << " -s <int> SAMPLE column (optional) default:"<< sampleCol << endl;
			MysqlApplication::printConnectOptions(out);
			}


#define ARGVCOL(flag,var) else if(std::strcmp(argv[optind],flag)==0 && optind+1<argc)\
	{\
	char* p2;\
	this->var=(int)strtol(argv[++optind],&p2,10);\
	if(this->var<1 || *p2!=0)\
		{cerr << "Bad column for "<< flag << ".\n";this->usage(cerr,argc,argv);return EXIT_FAILURE;}\
		this->var--;\
	}



		int main(int argc,char** argv)
		    {
			int n_optind;
		    int optind=1;
		    while(optind < argc)
				{
				if(strcmp(argv[optind],"-h")==0)
					{
					usage(cerr,argc,argv);
					return(EXIT_FAILURE);
					}
				else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
					{
					char* p=argv[++optind];
					if(strlen(p)!=1)
						{
						cerr<< "bad delimiter \"" << p << "\"\n";
						return (EXIT_FAILURE);
						}
					tokenizer.delim=p[0];
					}
				ARGVCOL("-c",chromCol)
				ARGVCOL("-p",posCol)
				ARGVCOL("-s",sampleCol)
				else if((n_optind=argument(optind,argc,argv))!=-1)
					{
					if(n_optind==-2) return EXIT_FAILURE;
					optind=n_optind;
					}
				else if(strcmp(argv[optind],"--")==0)
					{
					++optind;
					break;
					}
				else if(argv[optind][0]=='-')
					{
					cerr << "unknown option '" << argv[optind]<< "'" << endl;
					usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				else
					{
					break;
					}
				++optind;
				}
		    if(optind!=argc && optind+1!=argc)
		    	{
		    	cerr << "Illegal number of arguments.\n";
				usage(cerr,argc,argv);
				return EXIT_FAILURE;
		    	}

		    connect();
		    loadGenes();

		    double ticksH=(fHeight/2.0f)*0.6f;
			double ticksx=20;
		    time_t rawtime;
		   	 time ( &rawtime );
		    cout << "%!PS\n"
				"%%Creator: Pierre Lindenbaum PhD plindenbaum@yahoo.fr http://plindenbaum.blogspot.com\n"
				"%%Title: " << __FILE__ << "\n"
				"%%CreationDate: "<< ::ctime (&rawtime)<<
				"%%BoundingBox: 0 0 "
				<<(int)pageWidth()<< " "
				<<(int)pageHeight()<<"\n"
				""
				"/Courier findfont 9 scalefont setfont\n"
				"/circle { 10 0 360 arc} bind def\n"
				"/ticksF {\n"
				<< (-ticksH)<< " "<< (ticksH) << " rmoveto\n"
				<< ticksH << " "<< (-ticksH) << " rlineto\n"
				<< (-ticksH) << " "<< (-ticksH) << " rlineto\n"
				<< ticksH << " "<< ticksH << " rmoveto\n"
				"} bind def\n"

				"/ticksR {\n"
				<< (ticksH)<< " "<< (ticksH) << " rmoveto\n"
				<< (-ticksH) << " "<< (-ticksH) << " rlineto\n"
				<< (ticksH) << " "<< (-ticksH) << " rlineto\n"
				<< (-ticksH) << " "<< (ticksH) << " rmoveto\n"
				"} bind def\n"


				"/forticksF {2 dict begin\n"
				"/x2 exch def\n"
				"/x1 exch def\n"

				"0 1 x2 x1 sub "<< ticksx <<" div {\n"
				"ticksF "<< ticksx << " 0 rmoveto\n"
				"}for\n"
				"} bind def\n"

				"/forticksR {2 dict begin\n"
				"/x2 exch def\n"
				"/x1 exch def\n"
				"0 1 x2 x1 sub "<< ticksx <<" div {\n"
				" ticksR  "<< ticksx << " 0 rmoveto\n"
				"}for\n"
				"} bind def\n"


				"/box\n"
				"{\n"
				"4 dict begin\n"
				"/height exch def\n"
				"/width exch def\n"
				"/y exch def\n"
				"/x exch def\n"
				"x y moveto\n"
				"width 0 rlineto\n"
				"0 height rlineto\n"
				"width -1 mul 0 rlineto\n"
				"0 height -1 mul rlineto\n"
				"end\n"
				"} bind def\n"

				"/gradient\n"
				"{\n"
				"4 dict begin\n"
				"/height exch def\n"
				"/width exch def\n"
				"/y exch def\n"
				"/x exch def\n"
				"/i 0 def\n"
				"height 2 div /i exch def\n"
				"\n"
				"0 1 height 2 div {\n"
				"	1 i height 2.0 div div sub setgray\n"
				"	newpath\n"
				"	x  \n"
				"	y height 2 div i sub  add\n"
				"	width\n"
				"	i 2 mul\n"
				"	box\n"
				"	closepath\n"
				"	fill\n"
				"	i 1 sub /i exch def\n"
				"	}for\n"
				"newpath\n"
				"0 setgray\n"
				"0.4 setlinewidth\n"
				"x y width height box\n"
				"closepath\n"
				"stroke\n"
				"end\n"
				"} bind def\n"



				;


		    if(optind==argc)
				{
				igzstreambuf buf;
				istream in(&buf);
				run(in);
				}
		    else if(optind+1==argc)
				{
				igzstreambuf buf(argv[optind++]);
				istream in(&buf);
				run(in);
				buf.close();
				++optind;
				}
		    cout << "\n%%EOF\n";
		    return EXIT_SUCCESS;
		    }
	};


int main(int argc,char** argv)
    {
	GenePostcript app;
	return app.main(argc,argv);
    }
