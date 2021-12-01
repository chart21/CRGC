#include "circuitStructs.h"
#include <vector>
#include <queue>
#include <thread>

void getPotentiallyObfuscatedFixedGates(BristolCircuit* circuit, bool* po);
void getPotentiallyObfuscatedFixedGatesT(TransformedCircuit* circuit, bool* po);

void getPotentiallyIntermediaryGates(CircuitDetails details, bool* po, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors);
void getPotentiallyIntermediaryGatesFromOutput(CircuitDetails details, bool* po, uint_fast64_t* parents);

void getPotentiallyIntermediaryGatesFromOutputThread(CircuitDetails details, bool* po, uint_fast64_t* parents, bool* npib, bool* addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread);
void getPotentiallyIntermediaryGatesFromOutputMT(CircuitDetails details, bool* po, uint_fast64_t* parents, uint_fast64_t numThreads);


void getPotentiallyIntermediaryGatesFromOutputThread2(CircuitDetails details, bool *po, uint_fast64_t *parents, bool *npib, bool *addedGates, uint_fast64_t id, uint_fast64_t numThreads);
void getPotentiallyIntermediaryGatesFromOutputMT2(CircuitDetails details, bool *po, uint_fast64_t *parents, uint_fast64_t numThreads);













void getCircuitLineofWireIndex(TransformedCircuit* circuit, uint_fast64_t* circuitLineOfWireIndex);

void getLeakedInputsFromOutputSorted(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs);
void getLeakedInputsFromOutputUnsorted(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t* circuitLineOfWireIndex);

void getLeakedInputsFromOutputSortedMT(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t numThreads);
void getLeakedInputsFromOutputUnsortedMT(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t* circuitLineOfWireIndex, uint_fast64_t numThreads);
void getLeakedInputsFromOutputSortedThread(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, bool *addedGates[2], uint_fast64_t id, uint_fast64_t numThreads);
void getLeakedInputsFromOutputUnsortedThread(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t* circuitLineOfWireIndex, bool *addedGates[2], uint_fast64_t id, uint_fast64_t numThreads);
