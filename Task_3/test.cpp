#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>

// Очередь задач
std::queue<std::packaged_task<int()>> tasks;

// задача
int f(int x, int y)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return std::pow(x, y);
}

// пример сервера обрабатывающего задачи из очереди
int server_thread(std::stop_token stoken)
{
    std::packaged_task<int()> task;
    // пока не получили сигнал стоп
    while (!stoken.stop_requested())
    {
        // если очередь не пуста, то достаем задачу и решаем
        if(!tasks.empty())
        {
            task = std::move(tasks.front());
            task();
            tasks.pop();
        }
        // спим, чтобы не занимать одно ядро целиком
        // можно использовать std::condition_variable
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Server stop!\n";
    
}

// Пример потока добавляющего задачи
// Обратите внимание на метод emplace(), для конструирования на месте
void add_task_thread()
{
    // формируем задачу
    std::packaged_task<int()> task(std::bind(f, 2, 4));
    // получаем future
    std::future<int> result = task.get_future();
    
    // добавляем задачу в очередь
    tasks.push(std::move(task));

    //выводим результат задачи
    std::cout << "task_thread:\t" << result.get() << '\n';
}

int main()
{
    std::cout << "Start\n";

    std::jthread server(server_thread);
    std::thread add_task(add_task_thread);

    add_task.join();
    server.request_stop();
    server.join();
    std::cout << "End\n";
    
}