#include <iostream>
//#include "circuitProcessor/include/circuitStructs.h"

#include "circuitProcessor/include/circuitReader.h"
#include "circuitProcessor/include/circuitEvaluator.h"
//#include <./circuitProcessor/include/circuitStructs.h>
#include "circuitProcessor/include/helperFunctions.h"
#include "circuitProcessor/include/circuitFlipper.h"
#include "circuitProcessor/include/circuitObfuscator.h"
#include "circuitProcessor/include/circuitDetails.h"
#include "circuitProcessor/include/circuitIntegrityBreaker.h"
#include "circuitProcessor/include/circuitWriter.h"
#include "circuitProcessor/include/leakagePredictor.h"

//#include <emp-tool/emp-tool.h>
#include "programs/include/mult3.h"
#include "programs/include/paperoptm.h"

#include <chrono>

#define time_S t1 = startClock();
#define time_E stopClock(t1);

#define CIRCUITPATH "/home/christopher/Documents/C-RGCG/src/circuits/"

template <typename F, typename... Args>
void funcTime(F func, Args &&...args)
{
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - t1)
                    .count();

    std::cout << time << __func__ << '\n';
}

BristolCircuit *loadBristolCircuit(std::string circuitName, std::string fileFormat, std::string circuitFormat)
{

    if (fileFormat == "cpp")
    {
        std::vector<BristolGate> gateVec;
        CircuitDetails empDetails;
        funcTime(generateCircuitRAMPaper, &gateVec, &empDetails);
        auto bristolCircuit = importBristolCircuitExNotForLeakagePredictionFromRAM(&gateVec, empDetails);
        std::vector<BristolGate>().swap(gateVec); //deallocate memory of vector
        return bristolCircuit;
    }
    else if (fileFormat == "txt")
    {
        std::string filepath = CIRCUITPATH + circuitName;
        auto details = importBristolCircuitDetails(filepath + ".txt", circuitFormat);
        auto bristolCircuit = importBristolCircuitExNotForLeakagePrediction(filepath + ".txt", details);
        return bristolCircuit;
    }
}

void predictLeakage(BristolCircuit *bristolCircuit, uint_fast64_t numThreads, uint_fast64_t timeSleep, uint_fast64_t* parents)
{
    
    auto po = new bool[bristolCircuit->details.numWires];
    if (numThreads == 1)
    {
        funcTime(getPrevofEachWire, bristolCircuit, parents);
        funcTime(getPotentiallyObfuscatedGates, bristolCircuit, po);
        funcTime(getPotentiallyIntegrityBreakingGatesFromOutput, bristolCircuit->details, po, parents);
    }
    else
    {
        getPrevofEachWireMT(bristolCircuit, parents, numThreads);
        funcTime(getPotentiallyObfuscatedGatesMT, bristolCircuit, po, numThreads, timeSleep);
        funcTime(getPotentiallyIntegrityBreakingGatesFromOutputMT, bristolCircuit->details, po, parents, numThreads);
    }
    int poc = -bristolCircuit->details.bitlengthInputA;
    for (auto i = 0; i < bristolCircuit->details.numWires; i++)
    {
        poc += po[i];
    }
    std::cout << "potentially obfuscated and integrity breaking" << poc << '\n';
    delete[] po;
}

