#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include <queue>

void getIntegrityBreakingGates(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors);
void breakIntegrityOfGates(TransformedCircuit* circuit, bool* isObfuscated);

void getIntegrityBreakingGatesfromOutput(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents);
void getIntegrityBreakingGatesFromOutputThread(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, bool* notobfuscated, bool* addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread);
void getIntegrityBreakingGatesfromOutputMT(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, uint_fast64_t numThreads);


void breakIntegrityOfGatesMT(TransformedCircuit* circuit, bool* isObfuscated, uint_fast64_t id, uint_fast64_t numThreads);
void breakIntegrityOfGatesThread(TransformedCircuit* circuit, bool* isObfuscated, uint_fast64_t id, uint_fast64_t numThreads);
