#include "include/circuitTransformer.h"
#include "include/circuitStructs.h"
#include "include/helperFunctions.h"
#include <iostream>

UintCircuit* transformTransformedCircuitToUint(TransformedCircuit* circuit)
{
    auto gates = new UintGate[circuit->details.numGates];
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
    auto uintCircuit = new UintCircuit({circuit->details, gates});
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
    
    *circuit = {bristolCircuit->details,gates};


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
    
    auto circuit = new TransformedCircuit({bristolCircuit->details,gates});
    return circuit;

}