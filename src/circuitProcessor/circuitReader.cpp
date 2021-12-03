#include "include/circuitReader.h"
#include "include/circuitStructs.h"
#include "include/helperFunctions.h"
// #include "include/circuitCompressor.h"
#include "../../TurboPFor-Integer-Compression/vp4.h"
#include "../../TurboPFor-Integer-Compression/circuitutil.h"
// #include "emp-tool/emp-tool.h"
// #include "include/circuitHighSpeedNetIO.h"
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>

#define TBL_SHR
//#define BP
#define SEG(n,th) (n+th-n%th)/th
#define ROUND_UP(_n_, _a_) (((_n_) + ((_a_)-1)) & ~((_a_)-1))
#define P4NENC_BOUND(n, size) ((n + 127) / 128 + (n + 32) * (size))
#define P4NDEC_BOUND(n, size) (ROUND_UP(n, 32) * (size))

#define TYPE 1

#define DETAILS_NUM 6
#define UNT 8
#define LIMIT 500000

using namespace std;
vector<std::mutex> mtx_recv(100);

vector<unsigned char*> bufs_recv;
vector<uint32_t> bufLens_recv;
vector<uint32_t> id_recv;
vector<condition_variable> cd_recv(100);

void splitString(std::string s, std::vector<std::string> &v)
{

    std::string temp = "";
    for (auto i = 0; i < s.length(); ++i)
    {

        if (s[i] == ' ')
        {
            v.push_back(temp);
            temp = "";
        }
        else
        {
            temp.push_back(s[i]);
        }
    }
    v.push_back(temp);
}

CircuitDetails importBristolCircuitDetails(std::string filepath, std::string circuitFormat)
{
    std::ifstream f(filepath);
    std::string line;
    uint_fast64_t counter = 0;
    CircuitDetails details;
    if (circuitFormat == "bristol")
    {
        while (std::getline(f, line, '\n'))
        {
            std::vector<std::string> gateElements;
            splitString(line, gateElements);
            if (counter == 0)
            {

                details.numGates = std::stoul(gateElements[0]);
                details.numWires = std::stoul(gateElements[1]);
            }

            else if (counter == 1)
            {
                details.bitlengthInputA = std::stoul(gateElements[1]);
                details.bitlengthInputB = std::stoul(gateElements[2]);
            }

            else if (counter == 2)
            {
                details.numOutputs = std::stoul(gateElements[0]);
                details.bitlengthOutputs = std::stoul(gateElements[1]);
            }
            else if (counter > 2)
                break;

            counter++;
        }
    }
    else if (circuitFormat == "emp")
    {
        details.numOutputs = 1;
        while (std::getline(f, line, '\n'))
        {
            std::vector<std::string> gateElements;
            splitString(line, gateElements);
            if (counter == 0)
            {

                details.numGates = std::stoul(gateElements[0]);
                details.numWires = std::stoul(gateElements[1]);
            }

            else if (counter == 1)
            {
                details.bitlengthInputA = std::stoul(gateElements[0]);
                details.bitlengthInputB = std::stoul(gateElements[1]);
                details.bitlengthOutputs = std::stoul(gateElements[2]);
            }

            else if (counter > 1)
                break;

            counter++;
        }
    }
    else if (circuitFormat == "rgc")
    {
        while (std::getline(f, line, '\n'))
        {
            std::vector<std::string> gateElements;
            splitString(line, gateElements);
            if (counter == 0)
            {

                details.numGates = std::stoul(gateElements[0]);
                details.numWires = std::stoul(gateElements[1]);
            }

            else if (counter == 1)
            {
                details.bitlengthInputA = std::stoul(gateElements[0]);
                details.bitlengthInputB = std::stoul(gateElements[1]);
            }

            else if (counter == 2)
            {
                details.numOutputs = std::stoul(gateElements[0]);
                details.bitlengthOutputs = std::stoul(gateElements[1]);
            }
            else if (counter > 2)
                break;

            counter++;
        }
    }
    f.close();
    return details;
}

