#include "circuitStructs.h"
#include <vector>
#include <queue>
#include <thread>

void getPotentiallyObfuscatedGates(BristolCircuit* circuit, bool* po);
void getPotentiallyIntegrityBreakingGates(CircuitDetails details, bool* po, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors);
void getPotentiallyIntegrityBreakingGatesFromOutput(CircuitDetails details, bool* po, uint_fast64_t* parents);

void getPotentiallyIntegrityBreakingGatesFromOutputThread(CircuitDetails details, bool* po, uint_fast64_t* parents, bool* npib, bool* addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread);
void getPotentiallyIntegrityBreakingGatesFromOutputMT(CircuitDetails details, bool* po, uint_fast64_t* parents, uint_fast64_t numThreads);

void getPotentiallyObfuscatedGatesMT(BristolCircuit* circuit, bool* po, uint_fast64_t numThreads, uint_fast64_t sleepTime);
void getPotentiallyObfuscatedGatesThread(BristolCircuit* circuit, bool* po, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated, uint_fast64_t sleepTime);
