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

void getFlippedCircuit(TransformedCircuit* circuit, bool* flipped)
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