TransformedCircuit* importBristolCircuitExNot(std::string filepath, CircuitDetails details)
{
    TransformedGate *gates = new TransformedGate[details.numGates];
    uint_fast64_t *exchangeGate = new uint_fast64_t[details.numWires];
    bool *flipped = new bool[details.numWires];
    for (auto i = 0; i < details.numWires; i++)
    {
        exchangeGate[i] = i;
        flipped[i] = false;
    }

    int gateCounter = 0;

    std::ifstream f(filepath);
    std::string line;
    while (std::getline(f, line, '\n'))
    {
        std::vector<std::string> gateElements;
        splitString(line, gateElements);
        if (gateElements.size() > 4 and gateElements.size() < 7)
        {
            if (gateElements.size() == 5) //not gate
            {
                uint_fast64_t parent = std::stoul(gateElements[2]);
                uint_fast64_t output = std::stoul(gateElements[3]);
                char gateType = gateElements[4][0];

                if (output < details.numWires - details.numOutputs * details.bitlengthOutputs)
                {
                    exchangeGate[output] = exchangeGate[parent];
                    flipped[output] = not flipped[output];
                }
                else
                {
                    bool truthTable[2][2] = {{1, 1}, {0, 0}};

                    if (flipped[parent])
                        swapLeftParent(truthTable);
                    gates[gateCounter] = TransformedGate{exchangeGate[parent], exchangeGate[parent], output, {{truthTable[0][0], truthTable[0][1]}, {truthTable[1][0], truthTable[1][1]}}};
                    gateCounter++;
                }
            }

            else if (gateElements.size() == 6)
            {
                uint_fast64_t leftParent = std::stoul(gateElements[2]);
                uint_fast64_t rightParent = std::stoul(gateElements[3]);
                uint_fast64_t output = std::stoul(gateElements[4]);
                char gateType = gateElements[5][0];

                bool truthTable[2][2];
                if (gateType == 'X')
                    bool truthTable[2][2] = {{0, 1}, {1, 0}};
                else if (gateType == 'A')
                    bool truthTable[2][2] = {{0, 0}, {0, 1}};
                else if (gateType == 'O')
                    bool truthTable[2][2] = {{0, 1}, {1, 1}};

                // if there are problems with above syntax:
                // bool truthTable[2][2];
                // if (gateType == 'X')
                // {
                //     truthTable[0][0] = 0;
                //     truthTable[0][1] = 1;
                //     truthTable[1][0] = 1;
                //     truthTable[1][1] = 0;
                // }
                // else if (gateType == 'A')
                // {
                //     truthTable[0][0] = 0;
                //     truthTable[0][1] = 0;
                //     truthTable[1][0] = 0;
                //     truthTable[1][1] = 1;
                // }
                // else if (gateType == 'O')
                // {
                //     truthTable[0][0] = 0;
                //     truthTable[0][1] = 1;
                //     truthTable[1][0] = 1;
                //     truthTable[1][1] = 1;
                // }

                if (flipped[leftParent])
                    swapLeftParent(truthTable);

                if (flipped[rightParent])
                    swapRightParent(truthTable);

                gates[gateCounter] = TransformedGate{exchangeGate[leftParent], exchangeGate[rightParent], output, {{truthTable[0][0], truthTable[0][1]}, {truthTable[1][0], truthTable[1][1]}}};
                gateCounter++;
            }
        }
    }
    delete[] flipped;
    f.close();
    uint_fast64_t deleteCounter = 0;
    uint_fast64_t *adjustedWire = new uint_fast64_t[details.numWires];
    uint_fast64_t oldNumWires = details.numWires;
    for (int i = 0; i < oldNumWires; i++)
    {
        if (exchangeGate[i] != i)
        {
            deleteCounter++;
            details.numWires--;
            details.numGates--;
        }
        adjustedWire[i] = i - deleteCounter;
    }
    delete[] exchangeGate;

    for (auto i = 0; i < details.numGates; i++)
    {
        gates[i].leftParentID = adjustedWire[gates[i].leftParentID];
        gates[i].rightParentID = adjustedWire[gates[i].rightParentID];
        gates[i].outputID = adjustedWire[gates[i].outputID];
    }
    delete[] adjustedWire;

    TransformedCircuit *circuit = new TransformedCircuit(details);
    TransformedGate *transformedGates = circuit->gates;

    for (auto i = 0; i < details.numGates; i++)
    {
        transformedGates[i] = gates[i];
    }
    cout<<"read: "<<gates[0].truthTable[0][1]<<endl;
    delete[] gates;
    return circuit;
}

