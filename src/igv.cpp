/*
 * igv.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "xcurses.h"
#include "tablemodel.h"
#include "application.h"
#include "zstreambuf.h"
#include "varkitversion.h"
#include "xcurses.h"

using namespace std;


#define WIDTH 80
#define HEIGHT 10

int startx = 0;
int starty = 0;

char *choices[] = {
			"Choice 1",
			"Choice 2",
			"Choice 3",
			"Choice 4",
			"Exit",
		  };
int n_choices = sizeof(choices) / sizeof(char *);

class IGVBrowser:public AbstractApplication
    {
    public:
	 DefaultTableModel* model;

	Window* menu_win;
	vector<size_t> columns_length;
	size_t top_y;
	size_t row_y;
	IGVBrowser():menu_win(NULL)
	    {
		model=new DefaultTableModel;
	    top_y=0;
	    row_y=0;
	    }
	~IGVBrowser()
	    {
		delete model;
	    }

	void print_menu(int highlight)
	    {
		int x, y, i;
		 x = 2;
		y = 2;
		menu_win->border();

			for(i = 0; i < n_choices; ++i)
			{	if(highlight == i + 1) /* High light the present choice
		*/
				{	menu_win->attron(Window::ATTR_REVERSE);
					menu_win->printf(y, x, "%s", choices[i]);
					menu_win->attroff(Window::ATTR_REVERSE);
				}
				else
					menu_win->printf(y, x, "%s", choices[i]);
				++y;
			}
			menu_win->refresh();
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
				buf.close();
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

		int highlight = 1;
	    Screen* screen=Screen::startup();
	    int choice=0;

	    /* initscr();
	    	clear();
	    	noecho();
	    	cbreak();	  */
	    	startx = (80 - WIDTH) / 2;
	    	starty = (24 - HEIGHT) / 2;

	    	menu_win = new DefaultWindow(screen->height(),screen->width(), 0, 0);
	    	menu_win->keypad(true);
	    	menu_win->scroll(false);
	    	menu_win->printf(1,1, "Use arrow keys to go up and down, Press enter to select a choice");
	    	menu_win->refresh();
	    	print_menu(highlight);
	    	while(1)
	    	{
	    		int c = menu_win->getch();
	    		if(c=='a' || c==Window::K_UP)
					{
					if(highlight == 1)
						highlight = n_choices;
					else
						--highlight;
					}
	    		else if(c=='z' || c==Window::K_DOWN)
					{
					if(highlight == n_choices)
						highlight = 1;
					else
						++highlight;
					}
	    		else if(c==10 || c=='q')
	    			{
	    			choice = highlight;
	    			}
	    		else
	    			{
	    			screen->printf(24, 0, "Character pressed is = %d Hopefully it can be printed as '%c'", c, c);
	    			screen->refresh();
	    			}

	    		print_menu(highlight);
	    		if(choice != 0)	/* User did a choice come out of the
	    infinite loop */
	    			break;
	    	}
	    	screen->printf(23, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
	    	/*clrtoeol();
	    	refresh();
	    	endwin();*/
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
