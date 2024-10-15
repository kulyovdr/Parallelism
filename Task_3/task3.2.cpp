#include <iostream>
#include <thread>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <random>
#include <mutex>
#include <functional>
#include <string>
#include <cstring>


enum class Type
{
    Sin, Sqrt, Pow
};

class Task {
public:
    Type type;
    double arg;
    int iter;
    int arg2 = 0;

    Task() : arg2(0) {}

    Task(Type typ, int iterations)
    {
        type = typ;
        iter = iterations;
    }

    Task(Type typ, double argument, int iterations, int argument2)
    {
        type = typ;
        arg = argument;
        iter = iterations;
        arg2 = argument2;
    }
};

template<typename T>
class Server {
public:
    Server() : running(true) {}

    void start()
    {
        thread_ = std::thread(&Server::run, this);
    }

    void stop()
    {
        running = false;
        thread_.join();
    }

    size_t add_task(Task task)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_[task_id_] = task;
        return task_id_++;
    }

    T request_result(size_t id)
    {
        while (true)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (results_.find(id) != results_.end())
            {
                T result = results_[id];
                results_.erase(id);
                return result;
            }
        }
    }

private:
    std::thread thread_;
    std::unordered_map<size_t, Task> tasks_;
    std::unordered_map<size_t, T> results_;
    size_t task_id_ = 0;
    std::mutex mutex_;
    bool running;

    void run()
    {
        while (running)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& task : tasks_)
            {
                T result;
                if(task.second.type == Type::Sin)
                {
                    result = std::sin(task.second.arg);
                }
                else if(task.second.type == Type::Sqrt)
                {
                    result = std::sqrt(task.second.arg);
                }
                else
                {
                    result = std::pow(task.second.arg, task.second.arg2);
                }
                results_[task.first] = result;
            }
            tasks_.clear();
        }
    }
};


void client(Server<double>& server, Task task, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "File not found" << filename << std::endl;
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 10.0);
    std::uniform_int_distribution<int> dist2(0, 10);

    for (size_t i = 0; i < task.iter; ++i)
    {
        double argument = dist(gen);
        int argument2 = 0;
        if (task.type == Type::Pow)
        {
            argument2 = dist2(gen); 
        }

        Task current_task(task.type, argument, task.iter, argument2);
        size_t task_id = server.add_task(current_task);
        double result = server.request_result(task_id);

        file << "ID: " << task_id << " arg1: " << argument;
        if (task.type == Type::Pow)
        {
            file << " arg2: " << argument2;
        }
        file << " Result: " << result << std::endl;
    }

    file.close();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Enter the number of tasks" << std::endl;
        return 1;
    }

    Task client1(Type::Sin, std::stoi(argv[1]));
    std::string file1 = "sin.txt";

    Task client2(Type::Sqrt, std::stoi(argv[1]));
    std::string file2 = "sqrt.txt";

    Task client3(Type::Pow, std::stoi(argv[1]));
    std::string file3 = "pow.txt";

    Server<double> server;
    server.start();

    std::thread client_serv1(client, std::ref(server), client1, file1);
    std::thread client_serv2(client, std::ref(server), client2, file2);
    std::thread client_serv3(client, std::ref(server), client3, file3);

    client_serv1.join();
    client_serv2.join();
    client_serv3.join();

    server.stop();

    return 0;
}