BristolCircuit* importBristolCircuitExNotForLeakagePrediction(std::string filepath, CircuitDetails details, bool *flipped)
{
    BristolGate *gates = new BristolGate[details.numGates];
    uint_fast64_t *exchangeGate = new uint_fast64_t[details.numWires];
    for (auto i = 0; i < details.numWires; i++)
    {
        exchangeGate[i] = i;
        flipped[i] = false;
    }

    int gateCounter = 0;

    std::ifstream f(filepath);
    std::string line;
    while (std::getline(f, line, '\n'))
    {
        std::vector<std::string> gateElements;
        splitString(line, gateElements);
        if (gateElements.size() > 4 and gateElements.size() < 7)
        {
            if (gateElements.size() == 5) //not gate
            {
                uint_fast64_t parent = std::stoul(gateElements[2]);
                uint_fast64_t output = std::stoul(gateElements[3]);
                std::string gateType = gateElements[4];

                if (output < details.numWires - details.numOutputs * details.bitlengthOutputs)
                {
                    exchangeGate[output] = exchangeGate[parent];
                    flipped[exchangeGate[output]] = not flipped[exchangeGate[output]];
                }
                else
                {
                    gates[gateCounter] = BristolGate{exchangeGate[parent], exchangeGate[parent], output, 'X'};

                    gateCounter++;
                }
            }

            else if (gateElements.size() == 6)
            {
                uint_fast64_t leftParent = std::stoul(gateElements[2]);
                uint_fast64_t rightParent = std::stoul(gateElements[3]);
                uint_fast64_t output = std::stoul(gateElements[4]);
                std::string gateType = gateElements[5];

                char type;
                if (gateType == "XOR")
                    type = 'X';
                else if (gateType == "AND")
                    type = 'A';
                else if (gateType == "OR")
                    type = 'O';

                gates[gateCounter] = BristolGate{exchangeGate[leftParent], exchangeGate[rightParent], output, type};
                gateCounter++;
            }
        }
    }
    uint_fast64_t deleteCounter = 0;
    uint_fast64_t *adjustedWire = new uint_fast64_t[details.numWires];
    uint_fast64_t oldNumWires = details.numWires;
    for (int i = 0; i < oldNumWires; i++)
    {

        if (exchangeGate[i] != i)
        {
            deleteCounter++;
            details.numWires--;
            details.numGates--;
        }
        adjustedWire[i] = i - deleteCounter;
    }
    delete[] exchangeGate;

    for (auto i = 0; i < details.numGates; i++)
    {
        gates[i].leftParentID = adjustedWire[gates[i].leftParentID];
        gates[i].rightParentID = adjustedWire[gates[i].rightParentID];
        gates[i].outputID = adjustedWire[gates[i].outputID];
        flipped[gates[i].outputID] = flipped[adjustedWire[gates[i].outputID]];
        
    }
    delete[] adjustedWire;


    BristolCircuit *circuit = new BristolCircuit(details);
    BristolGate *bristolGates = circuit->gates;
    for (auto i = 0; i < details.numGates; i++)
    {
        bristolGates[i] = gates[i]; 
    }
    delete[] gates;
    return circuit;
}

void importBinaryInput(std::string filepath, uint_fast64_t bitlength, bool *valArr)
{
    char ch;
    std::fstream fin(filepath, std::fstream::in);
    uint_fast64_t counter = 0;
    while (fin >> std::noskipws >> ch)
    {
        valArr[counter] = ch == '1';
        counter++;
    }
}


