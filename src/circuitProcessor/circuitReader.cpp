#include "include/circuitReader.h"
#include "include/circuitStructs.h"
#include "include/helperFunctions.h"
#include "include/circuitCompressor.h"
#include "../../TurboPFor-Integer-Compression/vp4.h"
#include "../../TurboPFor-Integer-Compression/circuitutil.h"
//#include "emp-tool/emp-tool.h"
#include "include/circuitHighSpeedNetIO.h"
#include <fstream>
#include <vector>

using namespace std;

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
    details.numGates = gateCounter;
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

    // TransformedCircuit *circuit = new TransformedCircuit(details);
    // TransformedGate *transformedGates = circuit->gates;
    TransformedGate *transformedGates = new TransformedGate[details.numGates];
    for (auto i = 0; i < details.numGates; i++)
    {
        transformedGates[i] = gates[i]; //does that really copy?
    }
    delete[] gates;

    TransformedCircuit *circuit = new TransformedCircuit;
    *circuit = {details, transformedGates};

    return circuit;
}

BristolCircuit* importBristolCircuitExNotForLeakagePrediction(std::string filepath, CircuitDetails details, bool *flipped)
{
    BristolGate *gates = new BristolGate[details.numGates];
    uint_fast64_t *exchangeGate = new uint_fast64_t[details.numWires];
    //flipped = new bool[details.numWires];
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
                    //flipped[output] = not flipped[output];
                    flipped[exchangeGate[output]] = not flipped[exchangeGate[output]];
                }
                else
                {
                    //gates[gateCounter] = BristolGate { 'X', };
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
    //delete[] flipped;

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
        //flipped[adjustedWire[i]] = flipped[i];
        flipped[gates[i].outputID] = flipped[adjustedWire[gates[i].outputID]];
        //std::cout << flipped[i];
    }
    delete[] adjustedWire;

    BristolGate *bristolGates = new BristolGate[details.numGates];
    for (auto i = 0; i < details.numGates; i++)
    {
        bristolGates[i] = gates[i]; //does that really copy?
    }
    delete[] gates;

    BristolCircuit *circuit = new BristolCircuit;
    *circuit = {details, bristolGates};

    return circuit;
}

void importBinaryInput(std::string filepath, uint_fast64_t bitlength, bool *valArr)
{
    //auto valArr = new bool[bitlength];
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
    //bool *flipped = new bool[details.numWires];
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
                //gates[gateCounter] = BristolGate { 'X', };

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

    //delete[] flipped;
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
        //std::cout << gates[i].leftParentID <<' ' <<  gates[i].rightParentID <<' ' << gates[i].outputID <<' ' << gates[i].truthTable <<'\n';
        gates[i].leftParentID = adjustedWire[gates[i].leftParentID];
        gates[i].rightParentID = adjustedWire[gates[i].rightParentID];
        gates[i].outputID = adjustedWire[gates[i].outputID];
        flipped[i] = flipped[adjustedWire[i]];
    }
    delete[] adjustedWire;

    BristolGate *bristolGates = new BristolGate[details.numGates];
    for (auto i = 0; i < details.numGates; i++)
    {
        bristolGates[i] = gates[i]; //does that really copy?
    }
    delete[] gates;

    BristolCircuit *circuit = new BristolCircuit;
    *circuit = {details, bristolGates};

    return circuit;
}

template <typename IO>
TransformedCircuit* Eva<IO>::importTransformedCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate> *gateVec, CircuitDetails details)
{
    if(io) {
        // io->recv_data(&details, sizeof(details));
        recv_data_eva(&details, sizeof(details));
    }
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
        if(io){
            BristolGate bg;
            // io->recv_data(&bg, sizeof(BristolGate));
            recv_data_eva(&bg, sizeof(BristolGate));
            gateVec->push_back(bg);
        }
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


    TransformedGate *transformedGates = new TransformedGate[details.numGates];
    for (auto i = 0; i < details.numGates; i++)
    {
        transformedGates[i] = gates[i]; //does that really copy?
    }
    delete[] gates;

    TransformedCircuit *circuit = new TransformedCircuit;
    *circuit = {details, transformedGates};

    return circuit;
}

