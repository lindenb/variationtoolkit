/*
 * Author:
 *	Pierre Lindenbaum PhD.
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	Generates a Postscript Manhattan Plot 
 * Date:
 *	2011
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
#include "tokenizer.h"
#include "smartcmp.h"
#include "zstreambuf.h"
using namespace std;


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


class Manhattan
    {
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
		ChromInfo():chromStart(INT_MAX),chromEnd(INT_MIN)
			{
			
			}
		pos_t length()
		    {
		    return chromEnd-chromStart;
		    }
		vector<Data> data;
		
		pixel_t convertToX(const Data* d)
			{
			return x+ ((d->pos-chromStart)/(double)length())*width;
			}
		
	    };
	typedef std::map<string,ChromInfo*,SmartComparator> chrom2info_t;
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
		output(&cout)
	    {
	    minValue=DBL_MAX;
	    maxValue=-DBL_MAX;
	    margin_left=100;
	    margin_bottom=100;
	    margin_top=100;
	    margin_right=100;
	    x_axis_width=841.89 -(margin_left+margin_right);
	    y_axis_height=595.28-(margin_top+margin_bottom);
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

	pixel_t convertToY(const Data* d)
		{
		return margin_bottom+ ((d->value-minValue)/(double)(maxValue-minValue))*y_axis_height;
		}
	

	void readData(std::istream& in)
	    {
	    string line;
	    while(getline(in,line,'\n'))
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
		c->chromEnd=(pos_t)(c->chromEnd+length5);

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
		    "/dd { 2 dict begin\n"
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
		    ;
	    int index_data=-1;
	    for(chrom2info_t::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
		    {
		    index_data++;
		    ChromInfo* chromInfo=r->second;
		    
		    out << " "<< (index_data%2==0?1:0.7)<< " setgray newpath "<< chromInfo->x << " "<< margin_bottom << " "<< chromInfo->width << " " << y_axis_height <<  " closepath box fill ";
		    
		    out << " 0.4 setgray newpath "<< chromInfo->x << " "<< margin_bottom << " "<< chromInfo->width << " " << y_axis_height <<  " closepath box stroke ";
		    }
		    
		    
		for(chrom2info_t::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
		    {
		    ChromInfo* chromInfo=r->second;
		    for(vector<Data>::iterator r2=r->second->data.begin();
			    r2!=r->second->data.end();
			    ++r2)
			    {
			    out << chromInfo->convertToX(&*(r2))  << " " << convertToY(&(*r2)) << " dd\n";
			    index_data++;
			    }
		    if(chromInfo->width>200)
		    	{
		    for(int i=0;i<=10.0;++i)
	   		{
	   		double x=chromInfo->x + ((chromInfo->width)/10.0)*i;
	   		out << " 0 setgray newpath "
	   		    << " " << x << " "  << (margin_bottom) << " moveto "
	   		    << " 0 -10 rlineto closepath stroke " 
	   		    << " " << x << " "  << (margin_bottom-20) << " moveto "
	   		    << " -90 rotate (" << (int)(chromInfo->chromStart+((chromInfo->length()/10.0)*i)) << ") show 90 rotate"
	   		    << endl
	   		    ;
	   		}
	   		}
	   	    
	   	    out 	<< " 0 setgray "
	   	    		<< " " << (chromInfo->x + (chromInfo->width/2.0))
	   	    		<< " "  << (margin_bottom+10+y_axis_height) << " moveto "
	   		    	<< "  90 rotate (" << chromInfo->chrom << ") show -90 rotate "
	   		    	<< endl;
		    }

	
	   for(int i=0;i<=10;++i)
	   	{
	   	out << " 0 setgray newpath ";
	   	out << " " << margin_left << " " << (margin_bottom+(i/10.0)*y_axis_height) << " moveto -15 0 rlineto ";
	   	out << " closepath stroke ";
	   	out << " 1 " << (margin_bottom+(i/10.0)*y_axis_height) << " moveto";
	   	out << " (" << (minValue+ (i/10.0)*(maxValue-minValue)) << ") show "; 
	   	}
	  
	   
	    out << "0 setgray\n0 0 "<< pageWidth()<<" " << pageHeight() << " box\n"
		"0 setgray\n"
		"stroke\n"
		;
	    out << "showpage\n";
	    out.flush();
	    }
    void usage(int argc,char** argv)
	{
	cerr << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
   	cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
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
			app.usage(argc,argv);
   			return (EXIT_FAILURE);
   			}
   		else if(argv[optind][0]=='-')
   			{
   			cerr << "unknown option '"<< argv[optind] << "'\n";
			app.usage(argc,argv);
   			return (EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
                }
    if(optind==argc)
	    {
	    igzstreambuf buf;
	    istream in(&buf);
	    app.readData(in);
	    }
    else
	    {
	    while(optind< argc)
		{
		char* filename=argv[optind++];
		igzstreambuf buf(filename);
		istream in(&buf);
		app.readData(in);
		buf.close();
		}
	    }
    app.print();
    return EXIT_SUCCESS;
    }
