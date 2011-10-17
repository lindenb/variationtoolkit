#include <cstdlib>
#include <csignal>
#include <map>
#include <cerrno>
#include <cstring>
#include "segments.h"
#include "xcurses.h"
#include "application.h"
#include "auto_vector.h"
#include "auto_map.h"

using namespace std;

class Sample
    {
    public:
	string name;
	int column_index;
    };

class Data
    {
    public:
	   std::string qual;
	   std::string filter;
	   std::string info;
	   std::string format;
	   std::string call;
    };

class Row
    {
    public:
	ChromPosition* pos;
	string id;
	string ref;
	string alt;
	auto_vector<Data> data;

	Row()
	    {
	    }

	~Row()
	    {
	    if(pos!=0) delete pos;
	    }
    };



class Column
    {
    public:
        std::string label;
        int witdh;
        int x;
    };



class AngelHeart:public AbstractApplication
    {
    public:
    Screen* screen;
    int64_t cursor_y;
    int64_t cursor_x;
    auto_vector<Sample> samples;
    map<string,Sample*> sample2col;
    auto_vector<Column> columns;
    auto_vector<Row> rows;

    AngelHeart():cursor_y(0),cursor_x(0)
	{

	}

    void paintColumn(Column* col)
	{

	}

    void paintColumns()
	{
	for(size_t i=0;i< columns.size();++i)
	    {

	    }
	}

    void paintRows()
	{
	for(int y=0;screen->height();++y)
	    {

	    }
	}

    void repaint()
	{
	paintColumns();
	paintRows();
	}

    void loop()
	{
	rows.push_back(new Column);
	rows.back()->label.assign("CHROM");
	rows.push_back(new Column);
	rows.back()->label.assign("POS");
	rows.push_back(new Column);
	rows.back()->label.assign("ID");
	rows.push_back(new Column);
	rows.back()->label.assign("REF");
	rows.push_back(new Column);
	rows.back()->label.assign("ALT");
	for(int i=0;i< samples.size();++i)
	    {
	    Sample* sample=samples.at(i);
	    rows.push_back(new Column);
	    rows.back()->label.assign(sample->name).append(":QUAL");
	    rows.push_back(new Column);
	    rows.back()->label.assign(sample->name).append(":FILTER");
	    rows.push_back(new Column);
	    rows.back()->label.assign(sample->name).append(":INFO");
	    rows.push_back(new Column);
	    rows.back()->label.assign(sample->name).append(":FORMAT");
	    rows.push_back(new Column);
	    rows.back()->label.assign(sample->name).append(":CALL");
	    }


	screen=Screen::startup();
	screen->border();
	screen->refresh();
	for(;;)
	    {
	    if(screen->getch()=='q') break;
	    }

	Screen::shutdown();
	}

    void run(std::istream& in)
    	    {
    	    vector<string> tokens;
    	    string line;
    	    int chromCol=0;
    	    int posCol=1;
    	    int idCol=2;
    	    int refCol=3;
    	    int altCol=4;
    	    int sampleCol=-1;

    	    while(getline(in,line,'\n'))
    		    {
    		    if(AbstractApplication::stopping()) break;
    		    if(line.empty() || line[0]=='#') continue;
    		    tokenizer.split(line,tokens);

    		    string chrom=tokens[chromCol];
    		    chat *p2;
    		    int pos=(int)strtol(tokens[posCol].c_str(),&p2,10);
    		    string id=tokens[idCol];
    		    string ref=tokens[refCol];
    		    string alt=tokens[altCol];
    		    string sampleName=tokens[sampleCol];
    		    Row* therow=NULL;
    		    if(!rows.empty() &&
    			rows.back()->pos->chrom.compare(chrom)==0 &&
    			rows.back()->pos->pos==pos &&
    			rows.back()->ref.compare(ref)==0 &&
    			rows.back()->alt.compare(alt)==0
    			)
    			{
    			therow=rows.back();
    			}
    		    else
    			{
    			therow=new Row;
    			therow->pos=new ChromPosition(chrom,pos);
    			therow->id.assign(id);
    			therow->ref.assign(ref);
    			therow->alt.assign(alt);
    			rows.push_back(therow);
    			}
    		    int index_sample=0;
    		    if(sampleCol==-1)
    			{
    			if(sample2col.empty())
    			    {
    			    Sample* sample=new Sample;
    			    sample->name.assign("Sample");
    			    sample->column_index=0;
    			    samples.push_back(sample);
    			    }
    			index_sample=0;
    			}
    		    else
    			{
			map<string,Sample*>::iterator r= sample2col.find(sampleName);
			if(r==sample2col.end())
			    {
			    Sample* sample=new Sample;
			    sample->name.assign(sampleName);
			    sample->column_index=sample2col.size();
			    index_sample=sample->column_index;
			    samples.push_back(sample);
			    sample2col.put(sample->name,sample);
			    }
			else
			    {
			    index_sample=r->second->column_index;
			    }
    			}

    		    if(index_sample>=therow->data.size())
    			{
    			therow->data.resize(index_sample+1);
    			}
    		    Data* data=new Data;
    		    therow->data.assign(index_sample,data);
    		    }
    	    }

    int main(int argc,char ** argv)
	{
	if(optind==argc)
	    {
	    run(cin);
	    }
	else
	    {
	    while(optind< argc)
		{
		char* filename=argv[optind++];
		fstream in(filename,ios::in);
		if(!in.is_open())
		    {
		    cerr << "Cannot open "<< filename << " " << strerror(errno) << endl;
		    return EXIT_FAILURE;
		    }
		run(in);
		in.close();
		}
	    }
	loop();
	return 0;
	}
    };

int main(int argc,char** argv)
    {
    AngelHeart app;
    return app.main(argc,argv);
    }
