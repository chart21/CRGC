#include "circuitStructs.h"
#include <iostream>
#include <fstream>

void exportCircuitSeparateFiles(TransformedCircuit* circuit, std::string destinationPath);
void exportObfuscatedInput(bool* valArr, CircuitDetails details, std::string destinationPath);