#ifndef THROW_EXCEPT_H
#define THROW_EXCEPT_H

#include <stdexcept>
#include <sstream>

#define THROW(a) do{std::ostringstream _os;\
	_os << "\nFile : "<<  __FILE__ << "\nLine  : "<< __LINE__ << "\nMethod : " << __FUNCTION__ << "\nWhat : " << a << std::endl;\
	throw std::runtime_error(_os.str());\
	}while(0)
	

#endif
