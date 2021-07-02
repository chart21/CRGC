#include "include/circuitFlipper.h"
#include "include/circuitStructs.h"
#include "include/randomBoolGenerator.h"
#include "include/helperFunctions.h"
#include <thread>
#include <chrono>


void obfuscateInput(bool* inputA, bool* obfuscatedValArr, bool* flipped,  CircuitDetails details)
{
    auto counter = details.bitlengthInputA-1;
    sfc64 sfc;
    RandomizerWithSentinelShift<> randomizer;
    for (auto i = 0; i <details.bitlengthInputA; i++)
    {        
        obfuscatedValArr[i] = randomizer(sfc);
        flipped[counter] = obfuscatedValArr[i] != inputA[i];
        //flipped[counter] = randomizer(sfc) != inputA[i];
        //obfuscatedValArr[i] ^ flipped[counter];
        counter--;     
    }
    for (auto i = details.bitlengthInputA; i <details.bitlengthInputA+details.bitlengthInputB; i++)
    {        
        flipped[i] = 0;
    }   

}

void getFlippedCircuitWithoutOutputsN(TransformedCircuit* circuit, bool* flipped)
{
    sfc64 sfc;
    RandomizerWithSentinelShift<> randomizer;

    for (auto i = 0; i < circuit->details.numGates; i++)   
    {
        //recover integrity
        if(flipped[circuit->gates[i].leftParentID])        
            swapLeftParent(circuit->gates[i].truthTable);
        
        if(flipped[circuit->gates[i].rightParentID])
            swapRightParent(circuit->gates[i].truthTable);


        //flip all bits
        if(randomizer(sfc) == 1 && circuit->gates[i].outputID < circuit->details.numOutputs*circuit->details.bitlengthOutputs) 
        {
            flipTable(circuit->gates[i].truthTable);
            flipped[circuit->gates[i].outputID] = 1;
        }
        else
            flipped[circuit->gates[i].outputID] = 0;      
    }

}


void getFlippedCircuitWithoutOutputsMT(TransformedCircuit* circuit, bool* flipped, uint_fast64_t numThreads, uint_fast64_t timeSleep)
{

    bool* evaluated = new bool[circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs]();
    std::thread threads[numThreads];
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getFlippedCircuitWithoutOutputsThread, circuit, flipped, i, numThreads, evaluated, timeSleep);
    }
    for (auto i = 0; i < numThreads; i++)
        threads[i].join();

    delete[] evaluated;



}


void getFlippedCircuitWithoutOutputsThread(TransformedCircuit* circuit, bool* flipped, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated, uint_fast64_t timeSleep)
{
    sfc64 sfc;
    RandomizerWithSentinelShift<> randomizer;
    auto reducer = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB;
    for (auto i = id; i < circuit->details.numGates; i+= numThreads)   
    {

        if(circuit->gates[i].rightParentID >= reducer)
        {
            while(not evaluated[circuit->gates[i].rightParentID - reducer])
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(timeSleep));
            }
        }
        if(circuit->gates[i].leftParentID >= reducer)
        {
            while(not evaluated[circuit->gates[i].leftParentID - reducer])
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(timeSleep));
           }
        }   



        //recover integrity
        if(flipped[circuit->gates[i].leftParentID])        
            swapLeftParent(circuit->gates[i].truthTable);
        
        if(flipped[circuit->gates[i].rightParentID])
            swapRightParent(circuit->gates[i].truthTable);


        //flip all bits
        if(randomizer(sfc) == 1 && circuit->gates[i].outputID < circuit->details.numOutputs*circuit->details.bitlengthOutputs) 
        {
            flipTable(circuit->gates[i].truthTable);
            flipped[circuit->gates[i].outputID] = 1;
        }
        else
            flipped[circuit->gates[i].outputID] = 0; 
        if(circuit->gates[i].outputID < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
            evaluated[circuit->gates[i].outputID - reducer] = true;
    }     
}
