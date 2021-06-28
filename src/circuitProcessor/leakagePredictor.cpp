#include "include/leakagePredictor.h"
#include "include/circuitStructs.h"
#include <vector>
#include <queue>
#include <thread>

#define parents(i,j)   parents[(i)*2 + (j)] //making 2D array index more natural

void getPotentiallyObfuscatedGates(BristolCircuit* circuit, bool* po)
{
    //auto po = new bool[circuit->details.numWires];
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
        if(circuit->gates[i].truthTable == 'X')        
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];        
        else
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID];
    }    
}

void getPotentiallyIntegrityBreakingGates(CircuitDetails details, bool* po, uint_fast64_t* parents, std::vector<uint_fast64_t>* successors)
{
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numWires; i++)
    {
        if(po[i] )
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
                        bool allSuccessorsObfuscated = true;
                        for(auto s : successors[parents(currIndex,j)])
                        {
                            if(! po[s] and s != 0)
                            {
                                allSuccessorsObfuscated = false;
                            }
                            if(allSuccessorsObfuscated)
                            {
                                po[parents(currIndex,j)] = true;
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


void getPotentiallyIntegrityBreakingGatesFromOutput(CircuitDetails details, bool* po, uint_fast64_t* parents)
{
    auto npib = new bool[details.numWires]();
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
                 npib[currIndex] = true;
                 for (auto p = 0; p < 2; p++)
                 {
                    if(parents(currIndex,p) >= details.bitlengthInputA+details.bitlengthInputB && po[parents(currIndex,p)]  && not addedGates[parents(currIndex,p)])
                    {
                        addedGates[parents(currIndex,p)] = true;
                        pathQueue.push(parents(currIndex,p));                        
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


void getPotentiallyIntegrityBreakingGatesFromOutputThread(CircuitDetails details, bool* po, uint_fast64_t* parents, bool* npib, bool* addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread)
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
                 npib[currIndex] = true;
                 for (auto p = 0; p < 2; p++)
                 {
                    if(parents(currIndex,p) >= details.bitlengthInputA+details.bitlengthInputB && po[parents(currIndex,p)]  && not addedGates[parents(currIndex,p)])
                    {
                        addedGates[parents(currIndex,p)] = true;
                        pathQueue.push(parents(currIndex,p));                        
                    }
                 }
             }
        }
    }
}





void getPotentiallyIntegrityBreakingGatesFromOutputMT(CircuitDetails details, bool* po, uint_fast64_t* parents)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();
    uint_fast64_t numThreads = 7;
    uint_fast64_t numGates = details.bitlengthOutputs/numThreads;
    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i <numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyIntegrityBreakingGatesFromOutputThread,details,po, parents, npib, addedGates,i,numThreads); 
    }
    for (auto i = 0; i <numThreads; i++)
    {
        threads[i].join();
    }

        
    

    for (auto i = 0; i < details.numWires; i++)
        po[i] = !npib[i];
    delete[] addedGates; 
    delete[] npib;
}
