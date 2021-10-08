#include "include/circuitTransformer.h"
#include "include/circuitStructs.h"
#include "include/helperFunctions.h"
#include <iostream>
#include <unordered_set>

UintCircuit* transformTransformedCircuitToUint(TransformedCircuit* circuit)
{
    auto uintCircuit = new UintCircuit(circuit->details);
    UintGate* gates = uintCircuit->gates;
    //auto gates = new UintGate[circuit->details.numGates];
    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        uint_fast8_t truthTable = 0;
        truthTable+= circuit->gates[i].truthTable[0][0] << 3;
        truthTable+= circuit->gates[i].truthTable[0][1] << 2;
        truthTable+= circuit->gates[i].truthTable[1][0] << 1;
        truthTable+= circuit->gates[i].truthTable[1][1];
        
        gates[i] = UintGate{circuit->gates[i].leftParentID, circuit->gates[i].rightParentID,circuit->gates[i].outputID, truthTable};
        //std::cout <<  circuit->gates[i].truthTable[0][0] << circuit->gates[i].truthTable[0][1] << circuit->gates[i].truthTable[1][0] << circuit->gates[i].truthTable[1][1] << ' ' << (unsigned)gates[i].truthTable << '\n';
    }
    //auto uintCircuit = new UintCircuit({circuit->details, gates});
    return uintCircuit;
}


void transformBristolCircuitToTransformedCircuit(BristolCircuit *bristolCircuit, TransformedCircuit *circuit, bool* flipped)
{

    auto gates = new TransformedGate [bristolCircuit->details.numGates];
    for (auto i = 0; i < bristolCircuit->details.numGates; i++)
    {
        bool truthTable[2][2];
        if (bristolCircuit->gates[i].truthTable == 'X')
            bool truthTable[2][2] = {{0, 1}, {1, 0}};
        else if (bristolCircuit->gates[i].truthTable == 'A')
        {
            bool truthTable[2][2] = {{0, 0}, {0, 1}};
        }
        else if (bristolCircuit->gates[i].truthTable == 'O')
        {
            bool truthTable[2][2] = {{0, 1}, {1, 1}};
        }
        
        if (flipped[bristolCircuit->gates[i].leftParentID] == true)
            swapLeftParent(truthTable);
        if (flipped[bristolCircuit->gates[i].rightParentID] == true)
            swapRightParent(truthTable);
        

        gates[i] = TransformedGate{bristolCircuit->gates[i].leftParentID, bristolCircuit->gates[i].rightParentID, bristolCircuit->gates[i].outputID, {{truthTable[0][0], truthTable[0][1]}, {truthTable[1][0], truthTable[1][1]}}};
        //std::cout << gates[i].leftParentID <<' ' <<  gates[i].rightParentID <<' ' << gates[i].outputID <<' ' << gates[i].truthTable[0][0] << gates[i].truthTable[0][1] << gates[i].truthTable[1][0]<< gates[i].truthTable[1][1] <<'\n';
    }
    
    //*circuit = {bristolCircuit->details,gates};
    circuit->details = bristolCircuit->details;
    circuit->gates = gates;


}


TransformedCircuit* transformBristolCircuitToTransformedCircuitN(BristolCircuit *bristolCircuit)
{
        
    auto gates = new TransformedGate [bristolCircuit->details.numGates];
    for (auto i = 0; i < bristolCircuit->details.numGates; i++)
    {
        bool truthTable[2][2];
        if (bristolCircuit->gates[i].truthTable == 'X')
            bool truthTable[2][2] = {{0, 1}, {1, 0}};
        else if (bristolCircuit->gates[i].truthTable == 'A')
        {
            bool truthTable[2][2] = {{0, 0}, {0, 1}};
        }
        else if (bristolCircuit->gates[i].truthTable == 'O')
        {
            bool truthTable[2][2] = {{0, 1}, {1, 1}};
        }
        
        // if (flipped[bristolCircuit->gates[i].leftParentID] == true)
        //     swapLeftParent(truthTable);
        // if (flipped[bristolCircuit->gates[i].rightParentID] == true)
        //     swapRightParent(truthTable);
        

        gates[i] = TransformedGate{bristolCircuit->gates[i].leftParentID, bristolCircuit->gates[i].rightParentID, bristolCircuit->gates[i].outputID, {{truthTable[0][0], truthTable[0][1]}, {truthTable[1][0], truthTable[1][1]}}};
        //std::cout << gates[i].leftParentID <<' ' <<  gates[i].rightParentID <<' ' << gates[i].outputID <<' ' << gates[i].truthTable[0][0] << gates[i].truthTable[0][1] << gates[i].truthTable[1][0]<< gates[i].truthTable[1][1] <<'\n';
    }
    
    // auto circuit = new TransformedCircuit({bristolCircuit->details,gates});
    auto circuit = new TransformedCircuit();
    circuit->details = bristolCircuit->details;
    circuit->gates = gates;
    return circuit;

}

