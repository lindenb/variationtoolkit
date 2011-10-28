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
#include "tokenizer.h"
using namespace std;


#define WIDTH 40
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

class IGVBrowser
    {
    public:
	Window* menu_win;
	vector<size_t> columns_length;
	size_t top_y;
	size_t row_y;
	IGVBrowser():menu_win(NULL)
	    {

	    top_y=0;
	    row_y=0;
	    }
	~IGVBrowser()
	    {

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



	int main(int argc,char** argv)
	    {
		int highlight = 1;
	    Screen* screen=Screen::startup();
	    int choice=0;

	    /* initscr();
	    	clear();
	    	noecho();
	    	cbreak();	  */
	    	startx = (80 - WIDTH) / 2;
	    	starty = (24 - HEIGHT) / 2;

	    	menu_win = new DefaultWindow(HEIGHT, WIDTH, starty, startx);
	    	menu_win->keypad(true);
	    	menu_win->printf(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
	    	menu_win->refresh();
	    	print_menu(highlight);
	    	while(1)
	    	{
	    		int c = menu_win->getch();
	    		switch(c)
	    		{	case 'a':
	    				if(highlight == 1)
	    					highlight = n_choices;
	    				else
	    					--highlight;
	    				break;
	    			case 'z':
	    				if(highlight == n_choices)
	    					highlight = 1;
	    				else
	    					++highlight;
	    				break;
	    			case 10:
	    				choice = highlight;
	    				break;
	    			default:
	    				screen->printf(24, 0, "Charcter pressed is = %d Hopefully it can be printed as '%c'", c, c);
	    				screen->refresh();
	    				break;
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
