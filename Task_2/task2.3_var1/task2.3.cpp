#include <iostream>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <vector>
#include <numeric>

int numThr = 1;
const int n = 1000;
const int tau = 0.01;
const int eps = 0.00001;

//Сам алгоритм решения задачи по методу простой итерации
const std::chrono::duration<double> itera(std::vector<double> A, std::vector<double> B, std::vector<double> &X)
{
    const auto start{std::chrono::steady_clock::now()};
    double criter = 99999;
    double AXi = 0; //A[i] * X[i]
    std::vector<double> err(numThr);
    std::fill(err.begin(), err.end(), 0);
    std::vector<double> err_dnm(numThr);
    std::fill(err_dnm.begin(), err_dnm.end(), 0);
    int MAX_ITER = 0;
    while (criter >= eps && MAX_ITER != 3000)
    {
        #pragma omp parallel num_threads(numThr) private(AXi)
        {
            int threadid = omp_get_thread_num();
            int items_per_thread = n / numThr;
            int lb = threadid * items_per_thread;
            int ub = (threadid == numThr - 1) ? (numThr - 1) : (lb + items_per_thread - 1);
            for (int i = lb; i <= ub; i++)
            {
                AXi = 0;
                for (int j = 0; j < n; j++)
                {
                    AXi += A[i * n + j] * X[i];
                }
                double AXMinusB = AXi - B[i];
                X[i] = X[i] - tau * AXMinusB;
                err[i] = AXMinusB * AXMinusB;
                err_dnm[i] = B[i] * B[i];
            }
        }
        criter = std::reduce(err.begin(), err.end(), 0.0) / std::reduce(err_dnm.begin(), err_dnm.end(), 0.0);
        //std::cout << criter << std::endl;
        MAX_ITER++;
    }
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    return elapsed_seconds;
}

int main(int argc, char *argv[])
{
    int threads[] = {1, 2, 4, 8, 16, 20, 40, 80};

    //Заполнение матрицы A
    std::vector<double> A(n * n);
    std::fill(A.begin(), A.end(), 1);
    for (int i = 0; i < n; i++)
    {
        A[i * n + i] = 2;
    }

    //Заполнение вектора B
    std::vector<double> B(n);
    std::fill(B.begin(), B.end(), n + 1);

    //Инициализация вектора X
    std::vector<double> X(n);

    for (auto i : threads)
    {
        numThr = i;
        std::fill(X.begin(), X.end(), 0.0);
        const std::chrono::duration<double> time = itera(A, B, X);
        // for (auto j : X)
        // {
        //     std::cout << j << " ";
        // }
        // std::cout << std::endl;
        std::cout << i << " threads: " << time.count() << " seconds" << std::endl;
    }

    return 0;
}