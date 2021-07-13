#include "include/circuitDetails.h"
#include "include/circuitStructs.h"
#include <iostream>
#include <vector>
#include <thread>

#define parents(i,j)   parents[(i)*2 + (j)] //making 2D array index more natural
//#include "include/circuitDetails.h"

void getPrevofEachWire(BristolCircuit* circuit, uint_fast64_t* parents)
{
    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        parents(circuit->gates[i].outputID,0) = circuit->gates[i].leftParentID;    
        parents(circuit->gates[i].outputID,1) = circuit->gates[i].rightParentID;  
    }    
}

void getPrevofEachWireT(TransformedCircuit* circuit, uint_fast64_t* parents)
{
    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        parents(circuit->gates[i].outputID,0) = circuit->gates[i].leftParentID;    
        parents(circuit->gates[i].outputID,1) = circuit->gates[i].rightParentID;  
    }    
}








void getSuccessorsOfEachWire(CircuitDetails details, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors)
{
    for(auto id = 0; id < details.numWires;id++)
    {
        for(auto p = 0; p < 2; p++)
        {
            if(parents(id,p) != 0)
                successors[parents(id,p)].push_back(id);
        }
    }    
}

void getLevelOfGates(BristolCircuit* circuit, uint_fast64_t* levels)
{
    for(auto i = 0; i < circuit->details.bitlengthInputA+circuit->details.bitlengthInputB;i++)
    {
        levels[i] = 0;
    }
    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        levels[circuit->gates[i].outputID] = std::max(levels[circuit->gates[i].leftParentID],levels[circuit->gates[i].rightParentID])+1;
        std::cout << levels[circuit->gates[i].outputID] << '\n';
    }

}

uint_fast64_t getLevelOfGates(TransformedCircuit* circuit, uint_fast64_t* levels)
{
    uint_fast64_t maxLevel = 0;
    for(auto i = 0; i < circuit->details.bitlengthInputA+circuit->details.bitlengthInputB;i++)
    {
        levels[i] = 0;
    }
    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        levels[circuit->gates[i].outputID] = std::max(levels[circuit->gates[i].leftParentID],levels[circuit->gates[i].rightParentID])+1;
        maxLevel = std::max( levels[circuit->gates[i].outputID], maxLevel);
        
    }
return maxLevel;
}

void getCircuitbyLevels(TransformedCircuit* circuit, uint_fast64_t* levels, std::vector<TransformedGate>* circuitByLevels)
{
        for(auto i = 0; i < circuit->details.numGates;i++)
        {
            circuitByLevels[levels[circuit->gates[i].outputID]-1].push_back(circuit->gates[i]);
        }

}




void getPrevofEachWireThread(BristolCircuit* circuit, uint_fast64_t* parents, uint_fast64_t id, uint_fast64_t numThreads)
{
    for(auto i = id; i < circuit->details.numGates;i+= numThreads)
    {
        parents(circuit->gates[i].outputID,0) = circuit->gates[i].leftParentID;    
        parents(circuit->gates[i].outputID,1) = circuit->gates[i].rightParentID;  
    }    
}

void getPrevofEachWireMT(BristolCircuit* circuit, uint_fast64_t* parents, uint_fast64_t numThreads)
{
    std::thread threads[numThreads];
    for(auto i = 0; i < numThreads;i++)
        threads[i] = std::thread(getPrevofEachWireThread, circuit, parents, i, numThreads);

    for (auto i = 0; i < numThreads; i++)
        threads[i].join();
    
}

void getPrevofEachWireThreadTransformed(TransformedCircuit* circuit, uint_fast64_t* parents, uint_fast64_t id, uint_fast64_t numThreads)
{
    for(auto i = id; i < circuit->details.numGates;i+= numThreads)
    {
        parents(circuit->gates[i].outputID,0) = circuit->gates[i].leftParentID;    
        parents(circuit->gates[i].outputID,1) = circuit->gates[i].rightParentID;  
    }    
}

void getPrevofEachWireMTTransformed(TransformedCircuit* circuit, uint_fast64_t* parents, uint_fast64_t numThreads)
{
    std::thread threads[numThreads];
    for(auto i = 0; i < numThreads;i++)
        threads[i] = std::thread(getPrevofEachWireThreadTransformed, circuit, parents, i, numThreads);

    for (auto i = 0; i < numThreads; i++)
        threads[i].join();
    
}


void getCircuitLineofWireIndex(TransformedCircuit* circuit, uint_fast64_t* circuitLineOfWireIndex)
{
    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        circuitLineOfWireIndex[circuit->gates[i].outputID] = i;
    }
}

void compareCircuitSimilarity(TransformedCircuit* originalCircuit, TransformedCircuit* transformedCircuit)
{
    uint_fast64_t identicalGates = 0;
    for(auto i = 0; i < originalCircuit->details.numGates;i++)
    {
        if(originalCircuit->gates[i].truthTable[0][0] == transformedCircuit->gates[i].truthTable[0][0] && originalCircuit->gates[i].truthTable[0][1] == transformedCircuit->gates[i].truthTable[0][1] && originalCircuit->gates[i].truthTable[1][0] == transformedCircuit->gates[i].truthTable[1][0] && originalCircuit->gates[i].truthTable[1][1] == transformedCircuit->gates[i].truthTable[1][1])
            identicalGates+=1;
    } 
    std::cout << "Ratio of identical gates compared to original circuit: " << (float) identicalGates/originalCircuit->details.numGates << '\n';
}

