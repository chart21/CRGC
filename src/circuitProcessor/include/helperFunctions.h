#ifndef helperFunctions
#define helperFunctions

#include "randomBoolGenerator.h"
#include <iostream>
#include <random>
#include <stdlib.h> 
#include <functional>
#include <bitset>
#include <chrono>
//#define US


void generateRandomInput(size_t bitlength, bool* input);

void converIntToBoolArr(uint_fast64_t num, size_t bitlength, bool* arr);

uint_fast64_t convertBoolArrToInt(bool* arr, size_t bitlength);

bool equalBoolArr(bool* arr1, bool* arr2, size_t bitlength);

void swapLeftParent(bool (&arr)[2][2]);

void swapRightParent(bool (&arr)[2][2]);

void flipTable(bool (&arr)[2][2]);

std::chrono::_V2::system_clock::time_point startClock();

void stopClock(std::chrono::_V2::system_clock::time_point t1);

template<typename T>
void printArr(T *arr, uint_fast64_t size)
{
    for(auto i = 0; i < size; i++)
        std::cout <<  i << " : " << arr[i] << '\n';
}

bool check_number(std::string str);

template <typename F, typename... Args>
auto funcTime(std::string printText, F func, Args &&...args)
{
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
#ifdef US
    // auto time = std::chrono::duration_cast<std::chrono::microseconds>(
    //                 std::chrono::high_resolution_clock::now() - t1)
    //                 .count();
    // std::cout << "---TIMING--- " << time << "us " << printText << '\n';
#else
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - t1)
                    .count();
    std::cout << "---TIMING--- " << time << "ms " << printText << '\n';
#endif
    return time;
}

#endif