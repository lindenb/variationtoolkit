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

#include "xcurses.h"

#define CASTWIN(a) ((WINDOW*)a)

using namespace std;

Screen* Screen::INSTANCE=NULL;




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

int Window::getch()
    {
    return ::wgetch(CASTWIN(ptr()));
    }

Window::pixel_t Window::get(int y,int x)
    {
    return moveto(y,x)?get():-1;
    }

Window::pixel_t Window::get()
    {
    return ::wgetch(CASTWIN(ptr()));
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

 Screen* Screen::startup()
    {
    if(::initscr()==NULL)
	{
	cerr << "Cannot initialize curses" << endl;
	return false;
	}
    ::nonl();        /* tell curses not to do NL->CR/NL on output */
    ::cbreak();      /* take input chars one at a time, no wait for \n */
    ::noecho();      /* don't echo input */

    if (has_colors())
	{
	start_color();

	/*
	 * Simple color assignment, often all we need.
	 */
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

DefaultWindow::DefaultWindow(int nlines, int ncols, int begin_y, int begin_x)
    {

    }

DefaultWindow::~DefaultWindow()
    {
    ::delwin(CASTWIN(ptr()));
    }

void* DefaultWindow::ptr()
    {
    return _ptr;
    }
