#ifndef CIRCUIT_TRANSFORMER_H__
#define CIRCUIT_TRANSFORMER_H__

#include "circuitStructs.h"
#include "helperFunctions.h"

UintCircuit* transformTransformedCircuitToUint(TransformedCircuit* circuit);
void transformBristolCircuitToTransformedCircuit(BristolCircuit *bristolCircuit, TransformedCircuit *circuit, bool* flipped);
TransformedCircuit* transformBristolCircuitToTransformedCircuitN(BristolCircuit *bristolCircuit);
void deleteRevealGates(TransformedCircuit *circuit, uint_fast64_t* circuitLineOfWireIndex);
ShrinkedCircuit* transformCircuitToShrinkedCircuit(TransformedCircuit* circuit);

#endif
