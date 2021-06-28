#include "include/helperFunctions.h"
#include "include/randomBoolGenerator.h"
#include <iostream>
#include <random>
#include <stdlib.h> 
#include <functional>
#include <bitset>
#include <chrono>


void generateRandomInput(size_t bitlength, bool* input)
{
    //bool *input = new bool[bitlength];
    sfc64 sfc;
    RandomizerWithSentinelShift<> randomizer;    
    for (int i = 0; i < bitlength; i++)
    {
        input[i] = randomizer(sfc);
    }
}




void converIntToBoolArr(uint_fast64_t num, size_t bitlength, bool* arr)
{
//bool *arr = new bool[bitlength];
for (auto i = 0; i < bitlength; i++)
{
  arr[bitlength-i-1] = num & 1;
  num /= 2;
}
}

uint_fast64_t convertBoolArrToInt(bool* arr, size_t bitlength)
{
    uint_fast64_t num = 0;
    uint_fast64_t tmp;
    for (auto i = 0; i < bitlength; i++)
    {
    tmp = arr[i];
    num |= tmp << (bitlength - i - 1);    
    }
    return num;
}

void swapLeftParent(bool (&arr)[2][2])
{
    auto temp = arr[0][0];
    arr[0][0] = arr[1][0];
    arr[1][0] = temp;

    temp = arr[0][1];
    arr[0][1] = arr[1][1];
    arr[1][1] = temp;

}

void swapRightParent(bool (&arr)[2][2])
{
    auto temp = arr[0][0];
    arr[0][0] = arr[0][1];
    arr[0][1] = temp;

    temp = arr[1][0];
    arr[1][0] = arr[1][1];
    arr[1][1] = temp;

}

void flipTable(bool (&arr)[2][2])
{
    arr[0][0] = !arr[0][0];
    arr[0][1] = !arr[0][1];
    arr[1][0] = !arr[1][0];
    arr[1][1] = !arr[1][1];
}


std::chrono::_V2::system_clock::time_point startClock()
{
    return std::chrono::high_resolution_clock::now();
}

void stopClock(std::chrono::_V2::system_clock::time_point t1)
{
    auto t2 = std::chrono::high_resolution_clock::now();    

    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

    std::cout << duration1<<"\n";    
}






// bool* generateRandomInputOLD(size_t bitlength)
// {
//     auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());
//     //sfc64 sfc;
//     //RandomizerWithSentinelShift<> randomizer;

//     bool *input = new bool[bitlength];

//     for (int i = 0; i < bitlength; i++)
//     {
//         input[i] = gen();
//         //input[i] = randomizer(sfc);
//     }
//     return input;
// }