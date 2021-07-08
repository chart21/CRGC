#include "circuitStructs.h"
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>
#include <chrono>


//#include <condition_variable>



#include <iostream>
void evaluateTransformedCircuit(TransformedCircuit *circuit, bool* inputA, bool* inputB, bool* output);

void evaluateTransformedCircuitHackMT(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output, uint_fast64_t numThreads);
void evaluateTransformedCircuitHackThread(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads);

void evaluateTransformedCircuitMT(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output, uint_fast64_t numThreads, uint_fast64_t timeSleep);
void evaluateTransformedCircuitThread(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated, uint_fast64_t timeSleep);




void evaluateTransformedCircuitThreadHackBool2(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated);
void evaluateTransformedCircuitbyLevelThreadHackUint(TransformedCircuit *circuit, uint_fast8_t *evaluation, uint_fast64_t id, uint_fast64_t numThreads);
void evaluateTransformedCircuitHackMTCond(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output);
void evaluateTransformedCircuitbyLevelThreadHackCond(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, std::binary_semaphore **semaphores, bool* aquired);
void evaluateTransformedCircuitbyLevel(TransformedCircuit *circuit, std::vector<TransformedGate> *circuitByLine, uint_fast64_t maxLevel, bool *inputA, bool *inputB, bool *output);
void evaluateTransformedCircuitbyLevelMT(TransformedCircuit *circuit, std::vector<TransformedGate> *circuitByLine, uint_fast64_t maxLevel, bool *inputA, bool *inputB, bool *output);
void evaluateTransformedCircuitbyLevelThread(std::vector<TransformedGate> *circuitByLine, bool *evaluation, uint_fast64_t s, uint_fast64_t index, uint_fast64_t numGates);
void evaluateTransformedCircuitHackMTUint(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output);


void evaluateUintCircuit(UintCircuit *circuit, bool *inputA, bool *inputB, bool *output);
