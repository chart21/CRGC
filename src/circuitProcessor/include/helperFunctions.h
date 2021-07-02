#ifndef helperFunctions
#define helperFunctions
#include "helperFunctions.h"
#include "randomBoolGenerator.h"
#include <iostream>
#include <random>
#include <stdlib.h> 
#include <functional>
#include <bitset>
#include <chrono>


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

#endif