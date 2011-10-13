#ifndef WHERE_H
#define WHERE_H
#include <iostream>

#ifdef NOWHERE

#define WHERE(a) do { } while(0)

#else

#define WHERE(a) std::clog << __FILE__ << ":" << __LINE__ << \
    ":" << __FUNCTION__ << ":\"" << a << "\"" << std::endl
#endif
#endif
