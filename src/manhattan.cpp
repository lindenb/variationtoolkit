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
#include "numeric_cast.h"
#include "application.h"
#include "tokenizer.h"
#include "smartcmp.h"
#include "zstreambuf.h"
#include "color.h"

#include "where.h"
using namespace std;


typedef int32_t pos_t;
typedef float pixel_t;
typedef double value_t;
struct Data
    {
    /** genomic position */
    pos_t chromStart;
    pos_t chromEnd;
    
    /** y value */
    value_t value;
    /** rgb color */
    uint32_t rgb;


    bool operator < (const Data& cp) const
	{
	if(chromStart!=cp.chromStart)
		{
		return chromStart < cp.chromStart;
		}
	return chromEnd < cp.chromEnd;
	}
    };


class Manhattan:public AbstractApplication
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
	    private:
		 pixel_t convertPOSToX(pos_t p)
			{
			return x+ (( p -chromStart)/(double)length())*width;
			}   
	     public:
		/* vector<Data> data;*/
		pixel_t convertChromStartToX(const Data* d)
			{
			return convertPOSToX(d->chromStart);
			}
		pixel_t convertChromEndToX(const Data* d)
			{
			return convertPOSToX(d->chromEnd);
			}
	    };

	class Frame
	     {
	     public:
		 pixel_t y;
		 pixel_t height;
		 std::string sample;
		 Manhattan* owner;
		 map<ChromInfo*,vector<Data> > chrom2data;

		 pixel_t convertToY(const Data* d)
			{
			return y+ ((d->value-owner->minValue)/(double)(owner->maxValue-owner->minValue))*height;
			}
	     };

	typedef std::map<string,ChromInfo*,SmartComparator> chrom2info_t;
	chrom2info_t chrom2info;
	typedef std::map<string,Frame*,SmartComparator> sample2frame_t;
	sample2frame_t sample2frame;
	value_t minValue;
	value_t maxValue;
	std::ostream* output;
	pixel_t x_axis_width;
	pixel_t y_axis_height;
	pixel_t margin_left;
	pixel_t margin_right;
	pixel_t margin_top;
	pixel_t margin_bottom;
	pixel_t point_size;
	int chromCol;
	int posStartCol;
	int posEndCol;
	int valueCol;
	int colorCol;
	int sampleCol;
	value_t* user_min_value;
	value_t* user_max_value;


	Manhattan():
		chrom2info(),
		output(&cout),
		chromCol(0),
		posStartCol(1),
		posEndCol(1),
		valueCol(-1),
		colorCol(-1),
		sampleCol(-1),
		user_min_value(NULL),
		user_max_value(NULL)
	    {
	    tokenizer.delim='\t';
	    minValue=DBL_MAX;
	    maxValue=-DBL_MAX;
	    margin_left=100;
	    margin_bottom=100;
	    margin_top=100;
	    margin_right=100;
	    x_axis_width=841.89 -(margin_left+margin_right);
	    y_axis_height=595.28-(margin_top+margin_bottom);
	    point_size=5;
	    }

	~Manhattan()
	    {
	    for(map<string,ChromInfo*>::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
			{
			delete (*r).second;
			}
			for(map<string,Frame*>::iterator r=sample2frame.begin();
				r!=sample2frame.end();
				++r)
			{
			delete (*r).second;
			}
	    }

	


	void readData(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(AbstractApplication::stopping()) break;
		if(line.empty() || line[0]=='#') continue;
		tokenizer.split(line,tokens);
		if(chromCol>=(int)tokens.size())
		    {
		    cerr << "CHROM column out of range in "<< line << endl;
		    continue;
		    }
		if(posStartCol>=(int)tokens.size())
		    {
		    cerr << "POS column out of range in "<< line << endl;
		    continue;
		    }

		if(posEndCol>=(int)tokens.size())
		    {
		    cerr << "POS column out of range in "<< line << endl;
		    continue;
		    }



		if(valueCol>=(int)tokens.size())
		    {
		    cerr << "VALUE column out of range in "<< line << endl;
		    continue;
		    }

		if(sampleCol>=(int)tokens.size())
		    {
		    cerr << "SAMPLE column out of range in "<< line << endl;
		    continue;
		    }

		Data newdata;
		char* p2;
		/* value */
		newdata.value= strtod(tokens[valueCol].c_str(),&p2);
		if(!(*p2==0) || isnan(newdata.value))
		    {
		    THROW("Bad value in "<< line);
		    }


		if(user_min_value!=NULL && *user_min_value>newdata.value)
		    {
		    continue;
		    }
		if(user_max_value!=NULL && *user_max_value<newdata.value)
		    {
		    continue;
		    }

		/** searching chromosome */
		ChromInfo* chromInfo;
		chrom2info_t::iterator r= chrom2info.find(tokens[chromCol]);
		if(r==chrom2info.end())
		    {
		    chromInfo=new ChromInfo;
		    chromInfo->chrom=tokens[chromCol];
		    chrom2info.insert(make_pair<string,ChromInfo*>(tokens[chromCol],chromInfo));
		    }
		else
		    {
		    chromInfo=r->second;
		    }
		/* search frame */
		Frame* frame=NULL;
		if(sampleCol==-1)//no sample used
		    {
		    assert(!sample2frame.empty());
		    frame=sample2frame.begin()->second;
		    }
		else
		    {
		    sample2frame_t::iterator r2=sample2frame.find(tokens[sampleCol]);
		    if(r2==sample2frame.end()) /* new sample */
			{
			frame=new Frame;
			frame->owner=this;
			frame->sample.assign(tokens[sampleCol]);
			sample2frame.insert(make_pair<string,Frame*>(frame->sample,frame));
			}
		    else
			{
			frame=r2->second;
			}
		    }


		if(colorCol==-1 || tokens[colorCol].empty())
		    {
		    newdata.rgb=(int32_t)0L;
		    }
		else
		    {
		    Color color(tokens[colorCol].c_str());
		    newdata.rgb=color.asInt();
		    }
		   
		if(!numeric_cast<pos_t>(tokens[posStartCol].c_str(),&(newdata.chromStart)) || newdata.chromStart<0)
			{
			 THROW("Bad chromStart in "<< line);
			}   
		  if(posStartCol==posEndCol)
		  	{
		  	newdata.chromEnd=newdata.chromStart+1;
		  	}
		  else
		  	{
			if(!numeric_cast<pos_t>(tokens[posEndCol].c_str(),&(newdata.chromEnd))|| newdata.chromEnd<0)
				{
				THROW("Bad chromEnd in "<< line);
				}
			}

		chromInfo->chromStart=min(chromInfo->chromStart,newdata.chromStart);
		chromInfo->chromEnd=max(chromInfo->chromEnd,newdata.chromEnd);



		frame->chrom2data[chromInfo].push_back(newdata);
		//chromInfo->data.push_back(newdata);
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
	    return (margin_top+margin_bottom+y_axis_height*sample2frame.size());
	    }
	void print()
	    {
	    int64_t size_of_genome=0L;



	    if(user_min_value!=NULL)
		{
		minValue=*user_min_value;
		}
	    if(user_max_value!=NULL)
		{
		maxValue=*user_max_value;
		}

	    if(fabs(minValue-maxValue) < 10* DBL_EPSILON)
		{
		minValue-=1;
		maxValue+=1;
		}

	    if(minValue==DBL_MAX || maxValue==-DBL_MAX)
	    	{
	    	cerr << "manhattan: no data ?\n";
	    	return;
	    	}


	    for(chrom2info_t::iterator r=chrom2info.begin();
		    r!=chrom2info.end();
		    ++r)
		{
		ChromInfo* c=r->second;
		if(c->chromStart==c->chromEnd) c->chromEnd++;
		double length5=1+(c->length()/100.0)*5.0;
		c->chromStart=max(0,(pos_t)(c->chromStart-length5));
		c->chromEnd=(pos_t)(c->chromEnd+length5);

		//std::sort(c->data.begin(),c->data.end());

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

	    pixel_t y=pageHeight()-margin_top;
	    for(sample2frame_t::iterator r=sample2frame.begin();
		r!=sample2frame.end();
		++r)
		{
		Frame* f=r->second;
		f->height = y_axis_height;
		f->y = y-f->height;

		for(map<ChromInfo*,vector<Data> >::iterator r2=f->chrom2data.begin();
		    r2!=f->chrom2data.end();
		    ++r2)
		    {
		    std::sort(r2->second.begin(),r2->second.end());
		    }
		y-=f->height;
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
		    //"/marginBottom "<< margin_bottom << " def\n"
		    //"/marginLeft "<< margin_left << " def\n"
		    //"/yAxis "<< y_axis_height << " def\n"
		    //"/getChromName { 0 get } bind def\n"
		    //"/getChromStart { 1 get } bind def\n"
		    //"/getChromEnd { 2 get } bind def\n"
		    //"/getChromX { 3 get } bind def\n"
		    //"/getChromWidth { 4 get } bind def\n"
		    //"/getChromLength {1 dict begin /chrom exch def chrom getChromEnd  chrom getChromStart sub end} bind def\n"
		    //"/getChromDataIndex { 5 get } bind def\n"
		    //"/getChromDataCount { 6 get } bind def\n"
		    //"/toString { 20 string cvs} bind def\n"
		    "/Courier findfont 14 scalefont setfont\n"
		    ;
	if(posStartCol==posEndCol)
		   {
		    out << "/dd { 2 dict begin\n"
			"  /y exch def\n"
			"  /x exch def\n"
			" newpath "
			"   x y "<< point_size <<" add moveto\n"
			"   x "<< point_size <<" add y lineto\n"
			"   x y "<< point_size <<" sub lineto\n"
			"   x "<< point_size <<" sub y lineto\n"
			" closepath" << (colorCol==-1?" 0.1 0.1 0.5 setrgbcolor ":"") << " fill\n"
			"   end } bind def\n";
			}
	else
		{
		out << "/dd { 3 dict begin\n"
			"  /y exch def\n"
			"  /x2 exch def\n"
			"  /x1 exch def\n"
			" newpath "
			"   x1 y "<< point_size <<" add moveto\n"
			"   x2 y "<< point_size <<" add lineto\n"
			"   x2 y "<< point_size <<" sub lineto\n"
			"   x1 y "<< point_size <<" sub lineto\n"
			" closepath" << (colorCol==-1?" 0.1 0.1 0.5 setrgbcolor ":"") << " fill\n"
			"   end } bind def\n";
			
		}	
	out <<   "/box { 4 dict begin\n"
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




	int index_sample=-1;
	/* loop over samples */
	for(sample2frame_t::iterator r3=sample2frame.begin();
		r3!=sample2frame.end();
		++r3)
		{
		index_sample++;
		Frame* frame=r3->second;

		int index_chrom=-1;
		for(chrom2info_t::iterator r=chrom2info.begin();
			r!=chrom2info.end();
			++r)
			{
			index_chrom++;
			ChromInfo* chromInfo=r->second;

			out << " "<< (index_chrom%2==index_sample%2?1:0.7)<< " setgray newpath "
					<< chromInfo->x << " "<< frame->y << " "<< chromInfo->width << " " << frame->height
					<<  " closepath box fill ";

			out << " 0.4 setgray newpath "<< chromInfo->x << " "<< margin_bottom << " "<< chromInfo->width << " " << (pageHeight()-(margin_bottom+margin_top)) <<  " closepath box stroke ";
			}


		/* loop over chromosomes */
		for(map<ChromInfo*,vector<Data> >::iterator r=frame->chrom2data.begin();
		    r!=frame->chrom2data.end();
		    ++r)
		    {


		    ChromInfo* chromInfo=r->first;
		    /** loop over data */
		    for(vector<Data>::iterator r2=r->second.begin();
			    r2!=r->second.end();
			    ++r2)
			    {
			    if(colorCol!=-1)
				{

				Color color((uint32_t)r2->rgb);

				if(color.r==color.g && color.r==color.b)
				    {
				    out << color.r/255.0 << " setgray\n";
				    }
				else
				    {
				    out << color.r/255.0 << " "<< color.g/255.0 << " " << color.b/255.0 << " setrgbcolor\n";
				    }
				}
			    if(posStartCol==posEndCol)
			    	{
			    	out << chromInfo->convertChromStartToX(&*(r2)); 
				}
			   else
				{
				out	<< chromInfo->convertChromStartToX(&*(r2)) 
					<< " " << chromInfo->convertChromEndToX(&*(r2)) 
					;
				}
			    out << " " << frame->convertToY(&(*r2)) << " dd\n";
			    }
		    /* draw box for this chrom/frame */
		    out << "0 setgray\n"<< chromInfo->x<<" "<< frame->y<< " "
		    		<< chromInfo->width <<" " << frame->height << " box\n";

		    /** draw y ticks */
		    for(int i=0;i<=10;++i)
				{
		    	if(i==10 && index_sample!=0 && sample2frame.size()>1) continue;
				out << " 0 setgray newpath ";
				out << " " << margin_left << " " << (frame->y+(i/10.0)*frame->height) << " moveto -15 0 rlineto ";
				out << " closepath stroke ";
				out << " 1 " << (frame->y+(i/10.0)*frame->height) << " moveto";
				out << " (" << (minValue+ (i/10.0)*(maxValue-minValue)) << ") show ";
				}
		    /* draw label */
		    out      << " 0 setgray "
			    << " " << (pageWidth()-margin_right+10)
			    << " "  << (frame->y+frame->height) << " moveto "
			    << "  -90 rotate (" << frame->sample << ") show 90 rotate "
			    << endl;
		    }
		}/* end loop samples */

	    /** draw chromosomes */
	    for(chrom2info_t::iterator r=chrom2info.begin();
				r!=chrom2info.end();
				++r)
		{
		ChromInfo* chromInfo=r->second;
		/* draw ticks */
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
		/* draw label */
		out 	<< " 0 setgray "
			<< " " << (chromInfo->x + (chromInfo->width/2.0))
			<< " "  << (pageHeight()-margin_top+10) << " moveto "
			<< "  90 rotate (" << chromInfo->chrom << ") show -90 rotate "
			<< endl;
		}

	
	  

	   
	    out << "0 setgray\n0 0 "<< pageWidth()<<" " << pageHeight() << " box\n"
		"0 setgray\n"
		"stroke\n"
		;
	    out << "showpage\n";
	    cout << "\n%%EOF\n";
	    out.flush();
	    }
    void usage(ostream& out,int argc,char** argv)
		{
		out << endl;
		out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << VARKIT_REVISION << endl;
		out << " -c <int> CHROM column default:"<< (chromCol+1) << endl;
		out << " -p <int> POS column (or use option -b) default:"<< (posStartCol+1) << endl;
		out << " -v <int> value column default:"<< (valueCol+1) << endl;
		out << " -r <int> COLOR column (optional)" << endl;
		out << " -s <int> SAMPLE column (optional)" << endl;
		out << " -m <double> optional user's min value" << endl;
		out << " -M <double> optional user's max value" << endl;
		out << " -b <int> <int> chromStart and chromEnd columns (or use option -p). default:"<< (posStartCol+1) << " " << (posEndCol+1) << endl;
		out<< "Page options:\n";
		out << " --x-width (int) x-axis size: "<< this->x_axis_width << endl;
		out << " --y-height (int) y-axis size: "<< this->y_axis_height << endl;
		out << " --point (int) point size: "<< this->point_size << endl;
		out << endl;
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
	int optind=1;
	value_t my_min_value;
	value_t my_max_value;
	while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			    {
			    usage(cerr,argc,argv);
			    return (EXIT_FAILURE);
			    }
		    else if(strcmp(argv[optind],"--point")==0 && optind+1<argc)
				{
				char* p2;
				this->point_size=strtod(argv[++optind],&p2);
				if(this->point_size<1 || errno!=0 || *p2!=0)
					{
					cerr << "Bad value for point_size.\n";
					 usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				}
		    else if(strcmp(argv[optind],"--x-width")==0 && optind+1<argc)
		    	{
		    	char* p2;
		    	this->x_axis_width=strtod(argv[++optind],&p2);
		    	if(this->x_axis_width==0 || errno!=0 || *p2!=0)
		    		{
		    		cerr << "Bad value for x axis.\n";
		    		 usage(cerr,argc,argv);
		    		return EXIT_FAILURE;
		    		}
		    	}
		    else if(strcmp(argv[optind],"--y-height")==0 && optind+1<argc)
				{
				char* p2;
				this->y_axis_height=strtod(argv[++optind],&p2);
				if(this->y_axis_height==0 || errno!=0 || *p2!=0)
					{
					cerr << "Bad value for y axis.\n";
					 usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				}
		    else if(std::strcmp(argv[optind],"-m")==0)
				{
				char* p2;
				my_min_value=strtod(argv[++optind],&p2);
				if(*p2!=0 || errno!=0 || isnan(my_min_value))
					{
					cerr << "Bad min value "<< argv[optind] << endl;
					return (EXIT_FAILURE);
					}
				this->user_min_value=&my_min_value;
				}
		    else if(std::strcmp(argv[optind],"-M")==0)
			{
			char* p2;
			my_max_value=strtod(argv[++optind],&p2);
			if(*p2!=0 || errno!=0 || isnan(my_max_value))
			    {
			    cerr << "Bad max value "<< argv[optind] << endl;
			    return (EXIT_FAILURE);
			    }
			this->user_max_value=&my_max_value;
			}
		    ARGVCOL("-c",chromCol)
		    ARGVCOL("-v",valueCol)
		    ARGVCOL("-r",colorCol)
		    ARGVCOL("-s",sampleCol)
		    else if(std::strcmp(argv[optind],"-p")==0 && optind+1<argc)
		     	{
		     	if(!numeric_cast<int>(argv[++optind],&(this->posStartCol)) || this->posStartCol<1)
				{
				cerr << "Bad  POS value "<< argv[optind] << endl;
				return (EXIT_FAILURE);
				}
			this->posStartCol--;
			this->posEndCol=this->posStartCol;
		     	}
		    else if(std::strcmp(argv[optind],"-b")==0 && optind+2<argc)
		    	{
		    	if(!numeric_cast<int>(argv[++optind],&(this->posStartCol)) || this->posStartCol<1)
		    		{
		    		 cerr << "Bad start col value "<< argv[optind] << endl;
			    	return (EXIT_FAILURE);
		    		}
		    	if(!numeric_cast<int>(argv[++optind],&(this->posEndCol)) || this->posEndCol<1)
		    		{
		    		 cerr << "Bad end col value "<< argv[optind] << endl;
			    	return (EXIT_FAILURE);
		    		}
		    	this->posStartCol--;
		    	this->posEndCol--;
		    	}
		    else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind] << "'\n";
			usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else
			{
			break;
			}
		    ++optind;
		    }
	if(valueCol==-1)
	    {
	    cerr << "Column for VALUE undefined"<< endl;
	    usage(cerr,argc,argv);
	    return EXIT_FAILURE;
	    }
	if(sampleCol==-1)
	    {
	    Frame* f=new Frame;
	    f->sample.assign("");
	    f->owner=this;
	    sample2frame.insert(make_pair<string,Frame*>(f->sample,f));
	    }
	if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		this->readData(in);
		}
	else
		{
		while(optind< argc)
		    {
		    if(AbstractApplication::stopping()) break;
		    char* filename=argv[optind++];
		    igzstreambuf buf(filename);
		    istream in(&buf);
		    this->readData(in);
		    buf.close();
		    }
		}
	this->print();
	return EXIT_SUCCESS;
	}
    };


int main(int argc,char** argv)
    {
    Manhattan app;
    return app.main(argc,argv);
    }
