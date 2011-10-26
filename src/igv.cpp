/*
 * igv.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "tablemodel.h"
#include "application.h"
#include "zstreambuf.h"
#include "varkitversion.h"
#include "xcurses.h"

using namespace std;
class IGVBrowser:public AbstractApplication
    {
    public:
	DefaultTableModel* model;
	vector<size_t> columns_length;
	size_t top_y;
	size_t row_y;
	IGVBrowser()
	    {
	    model=new DefaultTableModel;
	    top_y=0;
	    row_y=0;
	    }
	~IGVBrowser()
	    {
	    delete model;
	    }
	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    size_t nLine=0;
	    while(getline(in,line,'\n'))
		{
		++nLine;
		tokenizer.split(line,tokens);
		if(nLine==1)
		    {
		    model->header=tokens;
		    continue;
		    }
		model->table.push_back(tokens);
		}
	    }

	 void usage(ostream& out,int argc,char** argv)
	    {
	    out << endl;
	    out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << VARKIT_REVISION <<".\n";
	    out << endl;
	    }
	 void repaint()
	     {
	     Screen* screen=Screen::getInstance();
	     screen->clear();
	     for(int y=0;y< screen->height();++y)
		 {
		 if((y+top_y)>=model->rows()) continue;
		 ostringstream os;
		 for(size_t i=0;i< model->columns();++i)
		     {
		     if(i>0)os << " ";
		     const char* p=model->get((y+top_y),i);
		     if(p==NULL) continue;
		     os << p;
		     }
		 string line=os.str();
		 for(int i=0;i< screen->width() && i<(int)line.size();++i)
		     {
		     screen->set(y,i,line[i]);
		     }
		 }
	     screen->refresh();
	     }

	int main(int argc,char** argv)
	    {
	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    char* p=argv[++optind];
		    if(strlen(p)!=1)
			{
			cerr << "Bad delimiter \""<< p << "\"\n";
			usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    tokenizer.delim=p[0];
		    }
		else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
		    usage(cerr,argc,argv);
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
	   	    run(in);
	   	    }
	       else if(optind+1==argc)
	   	    {
	   	    char* filename=argv[optind++];
	   	    igzstreambuf buf(filename);
	   	    istream in(&buf);
	   	    run(in);
	   	    buf.close();
	   	    }
	       else
	   	    {
	   	    cerr << "Illegal number of arguments" << endl;
	   	    usage(cerr,argc,argv);
	   	    return EXIT_FAILURE;
	   	    }

	    if(model->rows()==0)
		{
		cerr << "No data."<< endl;
		return EXIT_FAILURE;
		}

	    columns_length.resize(model->columns(),0);


	    Screen* screen=Screen::startup();
	    screen->refresh();

	    repaint();
	    for(;;)
		{
		int c=screen->getch();
		if(c==-1)
		    {
		    continue;
		    }
		//cerr << c << endl;
		if(c== 'q')
		    {
		    break;
		    }
		else if(c=='a' || c==Screen::K_UP)
		    {
		    if(row_y>0)
			{
			row_y--;
			}
		    else if(top_y>0)
			{
			top_y--;
			}
		    repaint();
		    }
		else if(c=='z' ||c==Screen::K_DOWN)
		    {
		    if( (int)row_y+1< screen->height() &&
			((int)top_y+(int)row_y+1)<screen->height()
			)
			{
			row_y++;
			}
		    if(top_y+1< model->rows())
			{
			top_y++;
			repaint();
			}
		    }
		else
		    {
		    cerr << c << endl;
		    }
		}

	    Screen::shutdown();
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    IGVBrowser app;
    return app.main(argc,argv);
    }
