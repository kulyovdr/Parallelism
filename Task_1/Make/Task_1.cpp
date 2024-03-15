#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <iomanip>
#include <memory>
#define LEN 10000000

#if FL == 1
using my_type = float;
#else
using my_type = double;
#endif

my_type sin_array(std::shared_ptr<my_type[]> arr)
{
    my_type sum = 0;
    for (int i = 0; i < LEN; i++)
    {
<<<<<<< Updated upstream
        arr[i] = sinus((2 * M_PI) * ((my_type)i / LEN));
=======
        arr[i] = sin((2 * M_PI) * (i / LEN));
>>>>>>> Stashed changes
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char* argv[])
{
<<<<<<< Updated upstream
    std::shared_ptr<my_type[]> arr(new my_type[LEN]);
    std::cout << std::fixed << std::setprecision(30) << sin_array(arr) << std::endl;
=======
    my_type* arr = new my_type[LEN];
    cout << fixed << setprecision(30) << sin_array(arr) << endl;
    delete[] arr;
>>>>>>> Stashed changes
    return 0;
}