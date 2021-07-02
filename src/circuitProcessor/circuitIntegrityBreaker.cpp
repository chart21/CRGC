#include "include/circuitIntegrityBreaker.h"
#include "include/circuitStructs.h"
#include "include/randomBoolGenerator.h"
#include <queue>
#include <thread>

#define parents(i,j)   parents[(i)*2 + (j)] //making 2D array index more natural

void getIntegrityBreakingGates(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors)
{
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numWires; i++)
    {
        if(isObfuscated[i] )
        {
            pathQueue.push(i);
            while(! pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                for(auto j = 0; j < 2; j++)
                {
                    if(! addedGates[parents(currIndex,j)])
                    {
                        addedGates[parents(currIndex,j)] = true;
                        bool allSuccessorwObfuscated = true;
                        for(auto s : successors[parents(currIndex,j)])
                        {
                            if(! isObfuscated[s] and s != 0)
                            {
                                allSuccessorwObfuscated = false;
                            }
                            if(allSuccessorwObfuscated)
                            {
                                isObfuscated[parents(currIndex,j)] = true;
                                pathQueue.push(parents(currIndex,j));
                            }
                        }
                    }
                }
            }
        }
    }
    delete[] addedGates;
     
}

void breakIntegrityOfGates(TransformedCircuit* circuit, bool* isObfuscated)
{
    sfc64 sfc;
    RandomizerWithSentinelShift<> randomizer;
    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        if(isObfuscated[circuit->gates[i].outputID])
        {
            auto lookupTable = circuit->gates[i].truthTable;
            bool isLevelOneGate = false;
            if(circuit->gates[i].leftParentID < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB || circuit->gates[i].rightParentID < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB)
                isLevelOneGate = true;
            if(isLevelOneGate) //obfuscate to a gate indistinguishable from XOR
            {
                bool randBit = randomizer(sfc);
                lookupTable[0][0] = randBit;
                lookupTable[0][1] = ! randBit;                   
                lookupTable[1][0] = ! randBit;
                lookupTable[1][1] = randBit;
            }
            else //obfuscate to a random gate, except 0000/1111
            {
                bool randList[4];
                uint_fast8_t falseCounter = 0;
                while(falseCounter == 0 || falseCounter == 4) //ensure that 0000/1111 does not get generated
                {
                for(auto i = 0; i<4; i++)
                {
                    randList[i] = randomizer(sfc);
                    falseCounter+=randList[i];
                }
                }
                lookupTable[0][0]= randList[0];
                lookupTable[0][1]= randList[1];
                lookupTable[1][0]= randList[2];
                lookupTable[1][1]= randList[3];

            }
        }
    }
}


void getIntegrityBreakingGatesfromOutput(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents)
{
    auto notObfuscated = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for(auto j = 0; j < details.bitlengthOutputs; j++)
        {
            pathQueue.push(details.numWires-1-i-j);
             while(! pathQueue.empty())
             {
                 auto currIndex = pathQueue.front();
                 pathQueue.pop();
                 notObfuscated[currIndex] = true;
                 for (auto p = 0; p < 2; p++)
                 {
                    if(parents(currIndex,p) >= details.bitlengthInputA+details.bitlengthInputB && ! isObfuscated[parents(currIndex,p)]  && ! addedGates[parents(currIndex,p)])
                    {
                        addedGates[parents(currIndex,p)] = true;
                        pathQueue.push(parents(currIndex,p));                        
                    }
                 }
             }
        }
    } 

    for (auto i = details.bitlengthInputA+details.bitlengthInputB; i < details.numWires; i++)
        isObfuscated[i] = ! notObfuscated[i];
    delete[] addedGates; 
    delete[] notObfuscated;
     
}


void getIntegrityBreakingGatesFromOutputThread(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, bool* notobfuscated, bool* addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread)
{
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for(auto j = amountGatesperThread*id; j < amountGatesperThread*(id+1); j++)
        {
            pathQueue.push(details.numWires-1-i-j);
             while(! pathQueue.empty())
             {
                 auto currIndex = pathQueue.front();
                 pathQueue.pop();
                 notobfuscated[currIndex] = true;
                 for (auto p = 0; p < 2; p++)
                 {
                    if(parents(currIndex,p) >= details.bitlengthInputA+details.bitlengthInputB && ! isobfuscated[parents(currIndex,p)]  && not addedGates[parents(currIndex,p)])
                    {
                        addedGates[parents(currIndex,p)] = true;
                        pathQueue.push(parents(currIndex,p));                        
                    }
                 }
             }
        }
    }
}

