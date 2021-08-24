#include "include/circuitReader.h"
#include "include/circuitStructs.h"
#include "include/helperFunctions.h"
#include <fstream>
#include <vector>

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

TransformedCircuit *importBristolCircuitExNot(std::string filepath, CircuitDetails details)
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

BristolCircuit *importBristolCircuitExNotForLeakagePrediction(std::string filepath, CircuitDetails details, bool *flipped)
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

TransformedCircuit *importTransformedCircuit(std::string filepath, CircuitDetails details)
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

BristolCircuit *importBristolCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate> *gateVec, CircuitDetails details, bool *flipped)
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

TransformedCircuit *importTransformedCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate> *gateVec, CircuitDetails details)
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

