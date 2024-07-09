#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <memory>


void dgemvStep(std::shared_ptr<double[]> matr, std::shared_ptr<double[]> vecIn, std::shared_ptr<double[]> vecOut, int nthreads, const int m, const int n, int threadid)
{
    int items_per_thread = m / nthreads;
    int lb = threadid * items_per_thread;
    int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
    for (int i = lb; i <= ub; i++)
    {
        vecOut[i] = 0;
        for (int j = 0; j < n; j++)
        {
            vecOut[i] += matr[i * n + j] * vecIn[j];
        }
    }
}

const std::chrono::duration<double> dgemvParallel(std::shared_ptr<double[]> matr, std::shared_ptr<double[]> vecIn, std::shared_ptr<double[]> vecOut, int numThr, const int m, const int n)
{
    const auto start{std::chrono::steady_clock::now()};

    std::unique_ptr<std::thread[]> threads(new std::thread[numThr]);
    for (int threadid = 0; threadid < numThr; threadid++)
    {
        threads[threadid] = std::thread(dgemvStep, matr, vecIn, vecOut, numThr, m, n, threadid);
    }

    for (int threadid = 0; threadid < numThr; threadid++)
    {
        threads[threadid].join();
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

    std::shared_ptr<double[]> matr1(new double[M1 * M1]);
    for (int i = 0; i < M1; i++)
    {
        for (int j = 0; j < M1; j++)
        {
            matr1[i * M1 + j] = (double)1;
        }
    }
    std::shared_ptr<double[]> vec1(new double[M1]);
    for (int i = 0; i < M1; i++)
    {
        vec1[i] = (double)4;
    }

    std::shared_ptr<double[]> matr2(new double[M2 * M2]);
    for (int i = 0; i < M2; i++)
    {
        for (int j = 0; j < M2; j++)
        {
            matr2[i * M2 + j] = (double)1;
        }
    }
    std::shared_ptr<double[]> vec2(new double[M2]);
    for (int i = 0; i < M2; i++)
    {
        vec2[i] = (double)4;
    }

    for (auto i : threads)
    {
        std::cout << "20000*20000, " << i << " threads: " << dgemvParallel(matr1, vec1, vec1, i, M1, M1).count() << " seconds" << std::endl;
    }

    for (auto i : threads)
    {
        std::cout << "40000*40000, " << i << " threads: " << dgemvParallel(matr2, vec2, vec2, i, M2, M2).count() << " seconds" << std::endl;
    }
    return 0;
}
