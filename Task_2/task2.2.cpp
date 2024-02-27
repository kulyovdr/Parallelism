#include <iostream>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <cmath>

const double PI = 3.14159265358979323846;

const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

double func(double x)
{
    return exp(-x * x);
}

const std::chrono::duration<double> integrate_omp(double (*func)(double), double a, double b, int n, double &sum, int numThr)
{
    const auto start{std::chrono::steady_clock::now()};
    double h = (b - a) / n;
    sum = 0.0;
    #pragma omp parallel num_threads(numThr)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);
        double sumloc = 0.0;
        for (int i = lb; i <= ub; i++)
        {
            sumloc += func(a + h * (i + 0.5));
        }
        #pragma omp atomic
        sum += sumloc;
}
    sum *= h;
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    return elapsed_seconds;
}

int main(int argc, char *argv[])
{
    int threads[] = {1, 2, 4, 7, 8, 16, 20, 40};

    double sum = 0.0;
    for (auto i : threads)
    {
        const std::chrono::duration<double> time = integrate_omp(func, a, b, nsteps, sum, i);
        std::cout << i << " threads: " << time.count() << " seconds, sum = " << sum << std::endl;
    }

    return 0;
}