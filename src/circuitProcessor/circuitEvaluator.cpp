#include "include/circuitEvaluator.h"
#include "include/circuitStructs.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

//#include <condition_variable>

#define output(i, j) output[(i)*circuit->details.bitlengthOutputs + (j)] //making 2D array index more natural

void evaluateTransformedCircuit(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output)
{
    auto evaluation = new bool[circuit->details.numWires];

    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        evaluation[i] = inputA[circuit->details.bitlengthInputA - 1 - i];
    }

    for (auto i = 0; i < circuit->details.bitlengthInputB; i++)
    {
        evaluation[i + circuit->details.bitlengthInputA] = inputB[circuit->details.bitlengthInputB - 1 - i];
    }

    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        //std::cout << circuit->gates[i].leftParentID <<' ' <<  circuit->gates[i].rightParentID <<' ' << circuit->gates[i].outputID <<' ' << circuit->gates[i].truthTable[0][0] << circuit->gates[i].truthTable[0][1] << circuit->gates[i].truthTable[1][0] << circuit->gates[i].truthTable[1][1] <<'\n';
        evaluation[circuit->gates[i].outputID] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
    }
    //bool* output = new bool[circuit->details.numOutputs * circuit->details.bitlengthOutputs];

    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        for (auto j = 0; j < circuit->details.bitlengthOutputs; j++)
        {
            output(i, j) = evaluation[circuit->details.numWires - 1 - j - circuit->details.bitlengthOutputs * i];
            //std::cout << output(i,j);
        }
        //std::cout << std::endl;
    }

    delete[] evaluation;
}

void evaluateSortedTransformedCircuit(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output)
{
    auto evaluation = new bool[circuit->details.numWires];

    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        evaluation[i] = inputA[circuit->details.bitlengthInputA - 1 - i];
    }

    for (auto i = 0; i < circuit->details.bitlengthInputB; i++)
    {
        evaluation[i + circuit->details.bitlengthInputA] = inputB[circuit->details.bitlengthInputB - 1 - i];
    }

    for (auto i = 0; i < circuit->details.numGates; i++)
    {
        //std::cout << circuit->gates[i].leftParentID <<' ' <<  circuit->gates[i].rightParentID <<' ' << circuit->gates[i].outputID <<' ' << circuit->gates[i].truthTable[0][0] << circuit->gates[i].truthTable[0][1] << circuit->gates[i].truthTable[1][0] << circuit->gates[i].truthTable[1][1] <<'\n';
        evaluation[i+circuit->details.bitlengthInputA+circuit->details.bitlengthInputB] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
    }
    //bool* output = new bool[circuit->details.numOutputs * circuit->details.bitlengthOutputs];

    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        for (auto j = 0; j < circuit->details.bitlengthOutputs; j++)
        {
            output(i, j) = evaluation[circuit->details.numWires - 1 - j - circuit->details.bitlengthOutputs * i];
            //std::cout << output(i,j);
        }
        //std::cout << std::endl;
    }

    delete[] evaluation;
}

void evaluateTransformedCircuitHackMT(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output, uint_fast64_t numThreads)
{
    auto evaluation = new bool[circuit->details.numWires];

    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        evaluation[i] = inputA[circuit->details.bitlengthInputA - 1 - i];
    }

    for (auto i = 0; i < circuit->details.bitlengthInputB; i++)
    {
        evaluation[i + circuit->details.bitlengthInputA] = inputB[circuit->details.bitlengthInputB - 1 - i];
    }

    std::thread threads[numThreads];
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(evaluateTransformedCircuitHackThread, circuit, evaluation, i, numThreads);
    }
    for (auto i = 0; i < numThreads; i++)
        threads[i].join();

    for (auto i = 0; i < circuit->details.numOutputs; i++)
    {
        for (auto j = 0; j < circuit->details.bitlengthOutputs; j++)
        {
            output(i, j) = evaluation[circuit->details.numWires - 1 - j - circuit->details.bitlengthOutputs * i];
            //std::cout << output(i,j);
        }
        //std::cout << std::endl;
    }
    delete[] evaluation;
}

void evaluateTransformedCircuitHackThread(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads)
{
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
    {
        evaluation[circuit->gates[i].outputID] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
    }
}




