#include "include/leakagePredictor.h"
#include "include/circuitStructs.h"
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
#include <iostream>

#define parents(i, j) parents[(i)*2 + (j)] //making 2D array index more natural

void getPotentiallyObfuscatedFixedGates(BristolCircuit *circuit, bool *po)
{
    
    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        po[i] = true;
    }
    for (auto i = circuit->details.bitlengthInputA; i < circuit->details.bitlengthInputB; i++)
    {
        po[i] = false;
    }
    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        if (circuit->gates[i].truthTable == 'X')
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];
        else
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID];
    }
}

void getPotentiallyObfuscatedFixedGatesT(TransformedCircuit *circuit, bool *po)
{    
    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        po[i] = true;
    }
    for (auto i = circuit->details.bitlengthInputA; i < circuit->details.bitlengthInputB; i++)
    {
        po[i] = false;
    }
    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        uint_fast8_t amountOnes = circuit->gates[i].truthTable[0][0] + circuit->gates[i].truthTable[0][1] + circuit->gates[i].truthTable[1][0] + circuit->gates[i].truthTable[1][1];
        switch (amountOnes)
        {
        case 1:
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID]; //AND or NOR
            break;
        case 3:
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID]; //NAND or OR
            break;
        case 2:
            if (circuit->gates[i].truthTable[0][1] == circuit->gates[i].truthTable[1][0]) //XOR or XNOR
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];
            }
            else if (circuit->gates[i].truthTable[0][0] == circuit->gates[i].truthTable[0][1]) // 0011 or 1100
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID];
            }
            else // 0101 or 1010
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].rightParentID];
            }

            break;

        default:
            po[circuit->gates[i].outputID] = true;
            break;
        }
    }
}

void getPotentiallyIntermediaryGates(CircuitDetails details, bool *po, uint_fast64_t *parents, std::vector<uint_fast64_t> *successors)
{
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numWires; i++)
    {
        if (po[i])
        {
            pathQueue.push(i);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                for (auto j = 0; j < 2; j++)
                {
                    if (!addedGates[parents(currIndex, j)])
                    {
                        addedGates[parents(currIndex, j)] = true;
                        bool allSuccessorsObfuscated = true;
                        for (auto s : successors[parents(currIndex, j)])
                        {
                            if (!po[s] and s != 0)
                            {
                                allSuccessorsObfuscated = false;
                            }
                            if (allSuccessorsObfuscated)
                            {
                                po[parents(currIndex, j)] = true;
                                pathQueue.push(parents(currIndex, j));
                            }
                        }
                    }
                }
            }
        }
    }
    delete[] addedGates;
}

void getPotentiallyIntermediaryGatesFromOutput(CircuitDetails details, bool *po, uint_fast64_t *parents)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for (auto j = 0; j < details.bitlengthOutputs; j++)
        {
            pathQueue.push(details.numWires - 1 - i - j);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                npib[currIndex] = true;
                for (auto p = 0; p < 2; p++)
                {
                    if (parents(currIndex, p) >= details.bitlengthInputA + details.bitlengthInputB && po[parents(currIndex, p)] && not addedGates[parents(currIndex, p)])
                    {
                        addedGates[parents(currIndex, p)] = true;
                        pathQueue.push(parents(currIndex, p));
                    }
                }
            }
        }
    }

    for (auto i = 0; i < details.numWires; i++)
        po[i] = !npib[i];
    delete[] addedGates;
    delete[] npib;
}

void getPotentiallyIntermediaryGatesFromOutputThread(CircuitDetails details, bool *po, uint_fast64_t *parents, bool *npib, bool *addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread)
{
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for (auto j = amountGatesperThread * id; j < std::min(amountGatesperThread * (id + 1), details.bitlengthOutputs); j++)
        {
            pathQueue.push(details.numWires - 1 - i - j);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                npib[currIndex] = true;
                for (auto p = 0; p < 2; p++)
                {
                    if (parents(currIndex, p) >= details.bitlengthInputA + details.bitlengthInputB && po[parents(currIndex, p)] && not addedGates[parents(currIndex, p)])
                    {
                        addedGates[parents(currIndex, p)] = true;
                        pathQueue.push(parents(currIndex, p));
                    }
                }
            }
        }
    }
}

