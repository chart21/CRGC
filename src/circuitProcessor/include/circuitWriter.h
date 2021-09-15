#ifndef CIRCUIT_WRITER_H__
#define CIRCUIT_WRITER_H__

#include "circuitStructs.h"

#include <iostream>
#include <fstream>

void exportCircuitSeparateFiles(TransformedCircuit* circuit, std::string destinationPath);

template <typename IO>
class Gen {
public:
    IO *io = nullptr;
    Gen(IO *io):io(io) {}
    Gen() {}
    
    void exportObfuscatedInput(bool* valArr, CircuitDetails details, std::string destinationPath);
    void exportBin(ShrinkedCircuit* circuit, std::string destinationPath);
    
    /* export compressed circuit to HDD */
    void exportCompressedCircuit( ShrinkedCircuit* cir, int thr_enc=1);
};
#endif