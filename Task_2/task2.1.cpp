#include <iostream>
#include <omp.h>
#include <chrono>
#include <algorithm>

const std::chrono::duration<double> dgemv_parallel(double** matr, double* vecIn, double* vecOut, int numThr, const int m, const int n)
{
    const auto start{std::chrono::steady_clock::now()};
    #pragma omp parallel num_threads(numThr)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
        for (int i = lb; i <= ub; i++)
        {
            vecOut[i] = 0;
            for (int j = 0; j < n; j++)
            {
                vecOut[i] += matr[i][j] * vecIn[j];
            }
        }
    }
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    return elapsed_seconds;
}

int main(int argc, char *argv[])
{
    int threads[] = {1, 2, 4, 7, 8, 16, 20, 40};
    const int M1 = 20000;
    const int M2 = 40000;

    double** matr1 = new double*[M1];
    for (int i = 0; i < M1; i++)
    {
        matr1[i] = new double[M1];
        for (int j = 0; j < M1; j++)
        {
            matr1[i][j] = 1;
        }
    }
    double* vec1 = new double[M1];
    for (int i = 0; i < M1; i++)
    {
        vec1[i] = 4;
    }

    double** matr2 = new double*[M2];
    for (int i = 0; i < M2; i++)
    {
        matr2[i] = new double[M2];
        for (int j = 0; j < M2; j++)
        {
            matr2[i][j] = 1;
        }
    }
    double* vec2 = new double[M2];
    for (int i = 0; i < M2; i++)
    {
        vec2[i] = 4;
    }

    for (auto i : threads)
    {
        std::cout << "20000*20000, " << i << " threads: " << dgemv_parallel(matr1, vec1, vec1, i, M1, M1).count() << " seconds" << std::endl;
    }

    for (auto i : threads)
    {
        std::cout << "40000*40000, " << i << " threads: " << dgemv_parallel(matr2, vec2, vec2, i, M2, M2).count() << " seconds" << std::endl;
    }
    return 0;
}