void evaluateCircuit(TransformedCircuit* circuit, uint_fast64_t numThreads, uint_fast64_t timeSleep, int argc, char *argv[], bool* inputA, bool *inputB, bool *output)
{
    //auto circuit = importBristolCircuitExNot(filepath+ ".txt", details);
    generateRandomInput(circuit->details.bitlengthInputA, inputA);
    generateRandomInput(circuit->details.bitlengthInputB, inputB);

    if (argc > 3)
    {
        uint_fast64_t a = std::stoul(argv[3]);
        converIntToBoolArr(a, circuit->details.bitlengthInputA, inputA);
    }
    if (argc > 4)
    {
        uint_fast64_t b = std::stoul(argv[4]);
        converIntToBoolArr(b, circuit->details.bitlengthInputB, inputB);
    }

    
    if(numThreads == 1)
        funcTime(evaluateTransformedCircuit, circuit, inputA, inputB, output);
    else
        funcTime(evaluateTransformedCircuitMT, circuit, inputA, inputB, output, numThreads, timeSleep);

    int inA = convertBoolArrToInt(inputA, circuit->details.bitlengthInputA);
    int inB = convertBoolArrToInt(inputB, circuit->details.bitlengthInputB);
    int iout = convertBoolArrToInt(output, circuit->details.bitlengthOutputs);
    std::cout << "inA" << inA << "\n";
    std::cout << "inB" << inB << "\n";
    std::cout << "iout" << iout << "\n";
}



   
    
   


    

  

void obfuscateCircuit(TransformedCircuit* circuit, bool* inputA, uint_fast64_t* parents, bool* obfuscatedValArr, uint_fast64_t numThreads, uint_fast64_t timeSleep)
{
    
    bool *flipped = new bool[circuit->details.numWires];

    obfuscateInput(inputA, obfuscatedValArr, flipped, circuit->details);
    if(numThreads == 1)
        funcTime(getFlippedCircuitWithoutOutputsN, circuit, flipped);
    else
        funcTime(getFlippedCircuitWithoutOutputsMT, circuit, flipped, numThreads, timeSleep);
    //funcTime(getFlippedCircuitWithoutOutputsMT,circuit, flipped, 7, 200);
    delete[] flipped;

    bool *isObfuscated = new bool[circuit->details.numWires]();
    if(numThreads == 1)
        funcTime(moreEfficientObfuscationArr, circuit, obfuscatedValArr, isObfuscated);
    else
        funcTime(moreEfficientObfuscationMT, circuit, obfuscatedValArr, isObfuscated, numThreads, timeSleep);

    uint_fast64_t counter = 0;
    for (auto i = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB; i < circuit->details.numWires; i++)
    {

        if (isObfuscated[i])
        {
            counter++;
            //std::cout << "obfuscatedInd" << i << '\n';
        }
    }
    std::cout << "obfuscated" << counter << '\n';
    if(numThreads == 1)
        funcTime(getIntegrityBreakingGatesfromOutput, circuit->details, isObfuscated, parents);
    else
        funcTime(getIntegrityBreakingGatesfromOutputMT2, circuit->details, isObfuscated, parents, numThreads);

    delete[] parents;

    auto poc = -circuit->details.bitlengthInputA;
    for (auto i = 0; i < circuit->details.numWires; i++)
    {
        poc += isObfuscated[i];
    }
    std::cout << "obfuscated and Integrity breaking" << poc << '\n';
    
    if(numThreads == 1)
        funcTime(breakIntegrityOfGates, circuit, isObfuscated);
    else
        funcTime(breakIntegrityOfGatesMT, circuit, isObfuscated, numThreads);

    delete[] isObfuscated;
}

 


void verifyIntegrityOfObfuscatedCircuit(TransformedCircuit* circuit, bool* obfuscatedValArr, bool* inputA, bool *inputB, bool* output, uint_fast64_t numThreads, uint_fast64_t timeSleep)
{
    auto outputRGC = new bool[circuit->details.bitlengthOutputs * circuit->details.numOutputs];
    
    if(numThreads == 1)
        evaluateTransformedCircuit(circuit, obfuscatedValArr, inputB, outputRGC);
    else
        evaluateTransformedCircuitMT(circuit, obfuscatedValArr, inputB, outputRGC, numThreads, timeSleep);


    std::cout << "Circuit outputs are equal?" << equalBoolArr(outputRGC, output, circuit->details.bitlengthOutputs) << '\n';

    auto inA = convertBoolArrToInt(obfuscatedValArr, circuit->details.bitlengthInputA);
    auto inB = convertBoolArrToInt(inputB, circuit->details.bitlengthInputB);
    auto iout = convertBoolArrToInt(outputRGC, circuit->details.bitlengthOutputs);
    std::cout << "inA" << inA << "\n";
    std::cout << "inB" << inB << "\n";
    std::cout << "iout" << iout << "\n";

    delete[] outputRGC;
}




