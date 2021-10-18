#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

using namespace std;

class Algo
{
    public:
        Algo();
        void test();

        /**calculating the gradient for linear regression*/
        double slope(const vector<double>& x, const vector<double>& y);
        /**calculating y-intercept*/
        double intercept(const vector<double>& x, const vector<double>& y, double gradient);
        /**predicting future prices using LR*/
        double predictYValue(double& slope, double& intercept, int x);

    private:

};