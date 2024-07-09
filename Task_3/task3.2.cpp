#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <fstream>
#include <random>
//#include <functional>
#include <future>
#include <vector>

enum class Type
{
    Sin, Sqrt, Pow
};

// Task class to store task information
class Task {
public:
    int id;
    double arg;
    double result;
    std::packaged_task<double(double)> task;
};

// Server class template
class Server {
private:
    std::queue<Task> taskQueue;
    std::vector<Task> results;
    std::mutex mut;
    std::condition_variable cv;
    std::thread serverThread;

    bool isRunning = true;

    void mainLoop()
    {
        while (isRunning)
        {
            Task task;
            
            std::unique_lock<std::mutex> u_lock{mut, std::defer_lock};
            cv.wait(u_lock, [&] { return !taskQueue.empty() || !isRunning; });

            u_lock.lock();
            task = std::move(taskQueue.front());
            taskQueue.pop();
            u_lock.unlock();

            std::future<double> res = task.task.get_future();
            task.task(task.arg);
            task.result = res.get();

            u_lock.lock();
            results.push_back(task);
            u_lock.unlock();
        }
    }

public:
    void start()
    {
        serverThread = std::thread(&Server::mainLoop, this); 
    }

    void stop()
    {
        isRunning = false;
        cv.notify_all();
        serverThread.join();
    }

    size_t add_task(Task task)
    {
        std::unique_lock<std::mutex> u_lock(mut);
        task.id = results.size();
        taskQueue.push(task);
        cv.notify_one();
        return task.id;
    }

    Task request_result(int id_res)
    {
        std::unique_lock<std::mutex> u_lock(mut);
        cv.wait(u_lock, [&] { return results.size() > id_res; });
        return std::move(results[id_res]);
    }
};

// Client function to add tasks to server
void client(Server& server, int numTasks, Type type, std::ofstream file) {
    std::default_random_engine re;
    std::uniform_real_distribution<double> dist(1, 100);

    for (int i = 0; i < numTasks; ++i)
    {
        Task task;
        int id = 0;
        Task result;
        switch (type)
        {
            case Type::Sin:
                task.task = std::packaged_task<double(double)>([](double x) {return std::sin(x);});
                task.arg = dist(re);
                id = server.add_task(std::move(task));
                result = server.request_result(id);
                file << "sin(" << result.arg << ") = " << result.result << std::endl;
                break;
            case Type::Sqrt:
                task.task = std::packaged_task<double(double)>([](double x) {return std::sqrt(x);});
                task.arg = dist(re);
                id = server.add_task(std::move(task));
                result = server.request_result(id);
                file << "sqrt(" << result.arg << ") = " << result.result << std::endl;
                break;
            case Type::Pow:
                task.task = std::packaged_task<double(double)>([](double x) {return std::pow(x, 2);});
                task.arg = dist(re);
                id = server.add_task(std::move(task));
                result = server.request_result(id);
                file << result.arg << "^2 = " << result.result << std::endl;
                break;
        }
    }
}

int main() {
    Server server;
    server.start();

    int countTasks = 200;

    std::ofstream file1("sin_results.txt");
    std::ofstream file2("sqrt_results.txt");
    std::ofstream file3("pow_results.txt");

    std::thread client1(client, std::ref(server), countTasks, Type::Sin, file1);
    std::thread client2(client, std::ref(server), countTasks, Type::Sqrt, file2);
    std::thread client3(client, std::ref(server), countTasks, Type::Pow, file3);

    client1.join();
    client2.join();
    client3.join();

    file1.close();
    file2.close();
    file3.close();

    server.stop();
    return 0;
}