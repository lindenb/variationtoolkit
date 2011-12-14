#ifndef LOESS_STATS_H
#define LOESS_STATS_H
#include <stdint.h>
#include <vector>
#include <memory>

class Loess
    {
    public:
	struct Point
	    {
	    double x;
	    double y;
	    };
	Loess();
	~Loess();
	/** roportion of points in the plot which influence the smooth at each value */
	double smoother_span;
	/** the number of ‘robustifying’ iterations which should be performed. */
	int32_t nsteps;
	/** used to speed up computation */
	double delta_speed;

	std::auto_ptr<std::vector<double> > lowess(const double *x, const double *y, int32_t n);
    private:
	static double fsquare(double x);
	static double fcube(double x);

	void lowest(const double *x, const double *y, int n, const double *xs, double *ys,
		int nleft, int nright, double *w,
		bool userw, double *rw, bool *ok);

	void clowess(const double  *x, const double *y, int n,
		     double f, int nsteps, double delta,
		     double *ys, double *rw, double *res);

    };

#endif