TransformedCircuit* importTransformedCircuit(std::string filepath, CircuitDetails details)
{
    std::ifstream f(filepath);
    std::string line;
    TransformedCircuit *circuit = new TransformedCircuit;
    circuit->details = details;
    TransformedGate *gates = new TransformedGate[details.numGates];

    uint_fast64_t gateCounter = 0;
    while (std::getline(f, line, '\n'))
    {
        std::vector<std::string> gateElements;
        splitString(line, gateElements);
        gates[gateCounter] = TransformedGate{std::stoul(gateElements[0]), std::stoul(gateElements[1]), std::stoul(gateElements[2]), {{gateElements[3][0] == '1', gateElements[3][1] == '1'}, {gateElements[3][2] == '1', gateElements[3][3] == '1'}}};
        gateCounter++;
    }
    circuit->gates = gates;
    return circuit;
}

BristolCircuit* importBristolCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate> *gateVec, CircuitDetails details, bool *flipped)
{

    BristolGate *gates = new BristolGate[details.numGates];
    uint_fast64_t *exchangeGate = new uint_fast64_t[details.numWires];
    for (auto i = 0; i < details.numWires; i++)
    {
        exchangeGate[i] = i;
        flipped[i] = false;
    }

    int gateCounter = 0;

    for (auto i = 0; i < details.numGates; i++)
    {

        if ((*gateVec)[i].truthTable == 'I') //not gate
        {

            if ((*gateVec)[i].outputID < details.numWires - details.numOutputs * details.bitlengthOutputs)
            {
                exchangeGate[(*gateVec)[i].outputID] = exchangeGate[(*gateVec)[i].leftParentID];
                flipped[(*gateVec)[i].outputID] = not flipped[(*gateVec)[i].outputID];
            }
            else
            {
                gates[gateCounter] = BristolGate{exchangeGate[(*gateVec)[i].leftParentID], exchangeGate[(*gateVec)[i].leftParentID], (*gateVec)[i].outputID, 'X'};

                gateCounter++;
            }
        }

        else
        {
            uint_fast64_t leftParent = (*gateVec)[i].leftParentID;
            uint_fast64_t rightParent = (*gateVec)[i].rightParentID;
            uint_fast64_t output = (*gateVec)[i].outputID;
            char type = (*gateVec)[i].truthTable;

            gates[gateCounter] = BristolGate{exchangeGate[leftParent], exchangeGate[rightParent], output, type};
            gateCounter++;
        }
    }

    details.numGates=gateCounter;
    uint_fast64_t deleteCounter = 0;
    uint_fast64_t *adjustedWire = new uint_fast64_t[details.numWires];
    uint_fast64_t oldNumWires = details.numWires;

    for (int i = 0; i < oldNumWires; i++)
    {
        if (exchangeGate[i] != i)
        {
            deleteCounter++;
            details.numWires--;
            details.numGates--;
        }
        adjustedWire[i] = i - deleteCounter;
    }
    delete[] exchangeGate;

    for (auto i = 0; i < details.numGates; i++)
    {
        gates[i].leftParentID = adjustedWire[gates[i].leftParentID];
        gates[i].rightParentID = adjustedWire[gates[i].rightParentID];
        gates[i].outputID = adjustedWire[gates[i].outputID];
        flipped[i] = flipped[adjustedWire[i]];
    }
    delete[] adjustedWire;

    BristolCircuit *circuit = new BristolCircuit(details);
    BristolGate *bristolGates = circuit->gates;
    for (auto i = 0; i < details.numGates; i++)
    {
        bristolGates[i] = gates[i];
    }
    delete[] gates;

    return circuit;
}

