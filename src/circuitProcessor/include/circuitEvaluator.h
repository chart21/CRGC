#include "circuitStructs.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>


//#include <condition_variable>



#include <iostream>
void evaluateTransformedCircuit(TransformedCircuit *circuit, bool* inputA, bool* inputB, bool* output);

void evaluateSortedTransformedCircuit(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output);

void evaluateTransformedCircuitHackMT(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output, uint_fast64_t numThreads);
void evaluateTransformedCircuitHackThread(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads);
