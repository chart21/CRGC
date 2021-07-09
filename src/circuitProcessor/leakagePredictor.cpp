#include "include/leakagePredictor.h"
#include "include/circuitStructs.h"
#include <vector>
#include <queue>
#include <thread>
#include <chrono>

#define parents(i, j) parents[(i)*2 + (j)] //making 2D array index more natural

void getPotentiallyObfuscatedGates(BristolCircuit *circuit, bool *po)
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
        if (circuit->gates[i].truthTable == 'X')
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];
        else
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID];
    }
}

void getPotentiallyObfuscatedGatesT(TransformedCircuit *circuit, bool *po)
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
        uint_fast8_t amountOnes = circuit->gates[i].truthTable[0][0] + circuit->gates[i].truthTable[0][1] + circuit->gates[i].truthTable[1][0] + circuit->gates[i].truthTable[1][1];
        switch (amountOnes)
        {
        case 1:
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID]; //AND or NOR
            break;
        case 3:
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID]; //NAND or OR
            break;
        case 2:
            if (circuit->gates[i].truthTable[0][1] == circuit->gates[i].truthTable[1][0]) //XOR or XNOR
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];
            }
            else if (circuit->gates[i].truthTable[0][0] == circuit->gates[i].truthTable[0][1]) // 0011 or 1100
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID];
            }
            else // 0101 or 1010
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].rightParentID];
            }

            break;

        default:
            po[circuit->gates[i].outputID] = true;
            break;
        }
    }
}

void getPotentiallyIntegrityBreakingGates(CircuitDetails details, bool *po, uint_fast64_t *parents, std::vector<uint_fast64_t> *successors)
{
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numWires; i++)
    {
        if (po[i])
        {
            pathQueue.push(i);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                for (auto j = 0; j < 2; j++)
                {
                    if (!addedGates[parents(currIndex, j)])
                    {
                        addedGates[parents(currIndex, j)] = true;
                        bool allSuccessorsObfuscated = true;
                        for (auto s : successors[parents(currIndex, j)])
                        {
                            if (!po[s] and s != 0)
                            {
                                allSuccessorsObfuscated = false;
                            }
                            if (allSuccessorsObfuscated)
                            {
                                po[parents(currIndex, j)] = true;
                                pathQueue.push(parents(currIndex, j));
                            }
                        }
                    }
                }
            }
        }
    }
    delete[] addedGates;
}

void getPotentiallyIntegrityBreakingGatesFromOutput(CircuitDetails details, bool *po, uint_fast64_t *parents)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for (auto j = 0; j < details.bitlengthOutputs; j++)
        {
            pathQueue.push(details.numWires - 1 - i - j);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                npib[currIndex] = true;
                for (auto p = 0; p < 2; p++)
                {
                    if (parents(currIndex, p) >= details.bitlengthInputA + details.bitlengthInputB && po[parents(currIndex, p)] && not addedGates[parents(currIndex, p)])
                    {
                        addedGates[parents(currIndex, p)] = true;
                        pathQueue.push(parents(currIndex, p));
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

void getPotentiallyIntegrityBreakingGatesFromOutputThread(CircuitDetails details, bool *po, uint_fast64_t *parents, bool *npib, bool *addedGates, uint_fast64_t id, uint_fast64_t amountGatesperThread)
{
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for (auto j = amountGatesperThread * id; j < std::min(amountGatesperThread * (id + 1), details.bitlengthOutputs); j++)
        {
            pathQueue.push(details.numWires - 1 - i - j);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                npib[currIndex] = true;
                for (auto p = 0; p < 2; p++)
                {
                    if (parents(currIndex, p) >= details.bitlengthInputA + details.bitlengthInputB && po[parents(currIndex, p)] && not addedGates[parents(currIndex, p)])
                    {
                        addedGates[parents(currIndex, p)] = true;
                        pathQueue.push(parents(currIndex, p));
                    }
                }
            }
        }
    }
}

void getPotentiallyIntegrityBreakingGatesFromOutputMT(CircuitDetails details, bool *po, uint_fast64_t *parents, uint_fast64_t numThreads)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();
    //uint_fast64_t numGates = details.bitlengthOutputs / numThreads + 1;
    uint_fast64_t numGates = details.bitlengthOutputs / numThreads;
    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyIntegrityBreakingGatesFromOutputThread, details, po, parents, npib, addedGates, i, numGates);
    }
    getPotentiallyIntegrityBreakingGatesFromOutputThread(details, po, parents, npib, addedGates, numThreads, numGates); //process leftover gates, remainder
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    for (auto i = 0; i < details.numWires; i++)
        po[i] = !npib[i];
    delete[] addedGates;
    delete[] npib;
}

void getPotentiallyObfuscatedGatesMT(BristolCircuit *circuit, bool *po, uint_fast64_t numThreads, uint_fast64_t sleepTime)
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

    std::thread threads[numThreads];
    bool *evaluated = new bool[circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs]();

    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyObfuscatedGatesThread, circuit, po, i, numThreads, evaluated, sleepTime);
    }
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }
    delete[] evaluated;
}