void getIntegrityBreakingGatesfromOutputMT(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, uint_fast64_t numThreads)
{
    auto notobfuscated = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();    
    uint_fast64_t numGates = details.bitlengthOutputs/numThreads+1;
    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i <numThreads; i++)
    {
        threads[i] = std::thread(getIntegrityBreakingGatesFromOutputThread,details, isobfuscated, parents, notobfuscated, addedGates,i,numGates); 
    }
    for (auto i = 0; i <numThreads; i++)
    {
        threads[i].join();
    }

        
    

    for (auto i = details.bitlengthInputA+details.bitlengthInputB; i < details.numWires; i++)
       isobfuscated[i] = !notobfuscated[i];
    delete[] addedGates; 
    delete[] notobfuscated;
}

void getIntegrityBreakingGatesFromOutputThread2(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, bool* notobfuscated, bool* addedGates, uint_fast64_t id, uint_fast64_t numThreads)
{
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for(auto j = id; j < details.bitlengthOutputs; j+=numThreads)
        {
            pathQueue.push(details.numWires-1-i-j);
             while(! pathQueue.empty())
             {
                 auto currIndex = pathQueue.front();
                 pathQueue.pop();
                 notobfuscated[currIndex] = true;
                 for (auto p = 0; p < 2; p++)
                 {
                    if(parents(currIndex,p) >= details.bitlengthInputA+details.bitlengthInputB && ! isobfuscated[parents(currIndex,p)]  && not addedGates[parents(currIndex,p)])
                    {
                        addedGates[parents(currIndex,p)] = true;
                        pathQueue.push(parents(currIndex,p));                        
                    }
                 }
             }
        }
    }
}

void getIntegrityBreakingGatesfromOutputMT2(CircuitDetails details, bool*isobfuscated, uint_fast64_t* parents, uint_fast64_t numThreads)
{
    auto notobfuscated = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();   
    
    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i <numThreads; i++)
    {
        threads[i] = std::thread(getIntegrityBreakingGatesFromOutputThread2,details, isobfuscated, parents, notobfuscated, addedGates,i,numThreads); 
    }
    for (auto i = 0; i <numThreads; i++)
    {
        threads[i].join();
    }

        
    

    for (auto i = details.bitlengthInputA+details.bitlengthInputB; i < details.numWires; i++)
       isobfuscated[i] = !notobfuscated[i];
    delete[] addedGates; 
    delete[] notobfuscated;
}

void breakIntegrityOfGatesMT(TransformedCircuit* circuit, bool* isObfuscated, uint_fast64_t numThreads)
{
    
    std::thread threads[numThreads];
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(breakIntegrityOfGatesThread, circuit, isObfuscated, i, numThreads);
    }
    for (auto i = 0; i < numThreads; i++)
        threads[i].join();
}

void breakIntegrityOfGatesThread(TransformedCircuit* circuit, bool* isObfuscated, uint_fast64_t id, uint_fast64_t numThreads)
{
    sfc64 sfc;
    RandomizerWithSentinelShift<> randomizer;
    for(auto i = id; i < circuit->details.numGates;i+= numThreads)
    {
        if(isObfuscated[circuit->gates[i].outputID])
        {
            auto lookupTable = circuit->gates[i].truthTable;
            bool isLevelOneGate = false;
            if(circuit->gates[i].leftParentID < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB || circuit->gates[i].rightParentID < circuit->details.bitlengthInputA + circuit->details.bitlengthInputB)
                isLevelOneGate = true;
            if(isLevelOneGate) //obfuscate to a gate indistinguishable from XOR
            {
                bool randBit = randomizer(sfc);
                lookupTable[0][0] = randBit;
                lookupTable[0][1] = ! randBit;                   
                lookupTable[1][0] = ! randBit;
                lookupTable[1][1] = randBit;
            }
            else //obfuscate to a random gate, except 0000/1111
            {
                bool randList[4];
                uint_fast8_t falseCounter = 0;
                while(falseCounter == 0 || falseCounter == 4) //ensure that 0000/1111 does not get generated
                {
                for(auto i = 0; i<4; i++)
                {
                    randList[i] = randomizer(sfc);
                    falseCounter+=randList[i];
                }
                }
                lookupTable[0][0]= randList[0];
                lookupTable[0][1]= randList[1];
                lookupTable[1][0]= randList[2];
                lookupTable[1][1]= randList[3];

            }
        }
    }
}

