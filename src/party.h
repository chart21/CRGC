#ifndef PARTY_H__
#define PARTY_H__

#include <cstdint>
#include <iostream>
#include <getopt.h>
#include <vector>
#include <limits>
#include <cstddef>
#include <chrono>

#include <emp-tool/emp-tool.h>

#include "config.h"
#include "circuitProcessor/include/circuitStructs.h"
#include "circuitProcessor/include/circuitReader.h"
#include "circuitProcessor/include/circuitWriter.h"
#include "circuitProcessor/include/circuitEvaluator.h"
#include "circuitProcessor/include/helperFunctions.h"
#include "circuitProcessor/include/circuitFlipper.h"
#include "circuitProcessor/include/leakagePredictor.h"
#include "circuitProcessor/include/circuitObfuscator.h"
#include "circuitProcessor/include/circuitDetails.h"
#include "circuitProcessor/include/circuitIntegrityBreaker.h"
#include "circuitProcessor/include/circuitTransformer.h"
#include "circuitProcessor/include/circuitNetIO.h"

#include "programs/include/examplePrograms.h"
#include "programs/include/circuitLinker.h"

#include "circuitProcessor/circuitReader.cpp"
#include "circuitProcessor/circuitWriter.cpp"
#include "circuitProcessor/circuitCompressor.cpp"

struct CircuitData {
    TransformedCircuit *circuit = nullptr;
    ShrinkedCircuit *scir = nullptr;
    bool *inputA = nullptr;
    bool *inputB = nullptr;
    bool *output = nullptr;
    bool *obfuscatedValArr = nullptr;
    ~CircuitData(){
        if(scir) delete scir;
        if(circuit) delete circuit;
        if(inputA) delete [] inputA;
        if(inputB) delete [] inputB;
        if(output) delete [] output;
        if(obfuscatedValArr) delete [] obfuscatedValArr;
    }
};
struct Party{
    std::string fileFormat = FILE_FORMAT;
    std::string circuitFormat = CIRCUIT_FORMAT;
    std::string circuitName = CIRCUIT_NAME;
    std::string network = NETWORK;
    std::string store = STORE;
    int compressThreads = COMRPRESS_THREADS;   
    
    int port = PORT;

    CircuitData circuitData; //could be static

    Reader_base *reader;
    Writer_base *writer;

    void writeCircuit(string format, string print, int thread);
    void readCircuit(string format, string print);
};

void forwardExportFunctions(Party *party, ShrinkedCircuit* &scir, bool* &valArr, bool bin, int thread){
    if(bin)
        party->writer->exportBin(scir, valArr);
    else
        party->writer->exportCompressedCircuit(scir,valArr, thread);
    return;
}

void Party::writeCircuit(string format, string print, int thread){

    if(format=="off") return;
    if (format=="bin" | format=="uncompressed") {
        funcTime( print, forwardExportFunctions, this, circuitData.scir, circuitData.obfuscatedValArr, true, 1);
        //this->writer->exportBin(Party::circuitData.scir,Party::circuitData.obfuscatedValArr);
    }
    else if(format=="compressed"){
        funcTime( print, forwardExportFunctions, this, circuitData.scir, circuitData.obfuscatedValArr, false, thread);
        //this->writer->exportCompressedCircuit(Party::circuitData.scir,Party::circuitData.obfuscatedValArr,this->compressThreads);
    }
    else if(format=="txt")
    {
        exportCircuitSeparateFiles(Party::circuitData.circuit, CIRCUITPATH + this->circuitName);
        exportObfuscatedInput(circuitData.obfuscatedValArr, circuitData.circuit->details, CIRCUITPATH + this->circuitName);
    }

}


void parseCLIArguments(vector<char*> inputs, Party* party){
    std::string filepath = CIRCUITPATH + party->circuitName;
    if (inputs[0])
    {
        uint_fast64_t a;
        if (check_number(inputs[0]))
        {
            uint_fast64_t a = std::stoul(inputs[0]);
            converIntToBoolArr(a, party->circuitData.circuit->details.bitlengthInputA, party->circuitData.inputA);
        }
        else
        {
            if (inputs[0] == std::string("r"))
                generateRandomInput(party->circuitData.circuit->details.bitlengthInputA, party->circuitData.inputA);
            else
                importBinaryInput(filepath + "_inputA.txt", party->circuitData.circuit->details.bitlengthInputA, party->circuitData.inputA);
        }
    }
    else
        generateRandomInput(party->circuitData.circuit->details.bitlengthInputA, party->circuitData.inputA);

    if (inputs[1])
    {
        uint_fast64_t b;
        if (check_number(inputs[1]))
        {
            uint_fast64_t b = std::stoul(inputs[1]);
            converIntToBoolArr(b, party->circuitData.circuit->details.bitlengthInputB, party->circuitData.inputB);
        }
        else
        {
            if (inputs[1] == std::string("r"))
                generateRandomInput(party->circuitData.circuit->details.bitlengthInputB, party->circuitData.inputB);
            else
                importBinaryInput(filepath + "_inputB.txt", party->circuitData.circuit->details.bitlengthInputB, party->circuitData.inputB);
        }
    }
    else
        generateRandomInput(party->circuitData.circuit->details.bitlengthInputB, party->circuitData.inputB);
}

#endif