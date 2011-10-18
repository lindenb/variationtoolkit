/*
 * xcurses.cpp
 *
 *  Created on: Oct 15, 2011
 *      Author: lindenb
 */
#ifndef XCURSES_H
#define XCURSES_H
#include <stdint.h>



class Window
    {
	protected:
		Window();
    public:

	typedef int32_t pixel_t;

	virtual ~Window();
	bool moveto(int y,int x);
	bool erase( int y, int x);
	bool erase();

	virtual int x();
	virtual int y();
	virtual int width();
	virtual int height();
	virtual int get();
	virtual pixel_t get(int y,int x);
	virtual void* ptr()=0;
	virtual void border();
	virtual void scroll(bool b);
	virtual void clear();
	virtual int getch();
	virtual void refresh();
    };

class Screen:public Window
    {
    public:
	Screen();
	virtual ~Screen();
	virtual void* ptr();

	static Screen* getInstance();
	static Screen* startup();
	static bool shutdown();
    private:
	static Screen* INSTANCE;
    };


class DelegateWindow:public Window
    {
    public:
	DelegateWindow(void* ptr,bool owner);
	virtual ~DelegateWindow();
	virtual void* ptr();
    private:
	void* _ptr;
	bool owner;
    };

class DefaultWindow:public Window
    {
    public:
	DefaultWindow(int nlines, int ncols, int begin_y, int begin_x);
	virtual ~DefaultWindow();
	virtual void* ptr();
    private:
	void* _ptr;
    };

#endif
