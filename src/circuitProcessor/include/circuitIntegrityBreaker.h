#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include <queue>

void getIntegrityBreakingGatesN4(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors);
void breakIntegrityOfGates(TransformedCircuit* circuit, bool* isObfuscated);