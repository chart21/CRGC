#include "include/circuitStructs.h"
#include "include/circuitEvaluator.h"
#include <iostream>
#define output(i,j)   output[(i)*(*circuit).details.bitlengthOutputs + (j)] //making 2D array index more natural

bool* evaluateTransformedCircuitN(TransformedCircuit *circuit, bool* inputA, bool* inputB)
{
    bool evaluation[(*circuit).details.numWires]; 
    for (int i = 0; i < (*circuit).details.numGates; i++)
    {
       evaluation[(*circuit).gates[i].outputID] = (*circuit).gates[i].truthTable [evaluation[(*circuit).gates[i].leftParentID]] [evaluation[(*circuit).gates[i].rightParentID] ];  
    }
    bool* output = new bool[(*circuit).details.numOutputs * (*circuit).details.bitlengthOutputs];
    
    for (int i = 0; i < (*circuit).details.numOutputs; i++)
    {
        for (int j = 0; j < (*circuit).details.bitlengthOutputs; j++)
        {
            output(i,j) = evaluation[(*circuit).details.numWires-1-j-(*circuit).details.bitlengthOutputs*i];
            std::cout << output(i,j);
        }        
    }
    std::cout << std::endl;
    return output;
} 