#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstring> 


int Sinus(std::ifstream& file)
{
    int count_err = 0;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);

        std::string idStr, argStr, resultStr;
        double arg, result, id;

        if (!(iss >> idStr >> id >> argStr >> arg >> resultStr >> result))
        {
            std::cerr << "Parse error: " << line << std::endl;
        }

        double sinArg = std::sin(arg);
        
        if (std::abs(sinArg - result) > 1e-5)
        {
            std::cout << "ID: "<< id << " arg1 = " << arg << ", sin(arg1) = " << sinArg << ", Result = " << result << std::endl;
            count_err++;
        }
    }
    file.close();
    return count_err;
}


int Sqrt(std::ifstream& file)
{
    int count_err = 0;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);

        std::string idStr, argStr, resultStr;
        double arg, result, id;

        if (!(iss >> idStr >> id >> argStr >> arg >> resultStr >> result))
        {
            std::cerr << "Parse error: " << line << std::endl;
        }

        double sqrtArg = std::sqrt(arg);
        
        if (std::abs(sqrtArg - result) > 1e-3)
        {
            std::cout << "ID: "<< id << " arg1 = " << arg << ", sin(arg1) = " << sqrtArg << ", Result = " << result << std::endl;
            count_err++;
        }
    }
    file.close();
    return count_err;
}


int Pow(std::ifstream& file)
{
    int count_err = 0;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);

        std::string idStr, argStr1, argStr2, resultStr;
        double arg1, arg2, result, id;

        if (!(iss >> idStr >> id >> argStr1 >> arg1 >> argStr2 >> arg2 >> resultStr >> result))
        {
            std::cerr << "Parse error: " << line << std::endl;
        }

        double powArg = std::pow(arg1, arg2);
        
        if (std::abs(powArg - result) > 1e-3)
        {
            std::cout << "ID: "<< id << " arg1 = " << arg1 << " arg2 = " << arg2 << ", arg1^arg2 = " << powArg << ", Result = " << result << std::endl;
            count_err++;
        }
    }
    file.close();
    return count_err;
}


int main(int argc, char *argv[])
{
    std::ifstream sin_file("sin.txt");
    if (!sin_file.is_open())
    {
        std::cerr << "sin.txt not found" << std::endl;
        return 1;
    }

    std::ifstream pow_file("pow.txt");
    if (!pow_file.is_open())
    {
        std::cerr << "pow.txt not found" << std::endl;
        return 1;
    }

    std::ifstream sqrt_file("sqrt.txt");
    if (!sqrt_file.is_open())
    {
        std::cerr << "sqrt.txt not found" << std::endl;
        return 1;
    }

    int sin_errors = Sinus(sin_file);
    int sqrt_errors = Sqrt(sqrt_file);
    int pow_errors = Pow(pow_file);

    std::cout << "Number of errors: sin = " << sin_errors << " sqrt = " << sqrt_errors << " pow = " << pow_errors << std::endl;

    return 0;
}