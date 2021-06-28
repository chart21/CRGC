#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include "helperFunctions.h"
void obfuscateInput(bool* inputA, bool* obfuscatedValArr, bool* flipped,  CircuitDetails details);
void getFlippedCircuitWithoutOutputsN(TransformedCircuit* circuit, bool* flipped);