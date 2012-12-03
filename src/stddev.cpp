#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

int main(int argc,char** argv)
    {
    double total=0;
    double value=0.0;
    vector<double> all;
    int ret;
    while((ret=fscanf(stdin, "%lf", &value))!=-1)
	{
	if(ret!=1)
	    {
	    int c;
	    clearerr(stdin);
	    if((c=fgetc(stdin))==EOF) break;
	    cerr << "I/O warning cannot convert input to double '"<< (char)c << "'" << endl;
	    continue;
	    }
	all.push_back(value);
	total+=value;
	}
    if(all.empty())
	{
	return EXIT_FAILURE;
	}

    ::sort(all.begin(),all.end());
    double mean=total/all.size();
    double stddev=0;
    for(vector<double>::iterator r=all.begin();r!=all.end();++r)
	{
	stddev+=abs(*r - mean);
	}
    stddev/=all.size();

    cout	<< all.size() << "\t"
		<< all.front() << "\t"
		<< all.back() << "\t"
		<< all[all.size()/2] << "\t"
		<< mean<< "\t"
		<< stddev
		<< endl;
    return EXIT_SUCCESS;
    }
