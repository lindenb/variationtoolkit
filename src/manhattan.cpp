/*
 * manhattan.cpp
 *
 *  Created on: Sep 28, 2011
 *      Author: lindenb
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <cerrno>
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
#include <stdint.h>
using namespace std;

#define THROW(a) do{ostringstream _os;\
	_os << __FILE__ << ":"<< __LINE__ << ":"<< a << endl;\
	throw runtime_error(_os.str());\
	}while(0);

typedef int32_t pos_t;
typedef float pixel_t;
typedef double value_t;
struct Data
    {
	pos_t pos;
	value_t value;
	bool operator < (const Data& cp) const
	    {
	    return pos < cp.pos;
	    }
    };

struct smart_cmp
    {
    bool operator() (const string& a, const string& b) const
	  {
	  char* p1=(char*)a.c_str();
	  char* p2=(char*)b.c_str();
	  for(;;)
	      {
	      if(*p1==0 && *p2==0) return false;
	      if(*p1==0 && *p2!=0) return true;
	      if(*p1!=0 && *p2==0) return false;
	      if(isdigit(*p1) && isdigit(*p2))
		  {
		  char* pp1;
		  char* pp2;
		  long n1=strtol(p1,&pp1,10);
		  long n2=strtol(p2,&pp2,10);
		  if(n1!=n2) return (n1<n2?true:false);
		  p1=pp1;
		  p2=pp2;
		  continue;
		  }
	      int i=toupper(*p1)-toupper(*p2);
	      if(i!=0) return (i<0?true:false);
	      ++p1;
	      ++p2;
	      }

	  }
    };

class Manhattan
    {
    private:
	bool readline(gzFile in,std::string& line)
	    {
	    line.clear();
	    int c;
	    if(gzeof(in)) return false;
	    while((c=gzgetc(in))!=EOF && c!='\n')
		{
		line+=(char)c;
		}
	    return true;
	    }
    public:

	class ChromInfo
	    {
	    public:
		string chrom;
		pos_t chromStart;
		pos_t chromEnd;
		pixel_t x;
		pixel_t width;
		size_t index_data;
		pos_t length()
		    {
		    return chromEnd-chromStart;
		    }
		vector<Data> data;
	    };
	typedef std::map<string,ChromInfo*,smart_cmp> chrom2info_t;
	char delim;
	chrom2info_t chrom2info;
	value_t minValue;
	value_t maxValue;
	std::ostream* output;
	pixel_t x_axis_width;
	pixel_t y_axis_height;
	pixel_t margin_left;
	pixel_t margin_right;
	pixel_t margin_top;
	pixel_t margin_bottom;
	Manhattan():delim('\t'),
		chrom2info(),
		output(&cout),
		x_axis_width(1000),
		y_axis_height(1000)
	    {
	    minValue=DBL_MAX;
	    maxValue=-DBL_MAX;
	    margin_left=100;
	    margin_bottom=100;
	    margin_top=100;
	    margin_right=100;
	    }

	~Manhattan()
	    {
	    for(map<string,ChromInfo*>::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
		{
		delete (*r).second;
		}
	    }


	void readData(gzFile in)
	    {
	    string line;
	    while(readline(in,line))
		{

		if(line.empty() || line[0]=='#') continue;
		string::size_type n1=line.find(delim);
		if(n1==string::npos) THROW("delimiter 'chrom' missing in "<< line);
		string str=line.substr(0,n1);

		ChromInfo* chromInfo;
		chrom2info_t::iterator r= chrom2info.find(str);
		if(r==chrom2info.end())
		    {
		    chromInfo=new ChromInfo;
		    chromInfo->chrom=str;
		    chrom2info.insert(make_pair<string,ChromInfo*>(str,chromInfo));
		    }
		else
		    {
		    chromInfo=r->second;
		    }
		string::size_type n2=line.find(delim,++n1);
		if(n2==string::npos) THROW("delimiter 'pos' missing in "<< line);
		str=line.substr(n1,n2-n1);

		char* p2;
		Data newdata;
		newdata.pos= strtol(str.c_str(),&p2,10);
		if(!(*p2==0) || newdata.pos<0)
		    {
		    THROW("Bad position in "<< line);
		    }
		chromInfo->chromStart=min(chromInfo->chromStart,newdata.pos);
		chromInfo->chromEnd=max(chromInfo->chromEnd,newdata.pos);
		string::size_type n3=line.find(delim,++n2);
		str=line.substr(n2,n3==string::npos?line.size()-n2:n3-n2);
		newdata.value= strtod(str.c_str(),&p2);
		if(!(*p2==0) || isnan(newdata.value))
		    {
		    THROW("Bad value in "<< line);
		    }
		chromInfo->data.push_back(newdata);
		minValue=min(minValue,newdata.value);
		maxValue=max(maxValue,newdata.value);
		}
	    }
	pixel_t pageWidth()
	    {
	    return (margin_left+margin_right+x_axis_width);
	    }
	pixel_t pageHeight()
	    {
	    return (margin_top+margin_bottom+y_axis_height);
	    }
	void print()
	    {
	    int64_t size_of_genome=0L;
	    if(fabs(minValue-maxValue) < 10* DBL_EPSILON)
		{
		minValue-=1;
		maxValue+=1;
		}
	    double value5=((maxValue-minValue)/100.0)*5.0;
	    minValue-=value5;
	    maxValue+=value5;

	    for(chrom2info_t::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
		{
		ChromInfo* c=r->second;
		if(c->chromStart==c->chromEnd) c->chromEnd++;
		double length5=1+(c->length()/100.0)*5.0;
		c->chromStart=max(0,(pos_t)(c->chromStart-length5));
		c->chromEnd=c->chromEnd+length5;

		std::sort(c->data.begin(),c->data.end());

		size_of_genome+=c->length();
		assert(size_of_genome>0);
		}
	    pixel_t x=margin_left;
	    for(chrom2info_t::iterator r=chrom2info.begin();
	   		    r!=chrom2info.end();
	   		    ++r)
		{
		ChromInfo* c=r->second;
		c->x=x;
		c->width=(c->length()/(double)size_of_genome)*x_axis_width;
		assert(c->length()>0);
		assert(size_of_genome>0);
		assert(x_axis_width>0);
		assert(c->width>0);
		x+=c->width;
		}

	    ostream& out=(*output);
	    time_t rawtime;
	    time ( &rawtime );
	    out <<
		    "%!PS\n"
		    "%%Creator: Pierre Lindenbaum PhD plindenbaum@yahoo.fr http://plindenbaum.blogspot.com\n"
		    "%%Title: " << __FILE__ << "\n"
		    "%%CreationDate: "<< ::ctime (&rawtime)<<
		    "%%BoundingBox: 0 0 "
			<<(int)pageWidth()<< " "
			<<(int)pageHeight()<<"\n"
		    "%%Pages! 1\n"
		    "/marginBottom "<< margin_bottom << " def\n"
		    "/marginLeft "<< margin_left << " def\n"
		    "/yAxis "<< y_axis_height << " def\n"
		    "/getChromName { 0 get } bind def\n"
		    "/getChromStart { 1 get } bind def\n"
		    "/getChromEnd { 2 get } bind def\n"
		    "/getChromX { 3 get } bind def\n"
		    "/getChromWidth { 4 get } bind def\n"
		     "/getChromLength {1 dict begin /chrom exch def chrom getChromEnd  chrom getChromStart sub end} bind def\n"
		    "/getChromDataIndex { 5 get } bind def\n"
		    "/getChromDataCount { 6 get } bind def\n"
		    "/convertPos2X {2 dict begin /pos exch def /chrom exch def  "
		    "pos chrom getChromStart sub chrom getChromLength div chrom getChromWidth mul chrom getChromX add\n"
		    "end } bind def\n"
		    "/minValue "<< minValue<<" def\n"
		    "/maxValue "<< maxValue<<" def\n"
		    "/toString { 20 string cvs} bind def\n"
		    "/Courier findfont 14 scalefont setfont\n"
		    "/convertValue2Y {\n"
		    " minValue sub maxValue minValue sub div yAxis mul marginBottom add"
		    "} bind def\n"
		    "/diams { 2 dict begin\n"
			"  /y exch def\n"
			"  /x exch def\n"
			" newpath "
			"   x y 5 add moveto\n"
			"   x 5 add y lineto\n"
			"   x y 5 sub lineto\n"
			"   x 5 sub y lineto\n"
			" closepath 0 0  1 setrgbcolor fill "
			"   end } bind def\n"
		    "/box { 4 dict begin\n"
		    "  /height exch def\n"
		    "  /width exch def\n"
		    "  /y exch def\n"
		    "  /x exch def\n"
		    "   x y moveto\n"
		    "   width 0 rlineto\n"
		    "   0 height rlineto\n"
		    "   width -1 mul 0 rlineto\n"
		    "   0 height -1 mul rlineto\n"
		    "   end } bind def\n"
		    "/paintChrom { 2 dict begin\n"
		    " /chrom exch def\n"
		    " /i 0 def\n"
		    " 0.8 setgray newpath chrom getChromX marginBottom chrom getChromWidth yAxis box closepath stroke "
		    " chrom getChromX chrom getChromWidth 0.5 mul add marginBottom yAxis add moveto "
		    " 0 setgray 90 rotate chrom getChromName show -90 rotate "
		    " i chrom getChromDataIndex 2 mul /i exch def\n"
		    " chrom getChromDataCount {"
		    " chrom expData i get convertPos2X "
		    " expData i 1 add get convertValue2Y "
		    " diams"
		    " i 2 add /i exch def\n"
		    " } repeat "
		    "i 0 /i exch def\n"
		    " chrom getChromWidth 200 gt { 10 {"
		    "  newpath "
		    "  i 10.0 div chrom getChromWidth mul chrom getChromX add marginBottom moveto\n"
		    "  0 -10 rlineto\n"
		    " closepath stroke "
		    "  i 10.0 div chrom getChromWidth mul chrom getChromX add marginBottom 10 sub moveto\n"
		    " -90 rotate "
		    "  i 10.0 div chrom getChromLength mul chrom getChromStart add toString show "
		    "  90 rotate "
		    "  i 1 add /i exch def\n"
		    "} repeat } if \n"
		    "end} bind def\n"

		    "/paintGenome { 2 dict begin "
		    " /chroms exch def\n"
		    " /i 0 def\n"
		    " chroms length {"
		    " chroms i get paintChrom "
		    " i 1 add /i exch def\n"
		    " } repeat"
		    " i 0 /i exch def\n"
		    " 10 {\n"
		    " 0.1 setgray\n"
		    " newpath\n"
		    " marginLeft i 10.0 div yAxis mul marginBottom add moveto\n"
		    " -10 0 rlineto\n"
		    " stroke\n"
		    " i 1 add /i exch def\n"
		    " 1 i 10.0 div yAxis mul marginBottom add moveto\n"
		    " i 10.0 div maxValue minValue sub mul minValue add toString show\n"
		    " } repeat\n"
		    " end} bind def "
		    ;
	    size_t index_data=0UL;
	    out << "/expData [\n";
	    for(chrom2info_t::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
		    {
		    r->second->index_data=index_data;
		    out << "% "<< r->second->chrom<< " 2x(" << r->second->index_data << ")\n";
		    for(vector<Data>::iterator r2=r->second->data.begin();
			    r2!=r->second->data.end();
			    ++r2)
			    {
			    out << r2->pos  << " " << r2->value << "\n";
			    index_data++;
			    }
		    }
	    out << "] def\n";

	    out << "/genome [";
	    for(chrom2info_t::iterator r=chrom2info.begin();
	   	   		    r!=chrom2info.end();
	   	   		    ++r)
		{
		ChromInfo* c=r->second;
		out << "[";
		out	<< "(" << c->chrom << ") "
			<< c->chromStart << " "
			<< c->chromEnd <<" "
			<< c->x<< " "
			<< c->width << " "
			<< c->index_data << " "
			<< c->data.size()
			;
		out << "]\n";
		}
	    out << "] def\n";
	    out << "genome paintGenome\n"
		"0 setgray\n0 0 "<< pageWidth()<<" " << pageHeight() << " box\n"
		"0 setgray\n"
		"stroke\n"
		;
	    out << "showpage\n";
	    out.flush();
	    }

    };

int main(int argc,char** argv)
    {
    Manhattan app;
    int optind=1;
    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			cerr << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
   			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
   			exit(EXIT_FAILURE);
   			}
   		else if(argv[optind][0]=='-')
   			{
   			fprintf(stderr,"unknown option '%s'\n",argv[optind]);
   			exit(EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
                }
    if(optind==argc)
	    {
	    gzFile in=gzdopen(fileno(stdin),"r");
	    if(in==NULL)
		{
		cerr << "Cannot open stdin" << endl;
		return EXIT_FAILURE;
		}
	    app.readData(in);
	    }
    else
	    {
	    while(optind< argc)
		{
		char* filename=argv[optind++];
		gzFile in=gzopen(filename,"r");
		if(in==NULL)
		    {
		    cerr << "Cannot open "<< filename << " " << strerror(errno) << endl;
		    return EXIT_FAILURE;
		    }
		app.readData(in);
		gzclose(in);
		}
	    }
    app.print();
    return EXIT_SUCCESS;
    }
