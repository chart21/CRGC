#ifndef CIRCUIT_WRITER_H__
#define CIRCUIT_WRITER_H__

#include "circuitStructs.h"

#include <iostream>
#include <fstream>

#define BUF_W 100000

void exportCircuitSeparateFiles(TransformedCircuit* circuit, std::string destinationPath);

void exportObfuscatedInput(bool* valArr, const CircuitDetails &details, std::string destinationPath="");

class Writer_base{
public:
    bool store = false;
    Writer_base(bool store=false):store(store) {}
    virtual void send_data_gen(const void * data, uint64_t nbyte)=0;
    virtual void exportBin(ShrinkedCircuit* circuit, bool* valArr)=0;
    virtual void exportCompressedCircuit( ShrinkedCircuit* cir, bool* valArr, int package=1)=0;
};

template <typename IO>
class Writer: public Writer_base {
public:
    IO *io = nullptr;
    Writer(IO *io,bool store=false):io(io),Writer_base(store) {}
    Writer() {}

    void send_data_gen(const void * data, uint64_t nbyte) override{
        uint64_t send = 0;
        while(send<nbyte){
            size_t n = nbyte-send>BUF_W? BUF_W : nbyte-send;
            io->send_data((char*)data+send,n);
            // io->flush();
            send+=n;

        }
    }
    
    void exportBin(ShrinkedCircuit* circuit, bool* valArr) override;
    
    /* export compressed circuit to HDD */
    void exportCompressedCircuit( ShrinkedCircuit* cir, bool* valArr, int package=1) override;

    // void sendThread();
};
#endif