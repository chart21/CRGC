#include "circuitStructs.h"
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

//#include <condition_variable>



#include <iostream>
void evaluateTransformedCircuitN(TransformedCircuit *circuit, bool* inputA, bool* inputB, bool* output);
void evaluateTransformedCircuitbyLevel(TransformedCircuit *circuit, std::vector<TransformedGate>* circuitByLine, uint_fast64_t maxLevel,  bool* inputA, bool* inputB, bool* output);


void evaluateTransformedCircuitbyLevelMT(TransformedCircuit *circuit, std::vector<TransformedGate>* circuitByLine, uint_fast64_t maxLevel,  bool* inputA, bool* inputB, bool* output);
void evaluateTransformedCircuitbyLevelThread(std::vector<TransformedGate>* circuitByLine, bool* evaluation, uint_fast64_t s, uint_fast64_t index, uint_fast64_t numGates);

void evaluateTransformedCircuitHackMp(TransformedCircuit *circuit, bool* inputA, bool* inputB, bool* output);
void evaluateTransformedCircuitbyLevelThreadHack(TransformedCircuit *circuit, bool* evaluation, uint_fast64_t id, uint_fast64_t numThreads);

void evaluateTransformedCircuitHackMpUint(TransformedCircuit *circuit, bool* inputA, bool* inputB, bool* output);
void evaluateTransformedCircuitbyLevelThreadHackUint(TransformedCircuit *circuit, uint_fast8_t* evaluation, uint_fast64_t id, uint_fast64_t numThreads);

void evaluateTransformedCircuitHackMpBool(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output);
void evaluateTransformedCircuitbyLevelThreadHackBool(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated);
void evaluateTransformedCircuitbyLevelThreadHackBool2(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated);

void evaluateTransformedCircuitHackMpCond(TransformedCircuit *circuit, bool* inputA, bool* inputB, bool* output);
void evaluateTransformedCircuitbyLevelThreadHackCond(TransformedCircuit *circuit, bool* evaluation, uint_fast64_t id, uint_fast64_t numThreads, std::binary_semaphore** semaphores, bool* aquired);