TransformedCircuit* importTransformedCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate> *gateVec, CircuitDetails details)
{

    TransformedGate *gates = new TransformedGate[details.numGates];
    uint_fast64_t *exchangeGate = new uint_fast64_t[details.numWires];
    bool *flipped = new bool[details.numWires];
    for (auto i = 0; i < details.numWires; i++)
    {
        exchangeGate[i] = i;
        flipped[i] = false;
    }

    int gateCounter = 0;

    for (auto i = 0; i < details.numGates; i++)
    {
        
        if ((*gateVec)[i].truthTable == 'I') //not gate
        {

            if ((*gateVec)[i].outputID < details.numWires - details.numOutputs * details.bitlengthOutputs)
            {
                exchangeGate[(*gateVec)[i].outputID] = exchangeGate[(*gateVec)[i].leftParentID];
                flipped[(*gateVec)[i].outputID] = not flipped[(*gateVec)[i].outputID];
            }
            else
            {
                   bool truthTable[2][2] = {{1, 1}, {0, 0}};

                    if (flipped[(*gateVec)[i].leftParentID])
                        swapLeftParent(truthTable);
                    gates[gateCounter] = TransformedGate{exchangeGate[(*gateVec)[i].leftParentID], exchangeGate[(*gateVec)[i].leftParentID], (*gateVec)[i].outputID, {{truthTable[0][0], truthTable[0][1]}, {truthTable[1][0], truthTable[1][1]}}};
                    gateCounter++;
            }
        }

        else
        {
            uint_fast64_t leftParent = (*gateVec)[i].leftParentID;
            uint_fast64_t rightParent = (*gateVec)[i].rightParentID;
            uint_fast64_t output = (*gateVec)[i].outputID;
            char type = (*gateVec)[i].truthTable;

            bool truthTable[2][2];
                
                if (type == 'X')
                    bool truthTable[2][2] = {{0, 1}, {1, 0}};
                else if (type == 'A')
                    bool truthTable[2][2] = {{0, 0}, {0, 1}};
                else if (type == 'O')
                    bool truthTable[2][2] = {{0, 1}, {1, 1}};

            if (flipped[leftParent])
                swapLeftParent(truthTable);
            if (flipped[rightParent])
                swapRightParent(truthTable);
            
            gates[gateCounter] = TransformedGate{exchangeGate[leftParent], exchangeGate[rightParent], output, {{truthTable[0][0], truthTable[0][1]}, {truthTable[1][0], truthTable[1][1]}}};
            gateCounter++;
        
        }
    }

    delete[] flipped;

    uint_fast64_t deleteCounter = 0;
    uint_fast64_t *adjustedWire = new uint_fast64_t[details.numWires];
    uint_fast64_t oldNumWires = details.numWires;
    for (int i = 0; i < oldNumWires; i++)
    {
        if (exchangeGate[i] != i)
        {
            deleteCounter++;
            details.numWires--;
            details.numGates--;
        }
        adjustedWire[i] = i - deleteCounter;
    }
    delete[] exchangeGate;

    for (auto i = 0; i < details.numGates; i++)
    {
        gates[i].leftParentID = adjustedWire[gates[i].leftParentID];
        gates[i].rightParentID = adjustedWire[gates[i].rightParentID];
        gates[i].outputID = adjustedWire[gates[i].outputID];
    }
    delete[] adjustedWire;

    TransformedCircuit *circuit = new TransformedCircuit(details);
    TransformedGate *transformedGates = circuit->gates;

    for (auto i = 0; i < details.numGates; i++)
    {
        transformedGates[i] = gates[i]; 
    }
    
    delete[] gates;

    return circuit;
}


void decThread(ShrinkedGate* gates_0, size_t seg, size_t residue, int package, int offset, bool store){
    size_t olg=0;
    size_t g_pr=0;
    size_t t_pr=0;

    ShrinkedGate* gates;
    size_t l;
    if(store==false) {
        std::unique_lock<std::mutex> lck(mtx_recv[offset]);
        while(bufs_recv[offset*2+1]==nullptr) cd_recv[offset].wait(lck); // what if cd_recv satisfied but keep looping
        gates = gates_0 + id_recv[offset]*seg;
        l = (id_recv[offset]==package-1)?residue:seg;
    }
    else{
        gates = gates_0 + offset*seg;
        l = (offset==package-1)?residue:seg;
    }
    
    
    size_t tbll = l%2==0 ? l>>1 : (l>>1)+1;

    uint64_t* outGates=decHlp64( bufs_recv[offset*2], l*2, &olg,TYPE);
    delete [] bufs_recv[offset*2];
    bufs_recv[offset*2] = nullptr;

    uint8_t* outTable=decHlp8( bufs_recv[offset*2+1], tbll, &olg,TYPE);
    delete [] bufs_recv[offset*2+1];
    bufs_recv[offset*2+1] = nullptr;


    
    for(int j=0;j<l;j++){
        gates[j].leftParentID = outGates[j];
        gates[j].rightParentOffset = outGates[j+l];

        if(j%2==0){
            gates[j].truthTable[0][0] = outTable[j>>1]&0b01;
            gates[j].truthTable[0][1] = outTable[j>>1]&0b10;
            gates[j].truthTable[1][0] = outTable[j>>1]&0b100;
            gates[j].truthTable[1][1] = outTable[j>>1]&0b1000;

        }
        else{
            gates[j].truthTable[0][0] = outTable[j>>1]&0b10000;
            gates[j].truthTable[0][1] = outTable[j>>1]&0b100000;
            gates[j].truthTable[1][0] = outTable[j>>1]&0b1000000;
            gates[j].truthTable[1][1] = outTable[j>>1]&0b10000000;

        }
    }

        delete [] outGates;
        delete [] outTable;
}

