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
	static const pixel_t K_A1;
	static const pixel_t K_A3;
	static const pixel_t K_B2;
	static const pixel_t K_BACKSPACE;
	static const pixel_t K_BEG;
	static const pixel_t K_BTAB;
	static const pixel_t K_C1;
	static const pixel_t K_C3;
	static const pixel_t K_CANCEL;
	static const pixel_t K_CATAB;
	static const pixel_t K_CLEAR;
	static const pixel_t K_CLOSE;
	static const pixel_t K_COMMAND;
	static const pixel_t K_COPY;
	static const pixel_t K_CREATE;
	static const pixel_t K_CTAB;
	static const pixel_t K_DC;
	static const pixel_t K_DL;
	static const pixel_t K_DOWN;
	static const pixel_t K_EIC;
	static const pixel_t K_END;
	static const pixel_t K_EOL;
	static const pixel_t K_EOS;
	static const pixel_t K_EXIT;
	static const pixel_t K_FIND;
	static const pixel_t K_HELP;
	static const pixel_t K_HOME;
	static const pixel_t K_IC;
	static const pixel_t K_IL;
	static const pixel_t K_LEFT;
	static const pixel_t K_LL;
	static const pixel_t K_MARK;
	static const pixel_t K_MAX;
	static const pixel_t K_MESSAGE;
	static const pixel_t K_MOUSE;
	static const pixel_t K_MOVE;
	static const pixel_t K_NEXT;
	static const pixel_t K_NPAGE;
	static const pixel_t K_OPEN;
	static const pixel_t K_OPTIONS;
	static const pixel_t K_PPAGE;
	static const pixel_t K_PREVIOUS;
	static const pixel_t K_PRINT;
	static const pixel_t K_REDO;
	static const pixel_t K_REFERENCE;
	static const pixel_t K_REFRESH;
	static const pixel_t K_REPLACE;
	static const pixel_t K_RESET;
	static const pixel_t K_RESTART;
	static const pixel_t K_RESUME;
	static const pixel_t K_RIGHT;
	static const pixel_t K_SAVE;
	static const pixel_t K_SBEG;
	static const pixel_t K_SCANCEL;
	static const pixel_t K_SCOMMAND;
	static const pixel_t K_SCOPY;
	static const pixel_t K_SCREATE;
	static const pixel_t K_SDC;
	static const pixel_t K_SDL;
	static const pixel_t K_SELECT;
	static const pixel_t K_SEND;
	static const pixel_t K_SEOL;
	static const pixel_t K_SEXIT;
	static const pixel_t K_SF;
	static const pixel_t K_SFIND;
	static const pixel_t K_SHELP;
	static const pixel_t K_SHOME;
	static const pixel_t K_SIC;
	static const pixel_t K_SLEFT;
	static const pixel_t K_SMESSAGE;
	static const pixel_t K_SMOVE;
	static const pixel_t K_SNEXT;
	static const pixel_t K_SOPTIONS;
	static const pixel_t K_SPREVIOUS;
	static const pixel_t K_SPRINT;
	static const pixel_t K_SR;
	static const pixel_t K_SREDO;
	static const pixel_t K_SREPLACE;
	static const pixel_t K_SRESET;
	static const pixel_t K_SRIGHT;
	static const pixel_t K_SRSUME;
	static const pixel_t K_SSAVE;
	static const pixel_t K_SSUSPEND;
	static const pixel_t K_STAB;
	static const pixel_t K_UNDO;
	static const pixel_t K_UP;

	virtual ~Window();
	bool moveto(int y,int x);
	bool erase( int y, int x);
	bool erase();

	virtual int x();
	virtual int y();
	virtual int width();
	virtual int height();
	virtual pixel_t getch(int y,int x);
	virtual int set(Window::pixel_t c);
	virtual int set(int y,int x,Window::pixel_t c);
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