void getPotentiallyObfuscatedGatesMTT(TransformedCircuit *circuit, bool *po, uint_fast64_t numThreads, uint_fast64_t sleepTime)
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

    std::thread threads[numThreads];
    bool *evaluated = new bool[circuit->details.numGates - circuit->details.bitlengthOutputs * circuit->details.numOutputs]();

    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyObfuscatedGatesThreadT, circuit, po, i, numThreads, evaluated, sleepTime);
    }
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }
    delete[] evaluated;
}

void getPotentiallyObfuscatedGatesThreadT(TransformedCircuit *circuit, bool *po, uint_fast64_t id, uint_fast64_t numThreads, bool *evaluated, uint_fast64_t sleepTime)
{
    using namespace std::chrono_literals;
    auto reducer = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB;
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
    {
        if (circuit->gates[i].rightParentID >= reducer)
        {
            while (not evaluated[circuit->gates[i].rightParentID - reducer])
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(sleepTime));
            }
        }
        if (circuit->gates[i].leftParentID >= reducer)
        {
            while (not evaluated[circuit->gates[i].leftParentID - reducer])
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(sleepTime));
            }
        }

        uint_fast8_t amountOnes = circuit->gates[i].truthTable[0][0] + circuit->gates[i].truthTable[0][1] + circuit->gates[i].truthTable[1][0] + circuit->gates[i].truthTable[1][1];
        switch (amountOnes)
        {
        case 1:
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID]; //AND or NOR
            break;
        case 3:
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID]; //NAND or OR
            break;
        case 2:
            if (circuit->gates[i].truthTable[0][1] == circuit->gates[i].truthTable[1][0]) //XOR or XNOR
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];
            }
            else if (circuit->gates[i].truthTable[0][0] == circuit->gates[i].truthTable[0][1]) // 0011 or 1100
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID];
            }
            else // 0101 or 1010
            {
                po[circuit->gates[i].outputID] = po[circuit->gates[i].rightParentID];
            }

            break;

        default: //0000 OR 1111
            po[circuit->gates[i].outputID] = true;
            break;
        }

        if (circuit->gates[i].outputID < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
            evaluated[circuit->gates[i].outputID - reducer] = true;
    }
}

void getPotentiallyObfuscatedGatesThread(BristolCircuit *circuit, bool *po, uint_fast64_t id, uint_fast64_t numThreads, bool *evaluated, uint_fast64_t sleepTime)
{
    using namespace std::chrono_literals;
    auto reducer = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB;
    for (auto i = id; i < circuit->details.numGates; i += numThreads)
    {
        if (circuit->gates[i].rightParentID >= reducer)
        {
            while (not evaluated[circuit->gates[i].rightParentID - reducer])
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(sleepTime));
            }
        }
        if (circuit->gates[i].leftParentID >= reducer)
        {
            while (not evaluated[circuit->gates[i].leftParentID - reducer])
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(sleepTime));
            }
        }
        if (circuit->gates[i].truthTable == 'X')
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] && po[circuit->gates[i].rightParentID];
        else
            po[circuit->gates[i].outputID] = po[circuit->gates[i].leftParentID] || po[circuit->gates[i].rightParentID];
        if (circuit->gates[i].outputID < circuit->details.numWires - circuit->details.bitlengthOutputs * circuit->details.numOutputs)
            evaluated[circuit->gates[i].outputID - reducer] = true;
    }
}

void getPotentiallyIntegrityBreakingGatesFromOutputThread2(CircuitDetails details, bool *po, uint_fast64_t *parents, bool *npib, bool *addedGates, uint_fast64_t id, uint_fast64_t numThreads)
{
    std::queue<uint_fast64_t> pathQueue;
    for (auto i = 0; i < details.numOutputs; i++)
    {
        for (auto j = id; j < details.bitlengthOutputs; j += numThreads)
        {
            pathQueue.push(details.numWires - 1 - i - j);
            while (!pathQueue.empty())
            {
                auto currIndex = pathQueue.front();
                pathQueue.pop();
                npib[currIndex] = true;
                for (auto p = 0; p < 2; p++)
                {
                    if (parents(currIndex, p) >= details.bitlengthInputA + details.bitlengthInputB && po[parents(currIndex, p)] && not addedGates[parents(currIndex, p)])
                    {
                        addedGates[parents(currIndex, p)] = true;
                        pathQueue.push(parents(currIndex, p));
                    }
                }
            }
        }
    }
}

void getPotentiallyIntegrityBreakingGatesFromOutputMT2(CircuitDetails details, bool *po, uint_fast64_t *parents, uint_fast64_t numThreads)
{
    auto npib = new bool[details.numWires]();
    auto addedGates = new bool[details.numWires]();

    std::thread threads[numThreads];
    for (uint_fast64_t i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(getPotentiallyIntegrityBreakingGatesFromOutputThread2, details, po, parents, npib, addedGates, i, numThreads);
    }
    for (auto i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    for (auto i = 0; i < details.numWires; i++)
        po[i] = !npib[i];
    delete[] addedGates;
    delete[] npib;
}