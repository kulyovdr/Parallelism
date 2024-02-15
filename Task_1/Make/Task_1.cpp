#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <iomanip>
#define LEN 10000000

#if FL == 1
using my_type = float;
#define sinus(x) sinf(x)
#else
using my_type = double;
#define sinus(x) sin(x)
#endif

using namespace std;

my_type sin_array(my_type* arr)
{
    my_type sum = 0;
    for (int i = 0; i < LEN; i++)
    {
        arr[i] = sinus((2 * M_PI) * (i / LEN));
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char* argv[])
{
    my_type* arr = new my_type[LEN];
    cout << fixed << setprecision(30) << sin_array(arr) << endl;
    return 0;
}