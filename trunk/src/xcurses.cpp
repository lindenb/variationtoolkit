/*
 * xcurses.cpp
 *
 *  Created on: Oct 15, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <ncurses.h>

#ifdef scroll
#undef scroll
#endif
#ifdef clear
#undef clear
#endif
#ifdef scroll
#undef scroll
#endif
#ifdef getch
#undef getch
#endif
#ifdef refresh
#undef refresh
#endif
#ifdef border
#undef border
#endif

#ifdef attron
#undef attron
#endif

#ifdef attroff
#undef attroff
#endif

#include "throw.h"
#include "xcurses.h"

#define CASTWIN(a) ((WINDOW*)a)

using namespace std;

Screen* Screen::INSTANCE=NULL;

const Window::pixel_t Window::K_A1 = KEY_A1;
const Window::pixel_t Window::K_A3 = KEY_A3;
const Window::pixel_t Window::K_B2 = KEY_B2;
const Window::pixel_t Window::K_BACKSPACE = KEY_BACKSPACE;
const Window::pixel_t Window::K_BEG = KEY_BEG;
const Window::pixel_t Window::K_BTAB = KEY_BTAB;
const Window::pixel_t Window::K_C1 = KEY_C1;
const Window::pixel_t Window::K_C3 = KEY_C3;
const Window::pixel_t Window::K_CANCEL = KEY_CANCEL;
const Window::pixel_t Window::K_CATAB = KEY_CATAB;
const Window::pixel_t Window::K_CLEAR = KEY_CLEAR;
const Window::pixel_t Window::K_CLOSE = KEY_CLOSE;
const Window::pixel_t Window::K_COMMAND = KEY_COMMAND;
const Window::pixel_t Window::K_COPY = KEY_COPY;
const Window::pixel_t Window::K_CREATE = KEY_CREATE;
const Window::pixel_t Window::K_CTAB = KEY_CTAB;
const Window::pixel_t Window::K_DC = KEY_DC;
const Window::pixel_t Window::K_DL = KEY_DL;
const Window::pixel_t Window::K_DOWN = KEY_DOWN;
const Window::pixel_t Window::K_EIC = KEY_EIC;
const Window::pixel_t Window::K_END = KEY_END;
const Window::pixel_t Window::K_EOL = KEY_EOL;
const Window::pixel_t Window::K_EOS = KEY_EOS;
const Window::pixel_t Window::K_EXIT = KEY_EXIT;
const Window::pixel_t Window::K_FIND = KEY_FIND;
const Window::pixel_t Window::K_HELP = KEY_HELP;
const Window::pixel_t Window::K_HOME = KEY_HOME;
const Window::pixel_t Window::K_IC = KEY_IC;
const Window::pixel_t Window::K_IL = KEY_IL;
const Window::pixel_t Window::K_LEFT = KEY_LEFT;
const Window::pixel_t Window::K_LL = KEY_LL;
const Window::pixel_t Window::K_MARK = KEY_MARK;
const Window::pixel_t Window::K_MAX = KEY_MAX;
const Window::pixel_t Window::K_MESSAGE = KEY_MESSAGE;
const Window::pixel_t Window::K_MOUSE = KEY_MOUSE;
const Window::pixel_t Window::K_MOVE = KEY_MOVE;
const Window::pixel_t Window::K_NEXT = KEY_NEXT;
const Window::pixel_t Window::K_NPAGE = KEY_NPAGE;
const Window::pixel_t Window::K_OPEN = KEY_OPEN;
const Window::pixel_t Window::K_OPTIONS = KEY_OPTIONS;
const Window::pixel_t Window::K_PPAGE = KEY_PPAGE;
const Window::pixel_t Window::K_PREVIOUS = KEY_PREVIOUS;
const Window::pixel_t Window::K_PRINT = KEY_PRINT;
const Window::pixel_t Window::K_REDO = KEY_REDO;
const Window::pixel_t Window::K_REFERENCE = KEY_REFERENCE;
const Window::pixel_t Window::K_REFRESH = KEY_REFRESH;
const Window::pixel_t Window::K_REPLACE = KEY_REPLACE;
const Window::pixel_t Window::K_RESET = KEY_RESET;
const Window::pixel_t Window::K_RESTART = KEY_RESTART;
const Window::pixel_t Window::K_RESUME = KEY_RESUME;
const Window::pixel_t Window::K_RIGHT = KEY_RIGHT;
const Window::pixel_t Window::K_SAVE = KEY_SAVE;
const Window::pixel_t Window::K_SBEG = KEY_SBEG;
const Window::pixel_t Window::K_SCANCEL = KEY_SCANCEL;
const Window::pixel_t Window::K_SCOMMAND = KEY_SCOMMAND;
const Window::pixel_t Window::K_SCOPY = KEY_SCOPY;
const Window::pixel_t Window::K_SCREATE = KEY_SCREATE;
const Window::pixel_t Window::K_SDC = KEY_SDC;
const Window::pixel_t Window::K_SDL = KEY_SDL;
const Window::pixel_t Window::K_SELECT = KEY_SELECT;
const Window::pixel_t Window::K_SEND = KEY_SEND;
const Window::pixel_t Window::K_SEOL = KEY_SEOL;
const Window::pixel_t Window::K_SEXIT = KEY_SEXIT;
const Window::pixel_t Window::K_SF = KEY_SF;
const Window::pixel_t Window::K_SFIND = KEY_SFIND;
const Window::pixel_t Window::K_SHELP = KEY_SHELP;
const Window::pixel_t Window::K_SHOME = KEY_SHOME;
const Window::pixel_t Window::K_SIC = KEY_SIC;
const Window::pixel_t Window::K_SLEFT = KEY_SLEFT;
const Window::pixel_t Window::K_SMESSAGE = KEY_SMESSAGE;
const Window::pixel_t Window::K_SMOVE = KEY_SMOVE;
const Window::pixel_t Window::K_SNEXT = KEY_SNEXT;
const Window::pixel_t Window::K_SOPTIONS = KEY_SOPTIONS;
const Window::pixel_t Window::K_SPREVIOUS = KEY_SPREVIOUS;
const Window::pixel_t Window::K_SPRINT = KEY_SPRINT;
const Window::pixel_t Window::K_SR = KEY_SR;
const Window::pixel_t Window::K_SREDO = KEY_SREDO;
const Window::pixel_t Window::K_SREPLACE = KEY_SREPLACE;
const Window::pixel_t Window::K_SRESET = KEY_SRESET;
const Window::pixel_t Window::K_SRIGHT = KEY_SRIGHT;
const Window::pixel_t Window::K_SRSUME = KEY_SRSUME;
const Window::pixel_t Window::K_SSAVE = KEY_SSAVE;
const Window::pixel_t Window::K_SSUSPEND = KEY_SSUSPEND;
const Window::pixel_t Window::K_STAB = KEY_STAB;
const Window::pixel_t Window::K_UNDO = KEY_UNDO;
const Window::pixel_t Window::K_UP = KEY_UP;

const int Window::ATTR_REVERSE=A_REVERSE;

Window::Window()
    {
    }

Window::~Window()
    {
    }


void Window::border()
    {
    wborder(CASTWIN(ptr()),0, 0,0,0, 0, 0, 0, 0);
    }

void Window::scroll(bool b)
    {
    ::wscrl(CASTWIN(ptr()),(int)b);
    }

void Window::clear()
    {
    ::wclear(CASTWIN(ptr()));
    }

void Window::refresh()
    {
    ::wrefresh(CASTWIN(ptr()));
    }

bool Window::moveto(int y,int x)
    {
    return ::wmove(CASTWIN(ptr()),y,x)!=ERR;
    }

bool Window::erase( int y, int x)
    {
    return moveto(y,x)?erase():false;
    }

bool Window::erase()
    {
    return ::wdelch(CASTWIN(ptr()))!=ERR;
    }

int32_t Window::getch()
    {
    return ::wgetch(CASTWIN(ptr()));
    }

int Window::set(Window::pixel_t ch)
    {
    return ::waddch(CASTWIN(ptr()),ch);
    }
int Window::set(int y,int x,Window::pixel_t c)
    {
    return moveto(y,x)?set(c):-1;
    }


int Window::getch(int y,int x)
    {
    return moveto(y,x)?getch():-1;
    }


int Window::x()
    {
    return getbegx(CASTWIN(ptr()));
    }

int Window::y()
    {
    return getbegy(CASTWIN(ptr()));
    }

int Window::width()
    {
    return getmaxx(CASTWIN(ptr()));
    }

int Window::height()
    {
    return getmaxy(CASTWIN(ptr()));
    }

int Window::keypad(bool sel)
    {
    return ::keypad(CASTWIN(ptr()), sel);
    }

int Window::nodelay(bool sel)
    {
    return ::nodelay(CASTWIN(ptr()), sel);
    }

int Window::printf(const char * fmt,...)
	{
	va_list vl;
	va_start(vl,fmt);
	int n= ::vwprintw(CASTWIN(ptr()),fmt,vl);
	va_end(vl);
	return n;
	}

int Window::printf(int y,int x,const char * fmt,...)
	{
	moveto(y,x);
	va_list vl;
	va_start(vl,fmt);
	int n= ::vw_printw(CASTWIN(ptr()),fmt,vl);
	va_end(vl);
	return n;
	}

int Window::attron(int att)
	{
	return ::wattron(CASTWIN(ptr()), att);
	}

int Window::attroff(int att)
	{
	return ::wattroff(CASTWIN(ptr()), att);
	}

int Window::attroff(int att,bool sel)
	{
	return sel?attron(att):attroff(att);
	}


Screen::Screen()
    {

    }

Screen::~Screen()
    {

    }

void* Screen::ptr()
    {
    return stdscr;
    }


Screen* Screen::getInstance()
    {
    if(INSTANCE==NULL)
		{
		INSTANCE=new Screen;
		}
    return INSTANCE;
    }

int Screen::getch()
	{
	return ::getch();
	}

 Screen* Screen::startup()
    {
    if(::initscr()==NULL)
		{
		cerr << "Cannot initialize curses" << endl;
		return NULL;
		}
    ::wclear(stdscr);
    //::nonl();        /* tell curses not to do NL->CR/NL on output */
    ::noecho();      /* don't echo input */
    ::cbreak();      /* take input chars one at a time, no wait for \n */
    ::nodelay(stdscr,TRUE);

    if(has_colors())
		{
		start_color();
		init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
		init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
		init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
		init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
		init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
		init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
		init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
		}
    return getInstance();
    }

bool Screen::shutdown()
    {
    bool b= ::endwin()==OK;
    if(INSTANCE!=NULL) delete INSTANCE;
    INSTANCE=NULL;
    return b;
    }



DelegateWindow::DelegateWindow(void* ptr,bool owner):_ptr(ptr),owner(owner)
    {
    }

DelegateWindow::~DelegateWindow()
    {
    if(owner && _ptr!=NULL) ::delwin(CASTWIN(_ptr));
    }

void* DelegateWindow::ptr()
    {
    return _ptr;
    }

DefaultWindow::DefaultWindow(int nlines, int ncols, int begin_y, int begin_x):_ptr(
		::newwin(nlines, ncols, begin_y, begin_x))
    {
	if(_ptr==NULL) THROW("Cannot create window");
    }

DefaultWindow::~DefaultWindow()
    {
    ::delwin(CASTWIN(ptr()));
    }

void* DefaultWindow::ptr()
    {
    return _ptr;
    }