void getPotentiallyIntermediaryGatesFromOutputMT(CircuitDetails details, bool *po, uint_fast64_t *parents, uint_fast64_t numThreads)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();
    //uint_fast64_t numGates = details.bitlengthOutputs / numThreads + 1;
    uint_fast64_t numGates = details.bitlengthOutputs / numThreads;
    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyIntermediaryGatesFromOutputThread, details, po, parents, npib, addedGates, i, numGates);
    }
    getPotentiallyIntermediaryGatesFromOutputThread(details, po, parents, npib, addedGates, numThreads, numGates); //process leftover gates, remainder
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    for (auto i = 0; i < details.numWires; i++)
        po[i] = !npib[i];
    delete[] addedGates;
    delete[] npib;
}



void getPotentiallyIntermediaryGatesFromOutputThread2(CircuitDetails details, bool *po, uint_fast64_t *parents, bool *npib, bool *addedGates, uint_fast64_t id, uint_fast64_t numThreads)
{
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for (auto j = id; j < details.bitlengthOutputs; j += numThreads)
        {
            pathQueue.push(details.numWires - 1 - i - j);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                npib[currIndex] = true;
                for (auto p = 0; p < 2; p++)
                {
                    if (parents(currIndex, p) >= details.bitlengthInputA + details.bitlengthInputB && po[parents(currIndex, p)] && not addedGates[parents(currIndex, p)])
                    {
                        addedGates[parents(currIndex, p)] = true;
                        pathQueue.push(parents(currIndex, p));
                    }
                }
            }
        }
    }
}

void getPotentiallyIntermediaryGatesFromOutputMT2(CircuitDetails details, bool *po, uint_fast64_t *parents, uint_fast64_t numThreads)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();

    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyIntermediaryGatesFromOutputThread2, details, po, parents, npib, addedGates, i, numThreads);
    }
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    for (auto i = 0; i < details.numWires; i++)
        po[i] = !npib[i];
    delete[] addedGates;
    delete[] npib;
}

