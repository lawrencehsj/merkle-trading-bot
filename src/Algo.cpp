#include "Algo.h"
#include <iostream>
#include <vector>
#include <cmath>

Algo::Algo()
{

}
/**calculating the gradient for linear regression*/
double Algo::slope(const std::vector<double>& x, const std::vector<double>& y) {
    double sumX  = accumulate(x.begin(), x.end(), 0.0);
    double sumY  = accumulate(y.begin(), y.end(), 0.0);
    double sumX_X = inner_product(x.begin(), x.end(), x.begin(), 0.0);
    double sumX_Y = inner_product(x.begin(), x.end(), y.begin(), 0.0);
    double slope = ((x.size() * sumX_Y - sumX * sumY) *1.0 )/ ((x.size() * sumX_X - sumX * sumX) * 1.0 );
    return slope;
}
/**calculating y-intercept*/
double Algo::intercept(const std::vector<double>& x, const std::vector<double>& y, double slope)
{
    double x_size = x.size();
    double sumX  = accumulate(x.begin(), x.end(), 0.0);
    double sumY  = accumulate(y.begin(), y.end(), 0.0);
    return (sumY - slope * sumX) / x_size;
}

double Algo::predictYValue(double& slope, double& intercept, int x) //not in use atm
{
    return slope*x + intercept;
}