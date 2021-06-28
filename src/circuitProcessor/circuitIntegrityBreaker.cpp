#include "include/circuitIntegrityBreaker.h"
#include "include/circuitStructs.h"
#include "include/randomBoolGenerator.h"
#include <queue>

#define parents(i,j)   parents[(i)*2 + (j)] //making 2D array index more natural

void getIntegrityBreakingGatesN4(CircuitDetails details, bool* isObfuscated, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors)
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