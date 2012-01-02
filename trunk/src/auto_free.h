#ifndef AUTO_FREE_H
#define AUTO_FREE_H
#include <memory>
#include <cassert>

template<typename T>
class auto_free
    {
    public:
	 typedef T element_type;
	 typedef void (*fun) (void*);
    private:
	 element_type* _ptr;
	 fun _fun;
    public:

	 auto_free(element_type* ptr,fun release):_ptr(ptr),_fun(release)
	     {
	     }
	 ~auto_free()
	     {
	     if(_ptr!=0 && _fun!=0) _fun((void*)_ptr);
	     }
	 bool nil() const
	     {
	     return _ptr==0;
	     }
	 void reset()
	     {
	     if(_ptr!=0 && _fun!=0) _fun((void*)_ptr);
	     _ptr=0;
	     }

	 element_type* operator->() const throw()
	       {
	 	assert(_ptr != 0);
	 	return _ptr;
	       }
	 element_type*
	      get() const throw() { return _ptr; }
    private:
	 auto_free(const auto_free& cp)
	     {
	     }
    };


#endif
