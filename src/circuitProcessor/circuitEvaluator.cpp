#include "include/circuitEvaluator.h"
#include "include/circuitStructs.h"
#include <iostream>
#include <semaphore>
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


void evaluateTransformedCircuitMT(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output, uint_fast64_t numThreads, uint_fast64_t timeSleep)
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

    bool* evaluated = new bool[circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs]();
    // bool* evaluated = new bool[circuit->details.numWires]();
    // std::fill_n(evaluated, circuit->details.bitlengthInputA+circuit->details.bitlengthInputB, true);

    

    //std::condition_variable cond;

    
    std::thread threads[numThreads];
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(evaluateTransformedCircuitThread, circuit, evaluation, i, numThreads, evaluated, timeSleep);
    }
    for (auto i = 0; i < numThreads; i++)
        threads[i].join();

    delete[] evaluated;
    
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

void evaluateTransformedCircuitThread(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated, uint_fast64_t timeSleep)
{    
    auto reducer = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB;
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
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
        evaluation[circuit->gates[i].outputID] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
        if(circuit->gates[i].outputID < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
            evaluated[circuit->gates[i].outputID - reducer] = true;
    }
}





















void evaluateTransformedCircuitThreadHackBool2(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, bool* evaluated)
{
    using namespace std::chrono_literals;
    
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
    {

            while(not evaluated[circuit->gates[i].rightParentID])
            {
                std::this_thread::sleep_for(300ns);
            }
   
       
            while(not evaluated[circuit->gates[i].leftParentID])
            {
                std::this_thread::sleep_for(300ns);
           }
                  
        evaluation[circuit->gates[i].outputID] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
        if(circuit->gates[i].outputID < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
            evaluated[circuit->gates[i].outputID] = true;
    }
}
























void evaluateTransformedCircuitHackMTCond(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output)
{
    auto evaluation = new bool[circuit->details.numWires];
    //auto conditions = new std::condition_variable[circuit->details.numGates]();
    //auto semaphores = new std::counting_semaphore<1>[circuit->details.numGates](0);
    //std::counting_semaphore<1> semaphores[circuit->details.numGates];
    int size = 1;
    //std::vector <std::counting_semaphore<1>> semaphores( circuit->details.numGates - circuit->details.numWires-circuit->details.bitlengthOutputs*circuit->details.numOutputs);
    //std::counting_semaphore<1> semaphores(1);
    //std::vector <std::counting_semaphore<1>> semaphores;
    //std::vector <std::binary_semaphore*> semaphores;

    std::binary_semaphore **semaphores = new std::binary_semaphore *[circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs];
    auto aquired = new bool[circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs]();
    for (auto i = 0; i < circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs; i++)
        semaphores[i] = new std::binary_semaphore(0);


    //auto semaphore = new std::binary_semaphore(1);

    //semaphores.reserve(circuit->details.numGates - circuit->details.bitlengthOutputs*circuit->details.numOutputs);
    //std::binary_semaphore semaphoros[1](std::binary_semaphore(1));

    //      for (auto i = 0; i < circuit->details.numGates - circuit->details.bitlengthOutputs*circuit->details.numOutputs; i++)
    //   {
    //      //std::counting_semaphore<1> semaphore(1);
    //     std::binary_semaphore semaphoros(1);
    //     semaphores.push_back(&semaphoros);
    //   }

    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        evaluation[i] = inputA[circuit->details.bitlengthInputA - 1 - i];
    }

    for (auto i = 0; i < circuit->details.bitlengthInputB; i++)
    {
        evaluation[i + circuit->details.bitlengthInputA] = inputB[circuit->details.bitlengthInputB - 1 - i];
    }

    uint_fast64_t numThreads = 7;
    std::thread threads[numThreads];
    for (auto i = 0; i < numThreads; i++)
    {
        //threads[i] = std::thread(evaluateTransformedCircuitbyLevelThreadHackCond, circuit, evaluation, i, numThreads, &semaphores);
        threads[i] = std::thread(evaluateTransformedCircuitbyLevelThreadHackCond, circuit, evaluation, i, numThreads, semaphores, aquired);
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

void evaluateTransformedCircuitbyLevelThreadHackCond(TransformedCircuit *circuit, bool *evaluation, uint_fast64_t id, uint_fast64_t numThreads, std::binary_semaphore **semaphores, bool* aquired)
{
    auto reducer = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB;
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
    {
        
        //conditions[circuit->gates[i].rightParentID].wait_for();
        if (circuit->gates[i].rightParentID >= reducer && not aquired[circuit->gates[i].rightParentID])
        {
            semaphores[circuit->gates[i].rightParentID - reducer]->acquire();
            semaphores[circuit->gates[i].rightParentID - reducer]->release();
        }
        if (circuit->gates[i].leftParentID >= reducer && not aquired[circuit->gates[i].rightParentID])
        {
            semaphores[circuit->gates[i].leftParentID - reducer]->acquire();
            semaphores[circuit->gates[i].leftParentID - reducer]->release();
        }

        evaluation[circuit->gates[i].outputID] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
        if (circuit->gates[i].outputID < circuit->details.numGates - circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
        {
            if(circuit->gates[i].outputID < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
            {
                semaphores[circuit->gates[i].outputID - reducer]->release();
                aquired[circuit->gates[i].outputID - reducer] = true;
            }
        }
    }
}




void evaluateTransformedCircuitbyLevel(TransformedCircuit *circuit, std::vector<TransformedGate> *circuitByLine, uint_fast64_t maxLevel, bool *inputA, bool *inputB, bool *output)
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
    for (auto s = 0; s < maxLevel; s++)
    {
        for (auto i = 0; i < circuitByLine[s].size(); i++)
        {
            evaluation[circuitByLine[s][i].outputID] = circuitByLine[s][i].truthTable[evaluation[circuitByLine[s][i].leftParentID]][evaluation[circuitByLine[s][i].rightParentID]];
        }
    }

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

void evaluateTransformedCircuitbyLevelMT(TransformedCircuit *circuit, std::vector<TransformedGate> *circuitByLine, uint_fast64_t maxLevel, bool *inputA, bool *inputB, bool *output)
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
    //uint_fast64_t numThreads = 4;

    //std::thread threads[numThreads];
    //for (auto i = 0; i < numThreads; i++)
    //    threads[i] = std::thread(evaluateTransformedCircuitbyLevelThread, circuitByLine, maxLevel, evaluation, i, numThreads);
    //for (auto i = 0; i < numThreads; i++)
    //    threads[i].join();

    for (auto s = 0; s < maxLevel; s++)
    {
        uint_fast64_t numThreads = 1;
        //uint_fast64_t numThreads = circuitByLine[s].size();
        std::thread threads[numThreads];
        for (auto i = 0; i < numThreads; i++)
        {
            threads[i] = std::thread(evaluateTransformedCircuitbyLevelThread, circuitByLine, evaluation, s, i, (circuitByLine[s].size() + numThreads - 1) / numThreads);
        }
        for (auto i = 0; i < numThreads; i++)
            threads[i].join();
    }

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

void evaluateTransformedCircuitbyLevelThread(std::vector<TransformedGate> *circuitByLine, bool *evaluation, uint_fast64_t s, uint_fast64_t index, uint_fast64_t numGates)
{
    for (auto i = index; i < index + numGates; i++)
    {
        evaluation[circuitByLine[s][i].outputID] = circuitByLine[s][i].truthTable[evaluation[circuitByLine[s][i].leftParentID]][evaluation[circuitByLine[s][i].rightParentID]];
    }
}


void evaluateTransformedCircuitHackMTUint(TransformedCircuit *circuit, bool *inputA, bool *inputB, bool *output)
{
    auto evaluation = new uint_fast8_t[circuit->details.numWires];

    for (auto i = 0; i < circuit->details.bitlengthInputA; i++)
    {
        evaluation[i] = inputA[circuit->details.bitlengthInputA - 1 - i];
    }

    for (auto i = 0; i < circuit->details.bitlengthInputB; i++)
    {
        evaluation[i + circuit->details.bitlengthInputA] = inputB[circuit->details.bitlengthInputB - 1 - i];
    }

    for (auto i = circuit->details.bitlengthInputB + circuit->details.bitlengthInputA; i < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs; i++)
    {
        evaluation[i] = 2;
    }

    //std::condition_variable cond;

    uint_fast64_t numThreads = 7;
    std::thread threads[numThreads];
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(evaluateTransformedCircuitbyLevelThreadHackUint, circuit, evaluation, i, numThreads);
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

void evaluateTransformedCircuitbyLevelThreadHackUint(TransformedCircuit *circuit, uint_fast8_t *evaluation, uint_fast64_t id, uint_fast64_t numThreads)
{
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
    {
        while (evaluation[circuit->gates[i].rightParentID > 2] || evaluation[circuit->gates[i].rightParentID > 2])
        {
        }
        evaluation[circuit->gates[i].outputID] = circuit->gates[i].truthTable[evaluation[circuit->gates[i].leftParentID]][evaluation[circuit->gates[i].rightParentID]];
    }
}