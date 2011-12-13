#include <cstdlib>
#include "random.h"

using namespace std;

double Random::rnd()
    {
    return std::rand()/(double)RAND_MAX;
    }

int32_t Random::rnd(int32_t end)
    {
    return std::rand()%end;
    }

int32_t Random::rnd(int32_t start,int32_t end)
    {
    return start+rnd(end-start);
    }

bool Random::boolean()
    {
    return std::rand()%2==0;
    }

int32_t Random::sign()
    {
    return boolean()?1:-1;
    }