void decompressObfuscatedInput(size_t dataInputLen, bool* &valArr, int package){
    std::unique_lock<std::mutex> lck(mtx_recv[package]);
    while(bufs_recv[package*2+1]==nullptr) cd_recv[package].wait(lck);

    size_t l;
    uint8_t* outInput = decHlp8( bufs_recv[package*2+1], dataInputLen, &l,TYPE);
    delete [] bufs_recv[package*2+1];
    bufs_recv[package*2+1] = nullptr;

    
    std::copy(outInput,outInput+dataInputLen,valArr);
    delete [] outInput;
}

/*
ShrinkedCircuit* decompressShrinkedCircuit(int package, CircuitDetails details) {
    // ShrinkedCircuit* scir = new ShrinkedCircuit(details);
    
    // size_t seg = SEG(details.numGates,package);
    // seg = seg%2==0?seg:seg+1;
    // int ll = details.numGates;

    // vector<thread> threads;
    // for(int i=0;i<package;i++){
    //     size_t l = ll>seg?seg:ll;
    //     threads.push_back(thread(decThread,scir->gates+i*seg,l,i));
    //     ll-=seg;
    // }
    
    // for(auto &th:threads) {
    //     th.join();
    // }
    // return scir;
}*/

template <typename IO>
void Reader<IO>::importCompressedCircuit(ShrinkedCircuit* &scir, bool* &valArr){   
    int package=0;
    recv_data_eva( &package, sizeof(int) );

    bufs_recv.assign(package*2,nullptr);
    bufLens_recv.assign(package*2,0);
    id_recv.assign(package,0);
    // cd_recv.assign(package*2+2,unique_ptr<condition_variable>(new condition_variable));

    unsigned char* buf_detail = new unsigned char[P4NENC_BOUND(DETAILS_NUM,64)];
    uint32_t bufLen_detail;
    recv_data_eva( &bufLen_detail, sizeof(bufLen_detail));
    recv_data_eva( buf_detail, sizeof(buf_detail[0])*bufLen_detail );

    size_t old;
    uint64_t* outDetails = decHlp64(buf_detail, DETAILS_NUM, &old,TYPE);

    delete [] buf_detail;
    buf_detail=nullptr;
    CircuitDetails details;
    details.numWires = outDetails[0];
    details.numGates = outDetails[1];
    details.numOutputs = outDetails[2];
    details.bitlengthInputA = outDetails[3];
    details.bitlengthInputB = outDetails[4];
    details.bitlengthOutputs = outDetails[5];

    scir = new ShrinkedCircuit(details);

    size_t seg = SEG(details.numGates,package);
    seg = seg%2==0?seg:seg+1;
    size_t residue = details.numGates%seg;
    if(store==true) {
        for(int j=0;j<package;j++){
            size_t l=seg;
            if(j==package-1) l=residue;

            bufs_recv[j*2] = new unsigned char[P4NENC_BOUND(l*2,64)];
            recv_data_eva( &(bufLens_recv[j*2]),sizeof(bufLens_recv[j*2]));
            recv_data_eva( bufs_recv[j*2], sizeof(bufs_recv[j*2][0])*bufLens_recv[j*2] );


            size_t tbll = l%2==0 ? l>>1 : (l>>1)+1;

            bufs_recv[1+j*2] = new unsigned char[P4NENC_BOUND( tbll ,8)];
            recv_data_eva( &(bufLens_recv[1+j*2]),sizeof(bufLens_recv[1+j*2]));
            recv_data_eva( bufs_recv[1+j*2], sizeof(bufs_recv[1+j*2][0])*bufLens_recv[1+j*2] );
        }
        vector<thread> threads;
        for(int i=0;i<package;i++){
            threads.push_back(thread(decThread,scir->gates,seg,residue,package,i,this->store));
        }    
        for(auto &th:threads) {
            th.join();
        }
    }
    else {
        thread recvThread([&]() {
            for(int j=0;j<package;j++){
                size_t l=seg;
                if(store==false) {
                    recv_data_eva( &(id_recv[j]), sizeof(id_recv[0]) );
                    l = id_recv[j]==(package-1)?residue:seg;
                }
                else if(j==package-1) l=residue;

                bufs_recv[j*2] = new unsigned char[P4NENC_BOUND(l*2,64)];
                recv_data_eva( &(bufLens_recv[j*2]),sizeof(bufLens_recv[j*2]));
                recv_data_eva( bufs_recv[j*2], sizeof(bufs_recv[j*2][0])*bufLens_recv[j*2] );


                size_t tbll = l%2==0 ? l>>1 : (l>>1)+1;

                bufs_recv[1+j*2] = new unsigned char[P4NENC_BOUND( tbll ,8)];
                recv_data_eva( &(bufLens_recv[1+j*2]),sizeof(bufLens_recv[1+j*2]));
                recv_data_eva( bufs_recv[1+j*2], sizeof(bufs_recv[1+j*2][0])*bufLens_recv[1+j*2] );

                cd_recv[j].notify_one();

            }
            
        });
        vector<thread> threads;
        for(int i=0;i<package;i++){
            threads.push_back(thread(decThread,scir->gates,seg,residue,package,i,this->store));
        }
        recvThread.join();
        for(auto &th:threads) {
            th.join();
        }
    }

    
    uint32_t bufLen_inputa;
    recv_data_eva( &bufLen_inputa, sizeof(bufLen_inputa) );
    unsigned char *buf_inputa = new unsigned char[P4NENC_BOUND(details.bitlengthInputA,8)];
    recv_data_eva( buf_inputa, sizeof(buf_inputa[0])*bufLen_inputa );

    size_t l;
    uint8_t* outInput = decHlp8( buf_inputa, details.bitlengthInputA, &l,TYPE);
    delete [] buf_inputa;
    buf_inputa = nullptr;

    valArr = new bool[details.bitlengthInputA];
    std::copy(outInput,outInput+details.bitlengthInputA,valArr);
    delete [] outInput;

    // decompressObfuscatedInput(scir->details.bitlengthInputA, valArr, package);
    
    return;
}

template <typename IO>
void Reader<IO>::importBin(ShrinkedCircuit* &circuit, bool* &valArr){
    uint64_t cir_param[6];
    recv_data_eva(cir_param, 6*sizeof(uint64_t) );
    CircuitDetails details;
    details.numWires = cir_param[0];
	details.numGates = cir_param[1];
    details.numOutputs = cir_param[2];
    details.bitlengthInputA = cir_param[3];
    details.bitlengthInputB = cir_param[4];    
    details.bitlengthOutputs = cir_param[5];
    auto scir = new ShrinkedCircuit(details);
    ShrinkedGate* gates = scir->gates;

    recv_data_eva(gates, details.numGates*sizeof(ShrinkedGate));
    valArr = new bool[details.bitlengthInputA];
    recv_data_eva(valArr, details.bitlengthInputA*sizeof(bool));

    circuit = scir;
    return;

}


void importObfuscatedInput(bool* &valArr, const CircuitDetails &details, std::string destinationPath){

    if(!destinationPath.empty()){
        std::ifstream outputFile (destinationPath + "_rgc_inputA.txt");
        for (auto i = 0; i < details.bitlengthInputA; i++)
        {
            outputFile >> valArr[i];
        }
        outputFile.close();
    }

    return;

}
