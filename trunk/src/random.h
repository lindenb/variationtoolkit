#ifndef RANDOM_H
#define RANDOM_H

class Random
    {
    public:
	double rnd();
	int32_t rnd(int32_t end);
	int32_t rnd(int32_t start,int32_t end);
	bool boolean();
	int32_t sign();
    };

#endif
