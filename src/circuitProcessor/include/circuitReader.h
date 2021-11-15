#ifndef CIRCUIT_READER_H__
#define CIRCUIT_READER_H__

#include "circuitStructs.h"
#include <fstream>
#include<vector>
#define BUF_R 100000

void splitString(std::string s, std::vector<std::string> &v);	

/* no need to tranfer plain circuit through io, preprocess of generator */
void importBinaryInput(std::string filepath, uint_fast64_t bitlength, bool *valArr);
CircuitDetails importBristolCircuitDetails(std::string filepath, std::string circuitFormat="bristol");
TransformedCircuit* importBristolCircuitExNot(std::string filepath, CircuitDetails details);

BristolCircuit* importBristolCircuitExNotForLeakagePrediction(std::string filepath, CircuitDetails details, bool* flipped);

BristolCircuit* importBristolCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate>* gateVec, CircuitDetails details, bool* flipped);

TransformedCircuit* importTransformedCircuit(std::string filepath, CircuitDetails details);

void importObfuscatedInput(bool* &valArr, const CircuitDetails &details, std::string destinationPath="");


template <typename IO>
class Eva{ public:
    IO *io = nullptr;
    Eva(IO *io):io(io) {}
    Eva() {}

    void recv_data_eva(void * data, uint64_t nbyte){
        size_t send = 0;
        while(send<nbyte){
            size_t n = nbyte-send>BUF_R? BUF_R : nbyte-send;
            io->recv_data((char*)data+send,n);
            // io->flush();
            send+=n;
            
        }
    }
    
    
    void importBin(ShrinkedCircuit* &circuit, bool* &valArr);
    TransformedCircuit *importTransformedCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate> *gateVec, CircuitDetails details);

    /* import compressed Circuit */
    void importCompressedCircuit(ShrinkedCircuit* &circuit, bool* &valArr);
    // void recvThread(size_t package, CircuitDetails details);

};

#endif