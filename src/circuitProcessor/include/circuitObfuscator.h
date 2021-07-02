#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include "helperFunctions.h"
#include <unordered_map>

void moreEfficientObfuscation(TransformedCircuit* circuit, bool* inputA);
void moreEfficientObfuscationArr(TransformedCircuit* circuit, bool* inputA, bool* isObfuscated);

void moreEfficientObfuscationMT(TransformedCircuit* circuit, bool* inputA, bool* isObfuscated, uint_fast64_t numThreads, uint_fast64_t timeSleep);
void moreEfficientObfuscationThread(TransformedCircuit* circuit, bool* inputA, bool* isObfuscated, bool* unobfuscatedValues, bool* evaluated, uint_fast64_t id, uint_fast64_t numThreads, uint_fast64_t timeSleep);