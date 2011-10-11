#ifndef WHERE_H
#define WHERE_H
#include <iostream>
#define WHERE(a) std::clog << __FILE__ << ":" << __LINE__ << \
    ":" << __FUNCTION__ << ":\"" << a << "\"" << std::endl

#endif
