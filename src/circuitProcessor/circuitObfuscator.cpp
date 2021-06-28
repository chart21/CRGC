#include "include/circuitObfuscator.h"
#include "include/circuitStructs.h"
#include "include/helperFunctions.h"
#include <unordered_map>

void more_efficient_obfuscation4N(TransformedCircuit* circuit, bool* inputA)
{
    std::unordered_map<uint_fast64_t, bool> unobfuscatedValues;
    for (auto i = 0; i <circuit->details.bitlengthInputA;i++)
    {
        unobfuscatedValues[circuit->details.bitlengthInputA-1-i] =inputA[i];
    }

    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        auto leftParent = circuit->gates[i].leftParentID;
        auto rightParent = circuit->gates[i].rightParentID;
        auto outputWire = circuit->gates[i].outputID;
        

               if( unobfuscatedValues.find(leftParent) != unobfuscatedValues.end() && unobfuscatedValues.find(rightParent) != unobfuscatedValues.end() && outputWire < circuit->details.numWires - circuit->details.numOutputs*circuit->details.bitlengthOutputs)
        {
            unobfuscatedValues[outputWire] = circuit->gates[i].truthTable[unobfuscatedValues[leftParent]][unobfuscatedValues[rightParent]];
        }
        else
        {            
            if( unobfuscatedValues.find(leftParent) != unobfuscatedValues.end())
            {
               auto leftParentTrueValue = unobfuscatedValues[leftParent];
                if (circuit->gates[i].truthTable[leftParentTrueValue][0] == circuit->gates[i].truthTable[leftParentTrueValue][1] && outputWire < circuit->details.numWires - circuit->details.numOutputs*circuit->details.bitlengthOutputs)
                    unobfuscatedValues[outputWire]= circuit->gates[i].truthTable[leftParentTrueValue][0];                    
                else //recover integrity of gate
                {
                    circuit->gates[i].truthTable[! leftParentTrueValue][0] = circuit->gates[i].truthTable[leftParentTrueValue][0]; 
                    circuit->gates[i].truthTable[! leftParentTrueValue][1] = circuit->gates[i].truthTable[leftParentTrueValue][1];
                } 

            }
            if( unobfuscatedValues.find(rightParent) != unobfuscatedValues.end())
            {
               auto rightParentTrueValue = unobfuscatedValues[rightParent];
                if (circuit->gates[i].truthTable[rightParentTrueValue][0] == circuit->gates[i].truthTable[rightParentTrueValue][1] && outputWire < circuit->details.numWires - circuit->details.numOutputs*circuit->details.bitlengthOutputs)
                    unobfuscatedValues[outputWire]= circuit->gates[i].truthTable[rightParentTrueValue][0];                    
                else //recover integrity of gate
                {
                    circuit->gates[i].truthTable[! rightParentTrueValue][0] = circuit->gates[i].truthTable[rightParentTrueValue][0]; 
                    circuit->gates[i].truthTable[! rightParentTrueValue][1] = circuit->gates[i].truthTable[rightParentTrueValue][1];
                } 

            }
        }
             
    }
    //return unobfuscatedValues;
}

        

    


void more_efficient_obfuscation4NArr(TransformedCircuit* circuit, bool* inputA, bool* isObfuscated)
{
    //bool *isObfuscated = new bool[circuit->details.numWires]();
    bool *unobfuscatedValues = new bool[circuit->details.numWires];
    
    


    for (auto i = 0; i <circuit->details.bitlengthInputA;i++)
    {
       // unobfuscatedValues[circuit->details.bitlengthInputA-1-i] = inputA[i];
       // isObfuscated[circuit->details.bitlengthInputA-1-i] = true;    
       unobfuscatedValues[i] = inputA[circuit->details.bitlengthInputA-1-i];
       isObfuscated[i] = true;
    }

    for(auto i = 0; i < circuit->details.numGates;i++)
    {
        auto leftParent = circuit->gates[i].leftParentID;
        auto rightParent = circuit->gates[i].rightParentID;
        auto outputWire = circuit->gates[i].outputID;
        

        if( isObfuscated[leftParent] && isObfuscated[rightParent] && outputWire < (circuit->details.numWires - circuit->details.numOutputs*circuit->details.bitlengthOutputs))
        {
                unobfuscatedValues[outputWire] = circuit->gates[i].truthTable[unobfuscatedValues[leftParent]][unobfuscatedValues[rightParent]];
                isObfuscated[outputWire] = true;
        }
        else
        {            
            if( isObfuscated[leftParent])
            {
               auto leftParentTrueValue = unobfuscatedValues[leftParent];
                if (circuit->gates[i].truthTable[leftParentTrueValue][0] == circuit->gates[i].truthTable[leftParentTrueValue][1] && outputWire < (circuit->details.numWires - circuit->details.numOutputs*circuit->details.bitlengthOutputs))
                    {
                        unobfuscatedValues[outputWire]= circuit->gates[i].truthTable[leftParentTrueValue][0]; 
                        isObfuscated[outputWire] = true;   
                    }
                else //recover integrity of gate
                {
                   circuit->gates[i].truthTable[not leftParentTrueValue][0] = circuit->gates[i].truthTable[leftParentTrueValue][0]; 
                   circuit->gates[i].truthTable[not leftParentTrueValue][1] = circuit->gates[i].truthTable[leftParentTrueValue][1];
                } 

            }
            if( isObfuscated[rightParent])
            {
               auto rightParentTrueValue = unobfuscatedValues[rightParent];
                if (circuit->gates[i].truthTable[0][rightParentTrueValue] == circuit->gates[i].truthTable[1][rightParentTrueValue] && (outputWire < circuit->details.numWires - circuit->details.numOutputs*circuit->details.bitlengthOutputs))
                    {
                        unobfuscatedValues[outputWire]= circuit->gates[i].truthTable[0][rightParentTrueValue];
                        isObfuscated[outputWire] = true;                    
                    }
                else //recover integrity of gate
                {
                    circuit->gates[i].truthTable[0][not rightParentTrueValue] = circuit->gates[i].truthTable[0][rightParentTrueValue];
                    circuit->gates[i].truthTable[1][not rightParentTrueValue] = circuit->gates[i].truthTable[1][rightParentTrueValue];
                } 

            }
        }
             
    }
    delete[] unobfuscatedValues;
}