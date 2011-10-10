#ifndef THROW_EXCEPT_H
#define THROW_EXCEPT_H

#include <stdexcept>
#include <sstream>

#define THROW(a) do{std::ostringstream _os;\
	_os << __FILE__ << ":"<< __LINE__ << ":" << __FUNCTION__ << ":" << a << std::endl;\
	throw std::runtime_error(_os.str());\
	}while(0)
	

#endif
