#include <iostream>
#include <boost/program_options.hpp>
#include <cmath>
#include <memory>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <chrono>

#include "nvtx3/nvToolsExt.h"
#include "omp.h"

namespace opt = boost::program_options;

#define OFFSET(x, y, m) (((x) * (m)) + (y))

double linearInterpolation(double x, double x1, double y1, double x2, double y2)
{
    return y1 + ((x - x1) * (y2 - y1) / (x2 - x1));
}

void initialize(std::shared_ptr<double[]> A, std::shared_ptr<double[]> Anew, int n)  
{  
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[OFFSET(i, j, n)] = Anew[OFFSET(i, j, n)] = 0;
        }
    }
  
    A[OFFSET(0, 0, n)] = Anew[OFFSET(0, 0, n)] = 10;
    A[OFFSET(0, n - 1, n)] = Anew[OFFSET(0, n - 1, n)] = 20;
    A[OFFSET(n - 1, n - 1, n)] = Anew[OFFSET(n - 1, n - 1, n)] = 30;
    A[OFFSET(n - 1, 0, n)] = Anew[OFFSET(n - 1, 0, n)] = 20;

    for (size_t i = 1; i < n - 1; i++)
    {
        A[OFFSET(0, i, n)] = linearInterpolation(i, 0.0, A[OFFSET(0, 0, n)], n - 1, A[OFFSET(0, n - 1, n)]);
        A[OFFSET(i, 0, n)] = linearInterpolation(i, 0.0, A[OFFSET(0, 0, n)], n - 1, A[OFFSET(n - 1, 0, n)]);
        A[OFFSET(i, n - 1, n)] = linearInterpolation(i, 0.0, A[OFFSET(0, n - 1, n)], n - 1, A[OFFSET(n - 1, n - 1, n)]);
        A[OFFSET(n - 1, i, n)] = linearInterpolation(i, 0.0, A[OFFSET(n - 1, 0, n)], n - 1, A[OFFSET(n - 1, n - 1, n)]);

        Anew[OFFSET(0, i, n)] = linearInterpolation(i, 0.0, Anew[OFFSET(0, 0, n)], n - 1, Anew[OFFSET(0, n - 1, n)]);
        Anew[OFFSET(i, 0, n)] = linearInterpolation(i, 0.0, Anew[OFFSET(0, 0, n)], n - 1, Anew[OFFSET(n - 1, 0, n)]);
        Anew[OFFSET(i, n - 1, n)] = linearInterpolation(i, 0.0, Anew[OFFSET(0, n - 1, n)], n - 1, Anew[OFFSET(n - 1, n - 1, n)]);
        Anew[OFFSET(n - 1, i, n)] = linearInterpolation(i, 0.0, Anew[OFFSET(n - 1, 0, n)], n - 1, Anew[OFFSET(n - 1, n - 1, n)]);
    }
}

double calcNext(std::shared_ptr<double[]> A, std::shared_ptr<double[]> Anew, int n)
{
    double error = 0.0;
    for( int j = 1; j < n - 1; j++)
    {
        for( int i = 1; i < n - 1; i++ )
        {
            Anew[OFFSET(j, i, n)] = 0.25 * ( A[OFFSET(j, i+1, n)] + A[OFFSET(j, i-1, n)]
                                           + A[OFFSET(j-1, i, n)] + A[OFFSET(j+1, i, n)]);
            error = fmax( error, fabs(Anew[OFFSET(j, i, n)] - A[OFFSET(j, i , n)]));
        }
    }
    return error;
}

void swap(std::shared_ptr<double[]> A, std::shared_ptr<double[]> Anew, int n)
{
    for( int j = 1; j < n - 1; j++)
    {
        for( int i = 1; i < n - 1; i++ )
        {
            A[OFFSET(j, i, n)] = Anew[OFFSET(j, i, n)];    
        }
    }
}

int main(int argc, char** argv)
{
    opt::options_description desc("опции");
    desc.add_options()
        ("accuracy",opt::value<double>()->default_value(1e-6),"точность")
        ("cellsCount",opt::value<int>()->default_value(256),"размер матрицы")
        ("iterCount",opt::value<int>()->default_value(1000000),"количество операций")
        ("help","помощь");

    const int n = 128;
    const int iter_max = 1000000;

    const double tol = 1.0e-6;
    double error = 1.0;

    std::shared_ptr<double[]> A(new double[n * n]);
    std::shared_ptr<double[]> Anew(new double[n * n]);

    nvtxRangePushA("init");
    initialize(A, Anew, n);
    nvtxRangePop();

    double st = omp_get_wtime();
    int iter = 0;

    nvtxRangePushA("while");
    while ( error > tol && iter < iter_max )
    {
        nvtxRangePushA("calc");
        error = calcNext(A, Anew, n);
        nvtxRangePop();

        nvtxRangePushA("swap");
        swap(A, Anew, n);
        nvtxRangePop();

        if(iter % 100 == 0)
        {
            printf("%5d, %0.6f\n", iter, error);
        }

        iter++;
    }
    nvtxRangePop();

    double runtime = omp_get_wtime() - st;

    std::cout << " total: " << runtime << " s" << std::endl;

    return 0;
}