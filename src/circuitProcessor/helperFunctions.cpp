#include "include/helperFunctions.h"
#include <iostream>
#include <random>
#include <functional>
#include <bitset>

bool* generateRandomInput(size_t bitlength)
{
    auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());
    bool *input = new bool[bitlength];

    for (int i = 0; i < bitlength; i++)
    {
        input[i] = gen();
    }
    return input;
}


bool* converIntToBoolArr(int num, size_t bitlength)
{
bool *arr = new bool[bitlength];
for (int i = 0; i < bitlength; i++)
{
  arr[i] = num & 1;
  num /= 2;
}
return arr;
}

int convertBoolArrToInt(bool* arr, size_t bitlength)
{
    int num = 0;
    int tmp;
    for (int i = 0; i < bitlength; i++)
    {
    tmp = arr[i];
    num |= tmp << (bitlength - i - 1);    
    }
    return num;
}