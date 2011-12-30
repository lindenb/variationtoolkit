#include <cstdlib>
#include <sys/types.h>
#include <regex.h>
#include "xregex.h"

#define CAST_MATCHER(a) ((regmatch_t*)a->_ptr)


Matcher::Matcher(void* p):_ptr(p)
    {
    }

Matcher::~Matcher()
    {
    std::free(_ptr);
    }


int32_t Matcher::begin() const
    {
    return CAST_MATCHER(this)->rm_so;
    }

int32_t Matcher::end() const
    {
    return CAST_MATCHER(this)->rm_eo;
    }


#define CAST_REGEX(a) ((regex_t*)a->_ptr)

Pattern::Pattern(void* p):_ptr(p)
    {
    }

 Pattern::~Pattern()
    {
    ::regfree(CAST_REGEX(this));
    std::free(_ptr);
    }



std::auto_ptr<Matcher> Pattern::exec(const char* s)
    {
    regmatch_t* m=(regmatch_t*)std::malloc(sizeof(regmatch_t));
    if(m==NULL) THROW("Cannot allocate regmatch_t");
    int err=0;
    int flags=0;
    if((err=::regexec(CAST_REGEX(this),s,1,m,flags))!=0)
	{
	if(err==REG_NOMATCH)
	    {
	    m->rm_eo=-1;
	    m->rm_so=-1;
	    return std::auto_ptr<Matcher>(new Matcher((void*)m));
	    }
	char err_msg[100];
	::regerror(err, CAST_REGEX(this), err_msg, 100);
	std::free((void*)reg);
	THROW("regexec failure.:"<< err_msg);
	}
    return std::auto_ptr<Matcher>(new Matcher((void*)m));
    }



Regex::Regex():case_sensible(true),
	extended(false),
	no_sub(false),
	new_line(false)
    {
    }

Regex::~Regex()
    {
    }

Regex& Regex::set_case_sensible(bool choice)
    {
    this->case_sensible=choice;
    return *this;
    }

Regex& Regex::set_extended(bool choice)
    {
    this->extended=choice;
    return *this;
    }

Regex& Regex::set_no_sub(bool choice)
    {
    this->no_sub=choice;
    return *this;
    }

Regex& Regex::set_new_line(bool choice)
    {
    this->new_line=choice;
    return *this;
    }

std::auto_ptr<Pattern> Regex::compile(const char* s)
    {
    int err;
    int cflags=0;
    regex_t* reg=(regex_t*)std::malloc(sizeof(regex_t));
    if(reg==NULL) THROW("Cannot allocate regex_t");
    if(!case_sensible) cflags |= REG_ICASE;
    if(extended) cflags |= REG_EXTENDED;
    if(no_sub) cflags |= REG_NOSUB;
    if(new_line) cflags |= REG_NEWLINE;
    if((err=::regcomp(reg,s,cflags))!=0)
	{
	char err_msg[100];
	::regerror(err, reg, err_msg, 100);
	std::free((void*)reg);
	THROW("regcomp failure.:"<< err_msg);
	}
    return std::auto_ptr<Pattern>(new Pattern(reg));
    }

