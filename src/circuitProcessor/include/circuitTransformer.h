#include "circuitStructs.h"
#include "helperFunctions.h"

UintCircuit* transformTransformedCircuitToUint(TransformedCircuit* circuit);
void transformBristolCircuitToTransformedCircuit(BristolCircuit *bristolCircuit, TransformedCircuit *circuit, bool* flipped);
TransformedCircuit* transformBristolCircuitToTransformedCircuitN(BristolCircuit *bristolCircuit);