void deleteRevealGates(TransformedCircuit *circuit, uint_fast64_t* circuitLineOfWireIndex)
{

    //std::queue<uint_fast64_t> deleteIndexQueue; //cant use a queue for this purpose -> map (elements are not in order)
    std::unordered_set<uint_fast64_t> deleteSet;
    uint_fast64_t deleteCounter = 0;
    for ( auto i = 0; i < circuit->details.numOutputs; i++)
    {
        for (auto j = 0; j < circuit->details.bitlengthOutputs; j++)
        {
            auto gateID = circuit->gates[circuit->details.numGates - (i+1)*j - 1].rightParentID;
            auto gateIndex = circuitLineOfWireIndex[gateID];
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].leftParentID = circuit->gates[gateIndex].leftParentID;
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].rightParentID = circuit->gates[gateIndex].rightParentID;
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].outputID = circuit->gates[gateIndex].outputID;
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].truthTable[0][0] = circuit->gates[gateIndex].truthTable[0][0];
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].truthTable[0][1] = circuit->gates[gateIndex].truthTable[0][1];
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].truthTable[1][0] = circuit->gates[gateIndex].truthTable[1][0];
            circuit->gates[circuit->details.numGates - (i+1)*j - 2].truthTable[1][1] = circuit->gates[gateIndex].truthTable[1][1];

            deleteSet.insert(gateIndex);
        }
    }

    
    for ( auto i = 0; i < circuit->details.numGates- (circuit->details.bitlengthOutputs*circuit->details.numOutputs)*2 - 1; i++)
    {
        if(deleteSet.find(i) == deleteSet.end())
        {
            
            
            circuit->gates[circuit->details.numGates- (circuit->details.bitlengthOutputs*circuit->details.numOutputs)*2 - 1 + deleteCounter] = circuit->gates[i];
            circuit->gates[i] = circuit->gates[i + deleteCounter +1];
            deleteCounter++;
            
        }
        else
        {  
            circuit->gates[i] = circuit->gates[i+ deleteCounter + 1];
        }        
    } 
    

        //the 0 0 XOR gate is no longer needed and the previous last gates are no longer needed
    circuit->details.numGates -= circuit->details.bitlengthOutputs*circuit->details.numOutputs - 1;
    circuit->details.numWires -= circuit->details.bitlengthOutputs*circuit->details.numOutputs - 1;;
    
}

ShrinkedCircuit* transformCircuitToShrinkedCircuit(TransformedCircuit* circuit)
{
    auto shrinkedCircuit = new ShrinkedCircuit(circuit->details);
    auto shrinkedGates = shrinkedCircuit->gates;
    //auto shrinkedGates = new ShrinkedGate[circuit->details.numGates];
    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        if(circuit->gates[i].leftParentID <= circuit->gates[i].rightParentID)
            shrinkedGates[i] = {circuit->gates[i].leftParentID, circuit->gates[i].rightParentID - circuit->gates[i].leftParentID, circuit->gates[i].truthTable[0][0], circuit->gates[i].truthTable[0][1], circuit->gates[i].truthTable[1][0], circuit->gates[i].truthTable[1][1]};
        else
        {
            //mirror truthTable
            auto temp = circuit->gates[i].truthTable[0][1];
            circuit->gates[i].truthTable[0][1] = circuit->gates[i].truthTable[1][0];
            circuit->gates[i].truthTable[1][0] = temp;

            shrinkedGates[i] = {circuit->gates[i].rightParentID, circuit->gates[i].leftParentID - circuit->gates[i].rightParentID, circuit->gates[i].truthTable[0][0], circuit->gates[i].truthTable[0][1], circuit->gates[i].truthTable[1][0], circuit->gates[i].truthTable[1][1]};
        
            

        }
    }
    // auto shrinkedCircuit = new ShrinkedCircuit{circuit->details, shrinkedGates};
    //*shrinkedCircuit = {circuit->details, shrinkedGates};
    return shrinkedCircuit;
}

TransformedCircuit* transformShrinkedCircuitToTransformedCircuit(ShrinkedCircuit* circuit)
{
    auto transformedCircuit = new TransformedCircuit(circuit->details);
    auto transformedGates = transformedCircuit->gates;
    //auto shrinkedGates = new ShrinkedGate[circuit->details.numGates];
    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        transformedGates[i] = {circuit->gates[i].leftParentID, 
                                circuit->gates[i].rightParentOffset + circuit->gates[i].leftParentID, 
                                circuit->details.bitlengthInputA + circuit->details.bitlengthInputB + i, 
                                circuit->gates[i].truthTable[0][0], circuit->gates[i].truthTable[0][1], 
                                circuit->gates[i].truthTable[1][0], circuit->gates[i].truthTable[1][1]};

    }
    // auto shrinkedCircuit = new ShrinkedCircuit{circuit->details, shrinkedGates};
    //*shrinkedCircuit = {circuit->details, shrinkedGates};
    return transformedCircuit;
}