//only works for circuits sorted by outputID
void getLeakedInputsFromOutput(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs)
{

    bool *addedGates[2] = {new bool[circuit->details.numWires](), new bool[circuit->details.numWires]()}; //one for each queue
    std::queue<uint_fast64_t> pathQueue[2];

    //std::vector<uint_fast64_t> leakedInputs;
    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        for (auto j = 0; j < circuit->details.bitlengthOutputs; j++)
        {
            auto outputWireGateIndex = circuit->details.numWires - 1 - i - j - circuit->details.bitlengthInputA - circuit->details.bitlengthInputB;
            uint_fast8_t amountOnesCurrIndex = circuit->gates[outputWireGateIndex].truthTable[0][0] + circuit->gates[outputWireGateIndex].truthTable[0][1] + circuit->gates[outputWireGateIndex].truthTable[1][0] + circuit->gates[outputWireGateIndex].truthTable[1][1];
            bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;           
            
            if(isImbalancedGate)
                pathQueue[0].push(outputWireGateIndex); 
            else
                pathQueue[1].push(outputWireGateIndex); //no leakage after one balanced gate           
            
            while (!pathQueue[0].empty() || !pathQueue[1].empty())
            {
                for (auto imbalancedCount = 0; imbalancedCount < 2; imbalancedCount++)
                {
                    while (!pathQueue[imbalancedCount].empty())
                    {
                        auto currIndex = pathQueue[imbalancedCount].front();
                        pathQueue[imbalancedCount].pop();
                        uint_fast8_t amountOnesCurrIndex = circuit->gates[currIndex].truthTable[0][0] + circuit->gates[currIndex].truthTable[0][1] + circuit->gates[currIndex].truthTable[1][0] + circuit->gates[currIndex].truthTable[1][1];
                        uint_fast64_t parents[2] = {circuit->gates[currIndex].leftParentID, circuit->gates[currIndex].rightParentID};

                        for (auto p = 0; p < 2; p++)
                        {
                            if (! addedGates[0][p]) //no need to check gates twice
                            {

                                if (parents[p] < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB) //input wire
                                {
                                    
                                    if (parents[p] < circuit->details.bitlengthInputA && !addedGates[0][parents[p]])
                                        leakedInputs->push_back(parents[p]);
                                    addedGates[0][parents[p]] = true;
                                }

                                else
                                {
                                    if(imbalancedCount == 0 || !addedGates[1][parents[p]]) //no need to check gates twice
                                    {

                                    
                                        auto parentGateIndex = parents[p] - circuit->details.bitlengthInputA - circuit->details.bitlengthInputB;
                                        uint_fast8_t amountOnesParent = circuit->gates[parentGateIndex].truthTable[0][0] + circuit->gates[parentGateIndex].truthTable[0][1] + circuit->gates[parentGateIndex].truthTable[1][0] + circuit->gates[parentGateIndex].truthTable[1][1];
                                        switch (amountOnesParent)
                                        {
                                        case 1: //AND or NOR

                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);
                                                
                                            }

                                            break;
                                        case 3:                                                  //NAND or NOR
                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);                                                
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);                                                
                                            }
                                            break;
                                        case 2: //balanced gates
                                            if (imbalancedCount == 0)
                                            {
                                                if (circuit->gates[parentGateIndex].truthTable[0][1] == circuit->gates[parentGateIndex].truthTable[1][0]) //XOR or XNOR
                                                {
                                                    addedGates[1][parents[p]] = true;
                                                    pathQueue[1].push(parentGateIndex);
                                                }
                                                else if (circuit->gates[parentGateIndex].truthTable[0][0] == circuit->gates[parentGateIndex].truthTable[0][1]) // 0011 or 1100
                                                {
                                                    if (p == 0)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                                else // 0101 or 1010
                                                {
                                                    if (p == 1)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                            }
                                            break;

                                        default: //0000 or 1111
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        //delete[] po;
    }
    delete[] addedGates[0];
    delete[] addedGates[1];
}


void getLeakedInputsFromOutputUnsorted(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t* circuitLineOfWireIndex)
{

    bool *addedGates[2] = {new bool[circuit->details.numWires](), new bool[circuit->details.numWires]()}; //one for each queue
    std::queue<uint_fast64_t> pathQueue[2];

    //std::vector<uint_fast64_t> leakedInputs;
    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        for (auto j = 0; j < circuit->details.bitlengthOutputs; j++)
        {
            auto outputWireGateIndex = circuitLineOfWireIndex[circuit->details.numWires - 1 - i - j]; 

            uint_fast8_t amountOnesCurrIndex = circuit->gates[outputWireGateIndex].truthTable[0][0] + circuit->gates[outputWireGateIndex].truthTable[0][1] + circuit->gates[outputWireGateIndex].truthTable[1][0] + circuit->gates[outputWireGateIndex].truthTable[1][1];
            bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;           
            
            if(isImbalancedGate)
                pathQueue[0].push(outputWireGateIndex); 
            else
                pathQueue[1].push(outputWireGateIndex); //no leakage after one balanced gate           
            
            while (!pathQueue[0].empty() || !pathQueue[1].empty())
            {
                for (auto imbalancedCount = 0; imbalancedCount < 2; imbalancedCount++)
                {
                    while (!pathQueue[imbalancedCount].empty())
                    {
                        auto currIndex = pathQueue[imbalancedCount].front();
                        pathQueue[imbalancedCount].pop();
                        //uint_fast8_t amountOnesCurrIndex = circuit->gates[currIndex].truthTable[0][0] + circuit->gates[currIndex].truthTable[0][1] + circuit->gates[currIndex].truthTable[1][0] + circuit->gates[currIndex].truthTable[1][1];
                        //bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;
                        uint_fast64_t parents[2] = {circuit->gates[currIndex].leftParentID, circuit->gates[currIndex].rightParentID};
                        
                        for (auto p = 0; p < 2; p++)
                        {
                            if (! addedGates[0][p]) //no need to check gates twice
                            {

                                if (parents[p] < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB) //input wire
                                {
                                    
                                    if (parents[p] < circuit->details.bitlengthInputA && !addedGates[0][parents[p]])
                                        leakedInputs->push_back(parents[p]);
                                    addedGates[0][parents[p]] = true;
                                }

                                else
                                {
                                    if(imbalancedCount == 0 || !addedGates[1][parents[p]]) //no need to check gates twice
                                    {

                                    
                                        auto parentGateIndex = circuitLineOfWireIndex[parents[p]];
                                        uint_fast8_t amountOnesParent = circuit->gates[parentGateIndex].truthTable[0][0] + circuit->gates[parentGateIndex].truthTable[0][1] + circuit->gates[parentGateIndex].truthTable[1][0] + circuit->gates[parentGateIndex].truthTable[1][1];
                                        
                                        
                                        switch (amountOnesParent)
                                        {
                                        case 1: //AND or NOR

                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);
                                                
                                            }

                                            break;
                                        case 3:                                                  //NAND or NOR
                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);                                                
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);                                                
                                            }
                                            break;
                                        case 2: //balanced gates
                                            if (imbalancedCount == 0)
                                            {
                                                if (circuit->gates[parentGateIndex].truthTable[0][1] == circuit->gates[parentGateIndex].truthTable[1][0]) //XOR or XNOR
                                                {
                                                    addedGates[1][parents[p]] = true;
                                                    pathQueue[1].push(parentGateIndex);
                                                }
                                                else if (circuit->gates[parentGateIndex].truthTable[0][0] == circuit->gates[parentGateIndex].truthTable[0][1]) // 0011 or 1100
                                                {
                                                    if (p == 0)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                                else // 0101 or 1010
                                                {
                                                    if (p == 1)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                            }
                                            break;

                                        default: //0000 or 1111
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        //delete[] po;
    }
    delete[] addedGates[0];
    delete[] addedGates[1];
}

void getLeakedInputsFromOutputUnsortedThread(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t* circuitLineOfWireIndex, bool *addedGates[2], uint_fast64_t id, uint_fast64_t numThreads)
{
std::queue<uint_fast64_t> pathQueue[2];

    //std::vector<uint_fast64_t> leakedInputs;
    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        
        for (auto j = id; j < circuit->details.bitlengthOutputs; j+=numThreads)
        {
            auto outputWireGateIndex = circuitLineOfWireIndex[circuit->details.numWires - 1 - i - j]; 

            uint_fast8_t amountOnesCurrIndex = circuit->gates[outputWireGateIndex].truthTable[0][0] + circuit->gates[outputWireGateIndex].truthTable[0][1] + circuit->gates[outputWireGateIndex].truthTable[1][0] + circuit->gates[outputWireGateIndex].truthTable[1][1];
            bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;           
            
            if(isImbalancedGate)
                pathQueue[0].push(outputWireGateIndex); 
            else
                pathQueue[1].push(outputWireGateIndex); //no leakage after one balanced gate  
                       
            
            while (!pathQueue[0].empty() || !pathQueue[1].empty())
            {
                for (auto imbalancedCount = 0; imbalancedCount < 2; imbalancedCount++)
                {
                    
                    while (!pathQueue[imbalancedCount].empty())
                    {
                        auto currIndex = pathQueue[imbalancedCount].front();
                        pathQueue[imbalancedCount].pop();
                        //uint_fast8_t amountOnesCurrIndex = circuit->gates[currIndex].truthTable[0][0] + circuit->gates[currIndex].truthTable[0][1] + circuit->gates[currIndex].truthTable[1][0] + circuit->gates[currIndex].truthTable[1][1];
                        //bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;
                        uint_fast64_t parents[2] = {circuit->gates[currIndex].leftParentID, circuit->gates[currIndex].rightParentID};
                        
                        
                        for (auto p = 0; p < 2; p++)
                        {
                            if (! addedGates[0][p]) //no need to check gates twice
                            {

                                if (parents[p] < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB) //input wire
                                {
                                    
                                    
                                    if (parents[p] < circuit->details.bitlengthInputA && !addedGates[0][parents[p]])
                                        leakedInputs->push_back(parents[p]);
                                    addedGates[0][parents[p]] = true;
                                }

                                else
                                {
                                    if(imbalancedCount == 0 || !addedGates[1][parents[p]]) //no need to check gates twice
                                    {

                                    
                                        auto parentGateIndex = circuitLineOfWireIndex[parents[p]];
                                        uint_fast8_t amountOnesParent = circuit->gates[parentGateIndex].truthTable[0][0] + circuit->gates[parentGateIndex].truthTable[0][1] + circuit->gates[parentGateIndex].truthTable[1][0] + circuit->gates[parentGateIndex].truthTable[1][1];
                                        
                                        
                                        switch (amountOnesParent)
                                        {
                                        case 1: //AND or NOR

                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);
                                                
                                            }

                                            break;
                                        case 3:                                                  //NAND or NOR
                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);                                                
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);                                                
                                            }
                                            break;
                                        case 2: //balanced gates
                                            if (imbalancedCount == 0)
                                            {
                                                if (circuit->gates[parentGateIndex].truthTable[0][1] == circuit->gates[parentGateIndex].truthTable[1][0]) //XOR or XNOR
                                                {
                                                    addedGates[1][parents[p]] = true;
                                                    pathQueue[1].push(parentGateIndex);
                                                }
                                                else if (circuit->gates[parentGateIndex].truthTable[0][0] == circuit->gates[parentGateIndex].truthTable[0][1]) // 0011 or 1100
                                                {
                                                    if (p == 0)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                                else // 0101 or 1010
                                                {
                                                    if (p == 1)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                            }
                                            break;

                                        default: //0000 or 1111
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        //delete[] po;
    }
}

void getLeakedInputsFromOutputSortedThread(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, bool *addedGates[2], uint_fast64_t id, uint_fast64_t numThreads)
{
std::queue<uint_fast64_t> pathQueue[2];

    //std::vector<uint_fast64_t> leakedInputs;
    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        
        for (auto j = id; j < circuit->details.bitlengthOutputs; j+=numThreads)
        {
            auto outputWireGateIndex = circuit->details.numWires - 1 - i - j - circuit->details.bitlengthInputA - circuit->details.bitlengthInputB; 

            uint_fast8_t amountOnesCurrIndex = circuit->gates[outputWireGateIndex].truthTable[0][0] + circuit->gates[outputWireGateIndex].truthTable[0][1] + circuit->gates[outputWireGateIndex].truthTable[1][0] + circuit->gates[outputWireGateIndex].truthTable[1][1];
            bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;           
            
            if(isImbalancedGate)
                pathQueue[0].push(outputWireGateIndex); 
            else
                pathQueue[1].push(outputWireGateIndex); //no leakage after one balanced gate           
            
            while (!pathQueue[0].empty() || !pathQueue[1].empty())
            {
                for (auto imbalancedCount = 0; imbalancedCount < 2; imbalancedCount++)
                {
                    while (!pathQueue[imbalancedCount].empty())
                    {
                        auto currIndex = pathQueue[imbalancedCount].front();
                        pathQueue[imbalancedCount].pop();
                        //uint_fast8_t amountOnesCurrIndex = circuit->gates[currIndex].truthTable[0][0] + circuit->gates[currIndex].truthTable[0][1] + circuit->gates[currIndex].truthTable[1][0] + circuit->gates[currIndex].truthTable[1][1];
                        //bool isImbalancedGate = amountOnesCurrIndex == 1 || amountOnesCurrIndex == 3;
                        uint_fast64_t parents[2] = {circuit->gates[currIndex].leftParentID, circuit->gates[currIndex].rightParentID};
                        
                        for (auto p = 0; p < 2; p++)
                        {
                            
                            if (! addedGates[0][p]) //no need to check gates twice
                            {

                                if (parents[p] < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB) //input wire
                                {
                                    
                                    
                                    if (parents[p] < circuit->details.bitlengthInputA && !addedGates[0][parents[p]])
                                        leakedInputs->push_back(parents[p]);
                                    addedGates[0][parents[p]] = true;
                                }

                                else
                                {
                                    if(imbalancedCount == 0 || !addedGates[1][parents[p]]) //no need to check gates twice
                                    {

                                    
                                        auto parentGateIndex = parents[p] - circuit->details.bitlengthInputA - circuit->details.bitlengthInputB;
                                        uint_fast8_t amountOnesParent = circuit->gates[parentGateIndex].truthTable[0][0] + circuit->gates[parentGateIndex].truthTable[0][1] + circuit->gates[parentGateIndex].truthTable[1][0] + circuit->gates[parentGateIndex].truthTable[1][1];
                                        
                                        
                                        switch (amountOnesParent)
                                        {
                                        case 1: //AND or NOR

                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);
                                                
                                            }

                                            break;
                                        case 3:                                                  //NAND or NOR
                                            if (imbalancedCount == 0 && po[parents[p] == false]) //imbalanced gates refreshes XOR count
                                            {
                                                addedGates[0][parents[p]] = true;
                                                pathQueue[0].push(parentGateIndex);                                                
                                            }
                                            else
                                            {
                                                addedGates[imbalancedCount][parents[p]] = true;
                                                pathQueue[imbalancedCount].push(parentGateIndex);                                                
                                            }
                                            break;
                                        case 2: //balanced gates
                                            if (imbalancedCount == 0)
                                            {
                                                if (circuit->gates[parentGateIndex].truthTable[0][1] == circuit->gates[parentGateIndex].truthTable[1][0]) //XOR or XNOR
                                                {
                                                    addedGates[1][parents[p]] = true;
                                                    pathQueue[1].push(parentGateIndex);
                                                }
                                                else if (circuit->gates[parentGateIndex].truthTable[0][0] == circuit->gates[parentGateIndex].truthTable[0][1]) // 0011 or 1100
                                                {
                                                    if (p == 0)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                                else // 0101 or 1010
                                                {
                                                    if (p == 1)
                                                    {
                                                        addedGates[1][parents[p]] = true;
                                                        pathQueue[1].push(parentGateIndex);
                                                    }
                                                }
                                            }
                                            break;

                                        default: //0000 or 1111
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        //delete[] po;
    }
}



void getLeakedInputsFromOutputUnsortedMT(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t* circuitLineOfWireIndex, uint_fast64_t numThreads)
{

    bool *addedGates[2] = {new bool[circuit->details.numWires](), new bool[circuit->details.numWires]()}; //one for each queue

    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getLeakedInputsFromOutputUnsortedThread, circuit, po, leakedInputs, circuitLineOfWireIndex, addedGates, i, numThreads);
    }
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }
    
    delete[] addedGates[0];
    delete[] addedGates[1];
}


void getLeakedInputsFromOutputSortedMT(TransformedCircuit *circuit, bool *po, std::vector<uint_fast64_t>* leakedInputs, uint_fast64_t numThreads)
{

    bool *addedGates[2] = {new bool[circuit->details.numWires](), new bool[circuit->details.numWires]()}; //one for each queue

    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getLeakedInputsFromOutputSortedThread, circuit, po, leakedInputs, addedGates, i, numThreads);
    }
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }
    
    delete[] addedGates[0];
    delete[] addedGates[1];
}
