#include <iostream>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <vector>
#include <numeric>

int numThr = 1;
const double n = 2000;
const double tau = 0.01;
const double eps = 0.000001;

//Сам алгоритм решения задачи по методу простой итерации
const std::chrono::duration<double> itera(std::vector<double>& A, std::vector<double>& B, std::vector<double>& X)
{
    const auto start{std::chrono::steady_clock::now()};
    double criter = 0;

    double AXi = 0; //A[i] * X[i]
    double err = 0;
    double err_dnm = 0;
    double AXMinusB = 0;

    do
    {
        double err1 = 0;
        double err_dnm1 = 0;
        #pragma omp parallel num_threads(numThr) private(AXi, err, err_dnm, AXMinusB)
        {
            int nthreads = omp_get_num_threads();
            int threadid = omp_get_thread_num();
            int items_per_thread = n / nthreads;
            int lb = threadid * items_per_thread;
            int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

            for (int i = lb; i <= ub; i++)
            {
                AXi = 0;
                for (int j = 0; j < n; j++)
                {
                    AXi += A[i * n + j] * X[i];
                }
                AXMinusB = AXi - B[i];
                X[i] = X[i] - tau * AXMinusB;

                err1 += AXMinusB * AXMinusB;
                err_dnm1 += B[i] * B[i];
            }
            #pragma omp barrier
            #pragma omp atomic
            err1 += err;
            #pragma omp atomic
            err_dnm1 += err_dnm;
        }
        criter = err1 / err_dnm1;
    } while (criter > eps);
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
        for (int j = 0; j < 5; j++)
        {
            std::fill(X.begin(), X.end(), 0);
            const std::chrono::duration<double> time = itera(A, B, X);

            std::cout << i << " threads: " << time.count() << " seconds" << std::endl;

            bool rrr = true;
            for (auto j : X)
            {
                if (j < 0.9999 || j > 1.00001)
                {
                    std::cout << "false" << std::endl;
                    break;
                }
                //std::cout << j << std::endl;
            }
        }
    }

    return 0;
}