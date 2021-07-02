#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include "helperFunctions.h"
void obfuscateInput(bool* inputA, bool* obfuscatedValArr, bool* flipped,  CircuitDetails details);
void getFlippedCircuitWithoutOutputsN(TransformedCircuit* circuit, bool* flipped);

void getFlippedCircuitWithoutOutputsMT(TransformedCircuit* circuit, bool* flipped, uint_fast64_t numThreads, uint_fast64_t timeSleep);
void getFlippedCircuitWithoutOutputsThread(TransformedCircuit* circuit, bool* flipped, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated, uint_fast64_t timeSleep);
