#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include <queue>

void getIntermediaryGates(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors);
void regenerateGates(TransformedCircuit* circuit, bool* isObfuscated);

void getIntermediaryGatesfromOutput(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents);
void getIntermediaryGatesFromOutputThread(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, bool* notobfuscated, bool* addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread);
void getIntermediaryGatesfromOutputMT(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, uint_fast64_t numThreads);


void regenerateGatesMT(TransformedCircuit* circuit, bool* isObfuscated, uint_fast64_t numThreads);
void regenerateGatesThread(TransformedCircuit* circuit, bool* isObfuscated, uint_fast64_t id, uint_fast64_t numThreads);

void getIntermediaryGatesFromOutputThread2(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, bool* notobfuscated, bool* addedGates, uint_fast64_t id, uint_fast64_t numThreads);
void getIntermediaryGatesfromOutputMT2(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, uint_fast64_t numThreads);