template <typename IO>
ShrinkedCircuit* Eva<IO>::importCompressedCircuit( int thr_dec){
    //chrono::time_point<std::chrono::system_clock> start, end_read, end_dec,a,b;
    //start = std::chrono::system_clock::now();

    // FILE * denc;
    // denc = fopen((filepath+"_compressed.dat").c_str(),"r");
    // denc = fopen("/home/ubuntu/BristolCircuitCompressor/src/circuits/insurance10_compressed.dat","r");
    // if(denc==NULL) std::cout<<"failed to open dec file"<<std::endl;

    size_t len;
    //fread(&len, sizeof(size_t), 1, denc);
    // io->recv_data( &len, sizeof(size_t) );
    recv_data_eva( &len, sizeof(size_t) );

    uint32_t bufLensSize;
    //fread(&bufLensSize,sizeof(uint32_t), 1, denc);
    // io->recv_data( &bufLensSize, sizeof(uint32_t) );
    recv_data_eva( &bufLensSize, sizeof(uint32_t) );
    unsigned char* bufLens = new unsigned char[P4NENC_BOUND(len,32)];

    //size_t test1 = fread(bufLens,sizeof(bufLens[0]), bufLensSize, denc);
    // io->recv_data( bufLens, sizeof(bufLens[0])*bufLensSize );
    recv_data_eva( bufLens, sizeof(bufLens[0])*bufLensSize );

    size_t old;
    uint32_t* outLens = decHlp32(bufLens, len, &old,TYPE);
    delete [] bufLens;

    unsigned char* bufDetails = new unsigned char[P4NENC_BOUND(DETAILS_NUM,64)];

    //size_t bld=fread(bufDetails,sizeof(bufDetails[0]), outLens[0], denc);
    // io->recv_data( bufDetails, sizeof(bufDetails[0])*outLens[0] );
    recv_data_eva( bufDetails, sizeof(bufDetails[0])*outLens[0] );
    uint64_t* outDetails = decHlp64(bufDetails, DETAILS_NUM, &old,TYPE);
    delete [] bufDetails;

    CircuitDetails details;
    details.numWires = outDetails[0];
    details.numGates = outDetails[1];
    details.numOutputs = outDetails[2];
    details.bitlengthInputA = outDetails[3];
    details.bitlengthInputB = outDetails[4];
    details.bitlengthOutputs = outDetails[5];


    ShrinkedCircuit* scir = new ShrinkedCircuit(details);

    vector<unsigned char*> bufGates;
    vector<unsigned char*> bufTables;
    vector<size_t> dataLens;
    size_t thr_enc = (len-1)/2;
    size_t seg = SEG(scir->details.numGates,thr_enc);
    // size_t seg = ROUND_UP(scir->details.numGates,thr_enc)/thr_enc;
    seg = seg%2==0?seg:seg+1;
    int ll = details.numGates;
    //a = std::chrono::system_clock::now();

    int offset=0;
    for(int j=0;j<thr_enc;j++){
        size_t l = ll>seg?seg:ll;

        unsigned char* bufGate = new unsigned char[P4NENC_BOUND(l*2,64)];
        //size_t blg=fread(bufGate,sizeof(bufGate[0]), outLens[1+j*2], denc);
        // io->recv_data( bufGate, sizeof(bufGate[0])*outLens[1+j*2] );
        recv_data_eva( bufGate, sizeof(bufGate[0])*outLens[1+j*2] );
        bufGates.push_back(bufGate);
        
        size_t tbll = l%2==0 ? l>>1 : (l>>1)+1;
        unsigned char* bufTable = new unsigned char[P4NENC_BOUND( tbll ,8)];
        //size_t blt=fread(bufTable,sizeof(bufTable[0]), outLens[j*2+2], denc);
        // io->recv_data( bufTable, sizeof(bufTable[0])*outLens[2+j*2] );
        recv_data_eva( bufTable, sizeof(bufTable[0])*outLens[2+j*2] );
        bufTables.push_back(bufTable);

        dataLens.push_back(l);

        ll-=seg;

    }
    delete [] outLens;
    // fclose(denc);
    //end_read = std::chrono::system_clock::now();
    //chrono::duration<double> elapsed_seconds = end_read - start;
    //t_read_sum += chrono::duration_cast<chrono::microseconds>(end_read - start).count();
    //t_read_sum += chrono::duration_cast<chrono::milliseconds>(end_read - start).count();
    //cout<<"finished read, elapsed seconds: "<< chrono::duration_cast<chrono::milliseconds>(end_read - start).count() 
    //    <<", " << chrono::duration_cast<chrono::milliseconds>(end_read - a).count() <<endl;
    
    decompressShrinkedCircuit(bufGates, bufTables, dataLens, scir, thr_enc, thr_dec, seg);
    //end_dec = std::chrono::system_clock::now();
    //elapsed_seconds = end_dec - start;
    //t_dec_sum += chrono::duration_cast<chrono::microseconds>(end_dec - start).count();
    //t_dec_sum += chrono::duration_cast<chrono::milliseconds>(end_dec - start).count();
    //cout<<"finished dec, elapsed seconds: "<< chrono::duration_cast<chrono::milliseconds>(end_dec - start).count() <<endl;

    return scir;
}

template <typename IO>
ShrinkedCircuit* Eva<IO>::importBin(){
    //std::chrono::time_point<std::chrono::system_clock> start, end;
    //start = std::chrono::system_clock::now();

    //FILE *f;
    //f = fopen((filepath+".bin").c_str(),"r");
    uint64_t cir_param[6];
    //fread((char*)cir_param, 1, 6*sizeof(uint64_t), f);
    //fread(cir_param, 1, 6*sizeof(uint64_t), f);
    // io->recv_data(cir_param, 6*sizeof(uint64_t) );
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
    //for(int i=0;i<details.numGates;i++){
        //fread(&gates[i].leftParentID,1,sizeof(uint64_t),f);
        //fread(&gates[i].rightParentOffset,1,sizeof(uint64_t),f);
        //fread(gates[i].truthTable,1,1,f);
        //fread(gates,1,details.numGates*sizeof(ShrinkedGate),f);
    //}
    // io->recv_data(gates, details.numGates*sizeof(ShrinkedGate));
    recv_data_eva(gates, details.numGates*sizeof(ShrinkedGate));
    //fclose(f);
    //end = std::chrono::system_clock::now();
    //std::chrono::duration<double> elapsed_seconds = end - start;

    //std::cout<<"finished import bin, elapsed ms:" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
    return scir;

}
