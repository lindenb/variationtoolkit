/*
 * igv.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "xcurses.h"
#include "application.h"
#include "zstreambuf.h"
#include "varkitversion.h"
#include "xcurses.h"

using namespace std;




class IGVBrowser:public AbstractApplication
    {
    public:
	Window* menu_win;
	vector<size_t> columns_length;
	vector<vector<string> > table;
	int top_y;
	int sel_row;
	int igvport;
	string igvhost;
	int chromCol;
	int posCol;

	IGVBrowser():menu_win(NULL),chromCol(0),posCol(1)
	    {
	    top_y=0;
	    sel_row=0;
	    igvhost.assign("127.0.0.1");
	    igvport=60151;
	    }
	~IGVBrowser()
	    {
	    }

	int selecty()
	    {
	    return sel_row-top_y;
	    }

	void error(const char* s)
		{
		cerr << s << endl;
		Screen::beep();
		}

	void call(const char* chromosome,int position)
		{
		struct sockaddr_in serv_addr;
		struct hostent *server;

		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		    {
		    error("Cannot socket");
		    return;
		    }
		server = ::gethostbyname(igvhost.c_str());
		if (server == NULL)
		    {
			error("Cannot gethostbyname");
		    return;
		    }
		::bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		::bcopy((char *)server->h_addr,
		     (char *)&serv_addr.sin_addr.s_addr,
		     server->h_length
		     );
		serv_addr.sin_port = htons(igvport);
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		    {
			error("Cannot connect");
		    return;
		    }
		char buffer[255];
		snprintf(buffer,255,"goto %s:%d",
			chromosome,
			position
			);
		int n = write(sockfd,buffer,strlen(buffer));
		if (n < 0)
		    {
		    Screen::beep();
		    return;
		    }
		clock_t now=clock();
		while((clock()-now)/(float)CLOCKS_PER_SEC< 1.0f)
			{
			//cerr << ((clock()-now)/(float)CLOCKS_PER_SEC)<< endl;
			}
		/*
		bzero(buffer,255);
		n = read(sockfd,buffer,255);
		if (n < 0)
		    {
			error("Cannot read");
		    return;
		    }*/
		close(sockfd);
		}


	void repaint()
	    {
	    menu_win->clear();
	    for(int y=0;y< menu_win->height();++y)
		{
		int row_y=top_y+y;
		if(row_y>=(int)table.size()) break;
		const vector<string>& tokens=table[row_y];
		ostringstream os;
		for(size_t t=0;t< tokens.size();++t)
		    {
		    if(t>0) os << " ";
		    const string& s=tokens[t];
		    os << s;
		    for(size_t L=s.size();L<columns_length[t];++L)
			{
			os << " ";
			}
		    }
		string row(os.str());
		if(sel_row==row_y)
		    {
		    menu_win->attron(Window::ATTR_REVERSE);
		    }
		for(size_t t=0;t< row.size() && (int)t < menu_win->width();++t)
		    {
		    menu_win->set(y,t,row[t]);
		    }
		if(sel_row==row_y)
		    {
		    menu_win->attroff(Window::ATTR_REVERSE);
		    }
		}

	    menu_win->refresh();
	    }


	void run(std::istream& in)
		{
		vector<string> tokens;
		string line;
		while(getline(in,line,'\n'))
		    {
		    tokenizer.split(line,tokens);
		    while(columns_length.size() < tokens.size())
			{
			columns_length.push_back(0);
			}
		    for(size_t i=0;i< tokens.size();++i)
			{
			if(columns_length[i]<tokens[i].size())
			    {
			    columns_length[i]=tokens[i].size();
			    }
			}
		    table.push_back(tokens);
		    }
		}


	 void usage(ostream& out,int argc,char** argv)
	    {
	    out << endl;
	    out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << VARKIT_REVISION <<".\n";
	    out << "Options:"<< endl;
	    out << " -d (char) delimiter. default:tab"<< endl;
	    out << " -c (index) CHROM column. default:"<< (chromCol+1)<<endl;
	    out << " -p (index) POS column. default:"<<  (posCol+1)<<endl;
	    out << endl;
	    }


	int main(int argc,char** argv)
	    {
	    int optind=1;
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
		else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			char* p2;
			chromCol=strtol(argv[++optind],&p2,10);
		    if(chromCol<1) THROW("Bad CHROM index in "<< argv[optind]);
			chromCol--;
			}
		else if(std::strcmp(argv[optind],"-p")==0 && optind+1<argc)
			{
			char* p2;
			posCol=strtol(argv[++optind],&p2,10);
			if(posCol<1) THROW("Bad POS index in "<< argv[optind]);
			posCol--;
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

	    if(optind+1==argc)
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

	    if(table.size()==0)
			{
			cerr << "No data."<< endl;
			return EXIT_FAILURE;
			}


	    Screen* screen=Screen::startup();
	    screen->border();
	    screen->refresh();
	    menu_win = new DefaultWindow(screen->height()-2,screen->width()-2, 1, 1);
	    menu_win->keypad(true);
	    menu_win->scroll(false);

	    repaint();
	    for(;;)
		{
		int c = menu_win->getch();
		if(c==Window::K_UP)
		    {
		    if(selecty()>0)
			{
			sel_row--;
			}
		    else if(top_y>0)
			{
			top_y--;
			sel_row--;
			}
		    }
		else if(c==Window::K_DOWN)
		    {
		    if(selecty()+1 < (int)menu_win->height() &&
			sel_row+1< (int)table.size())
			{
			sel_row++;
			}
		    else if(
			selecty()+1== (int)menu_win->height() &&
			sel_row+1 < (int)table.size())
			{
			top_y++;
			sel_row++;
			}
		    }
		else if(c=='\n' || c=='\r' ||
			c==Window::K_LEFT ||
			c==Window::K_RIGHT
			)
		    {
			const vector<string>& r=table[sel_row];
			if(chromCol>=(int)r.size())
				{
				Screen::beep();
				continue;
				}
			if(posCol>=(int)r.size())
				{
				Screen::beep();
				continue;
				}
			char* p2;
			int pos=strtol(r[posCol].c_str(),&p2,10);
			if(pos<1 || *p2!=0)
				{
				error("bad pos");
				continue;
				}
			call(r[chromCol].c_str(),pos);
		    }
		else if(c=='q')
		    {
		    break;
		    }
		else
		    {
		    Screen::flash();
		    }

		repaint();
		}

	    delete menu_win;
	    Screen::shutdown();
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    IGVBrowser app;
    return app.main(argc,argv);
    }
