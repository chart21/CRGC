#ifndef CIRCUIT_WRITER_H__
#define CIRCUIT_WRITER_H__

#include "circuitStructs.h"

#include <iostream>
#include <fstream>

#define BUF_W 100000

void exportCircuitSeparateFiles(TransformedCircuit* circuit, std::string destinationPath);

void exportObfuscatedInput(bool* valArr, const CircuitDetails &details, std::string destinationPath="");

template <typename IO>
class Gen {
public:
    IO *io = nullptr;

    Gen(IO *io):io(io) {}
    Gen() {}

    void send_data_gen(const void * data, uint64_t nbyte){
        uint64_t send = 0;
        while(send<nbyte){
            size_t n = nbyte-send>BUF_W? BUF_W : nbyte-send;
            io->send_data((char*)data+send,n);
            // io->flush();
            send+=n;

        }
    }
    
    void exportBin(ShrinkedCircuit* circuit, bool* valArr);
    
    /* export compressed circuit to HDD */
    void exportCompressedCircuit( ShrinkedCircuit* cir, bool* valArr, int thr_enc=1);
};
#endif