void verifyIntegrityOfExportedRGC(TransformedCircuit* circuit, bool* outputRGC, bool* inputB,  uint_fast64_t numThreads, uint_fast64_t timeSleep)
{
    std::string filepath = CIRCUITPATH;
    auto newDetails = importBristolCircuitDetails(filepath + "_rgc_details.txt", "rgc");
    auto newCircuit = importTransformedCircuit(filepath + "_rgc.txt", newDetails);

    auto newInputA = new bool[newCircuit->details.bitlengthInputA];
    importInput(filepath + "_rgc_inputA.txt", newCircuit->details.bitlengthInputA, newInputA);
    auto newOut2 = new bool[newCircuit->details.bitlengthOutputs * newCircuit->details.numOutputs];
    
    if(numThreads == 1)
        evaluateTransformedCircuit(newCircuit, newInputA, inputB, newOut2);
    else
        evaluateTransformedCircuitMT(newCircuit, newInputA, inputB, newOut2, numThreads, timeSleep);


    std::cout << "Circuit outputs are equal?" << equalBoolArr(outputRGC, newOut2, circuit->details.bitlengthOutputs) << '\n';

    auto inA = convertBoolArrToInt(newInputA, newDetails.bitlengthInputA);
    auto inB = convertBoolArrToInt(inputB, newDetails.bitlengthInputB);
    auto iout2 = convertBoolArrToInt(newOut2, newDetails.bitlengthOutputs);
}

int main(int argc, char *argv[])
{

    uint_fast64_t numThreads = 1;
    uint_fast64_t timeSleep = 300;
    std::string fileFormat = "cpp";
    std::string circuitName = "adder64";
    
    

    if (argc > 1)
    {
        circuitName = argv[1];
    }

    

    if (argc > 2)
    {
        fileFormat = argv[2];
    }

    std::string circuitFormat = "bristol";
    if (argc > 5)
    {
        circuitFormat = argv[5];
    }

    if (argc > 6)
    {
        numThreads = std::stoul(argv[6]);
    }

    if (argc > 7)
    {
        timeSleep = std::stoul(argv[7]);
    }

    auto bristolCircuit = loadBristolCircuit(circuitName, fileFormat, circuitFormat);

    auto parents = new uint_fast64_t[bristolCircuit->details.numWires * 2]();

    predictLeakage(bristolCircuit, numThreads, timeSleep, parents);

    auto circuit = new TransformedCircuit[bristolCircuit->details.numGates];

    transformBristolCircuitToTransformedCircuit(bristolCircuit, circuit);

    delete[] bristolCircuit->gates;
    delete[] bristolCircuit;

    

    bool *inputA = new bool[circuit->details.bitlengthInputA];
    bool *inputB = new bool[circuit->details.bitlengthInputB];
    bool *output = new bool[circuit->details.numOutputs * circuit->details.bitlengthOutputs];

    evaluateCircuit(circuit, numThreads, timeSleep, argc, argv, inputA, inputB, output);


    bool *obfuscatedValArr = new bool[circuit->details.bitlengthInputA];
    obfuscateCircuit(circuit, inputA, parents, obfuscatedValArr, numThreads, timeSleep);

    verifyIntegrityOfObfuscatedCircuit(circuit, obfuscatedValArr, inputA, inputB, output, numThreads, timeSleep);




    //exportCircuitSeparateFiles(circuit, CIRCUITPATH);
    //exportObfuscatedInput(obfuscatedValArr, circuit->details, CIRCUITPATH);

    //verifyIntegrityOfExportedRGC(circuit, output, inputB, numThreads, timeSleep);


    return 0;

}


 
 

