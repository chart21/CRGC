#include "circuitStructs.h"
#include "randomBoolGenerator.h"
#include "helperFunctions.h"
#include <unordered_map>

void indentifyFixedGates(TransformedCircuit* circuit, bool* inputA);
void indentifyFixedGatesArr(TransformedCircuit* circuit, bool* inputA, bool* isObfuscated);
