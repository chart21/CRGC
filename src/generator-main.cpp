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
#include "circuitProcessor/include/circuitTransformer.h"

#include "circuitProcessor/include/leakagePredictor.h"

//#include <emp-tool/emp-tool.h>
// #include "programs/include/mult3.h"
// #include "programs/include/paperoptm.h"
#include "programs/include/examplePrograms.h"
#include "programs/include/circuitLinker.h"

#include <chrono>

#define time_S t1 = startClock();
#define time_E stopClock(t1);

//#define CIRCUITPATH "/home/christopher/Documents/C-RGCG/src/circuits/"
#define CIRCUITPATH "../src/circuits/"

template <typename F, typename... Args>
void funcTime(std::string printText, F func, Args &&...args)
{
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - t1)
                    .count();

    std::cout << "---TIMING--- " << time << "ms " << printText << '\n';
}

// void generateCircuitRAM(std::vector<BristolGate> *gateVec, CircuitDetails *details, bool print, std::string programName)
// {
//     if (programName == "mult3")
//         generateCircuitRAMMult(gateVec, details, print);

//     else if (programName == "paperoptm")
//         generateCircuitRAMPaper(gateVec, details, print);
//     else
//         std::cout << "Program not found, make sure you added it in CircuitLinker.cpp." << '\n';
// }

TransformedCircuit *loadTransformedCircuit(std::string circuitName, std::string fileFormat, std::string circuitFormat)
{

    if (fileFormat == "cpp")
    {
        std::vector<BristolGate> gateVec;
        CircuitDetails empDetails;
        //funcTime("converting program to circuit", generateCircuitRAMPaper, &gateVec, &empDetails);
        funcTime("converting program to circuit", generateCircuitRAM, &gateVec, &empDetails, false, circuitName);
        //auto flipped = new bool[empDetails.numWires];
        //auto bristolCircuit = importBristolCircuitExNotForLeakagePredictionFromRAM(&gateVec, empDetails, flipped);
        auto circuit = importTransformedCircuitExNotForLeakagePredictionFromRAM(&gateVec, empDetails);
        std::vector<BristolGate>().swap(gateVec); //deallocate memory of vector
        std::cout << "---INFO--- numGates: " << circuit->details.numGates << '\n';
        //auto unflippedCircuit = new UnflippedCircuit{bristolCircuit, flipped};
        return circuit;
    }
    else if (fileFormat == "txt")
    {
        std::string filepath = CIRCUITPATH + circuitName;
        auto details = importBristolCircuitDetails(filepath + ".txt", circuitFormat);
        //auto flipped = new bool[details.numWires];
        //auto bristolCircuit = importBristolCircuitExNotForLeakagePrediction(filepath + ".txt", details, flipped);
        auto circuit = importBristolCircuitExNot(filepath + ".txt", details);
        //auto unflippedCircuit = new UnflippedCircuit{bristolCircuit, flipped};
        return circuit;
    }
}

void predictLeakage(TransformedCircuit *circuit, uint_fast64_t numThreads, uint_fast64_t *parents, std::string circuitFormat, std::string fileFormat)
{

    auto po = new bool[circuit->details.numWires];
    if (numThreads == 1)
    {
        funcTime("getting Parents of each Wire", getPrevofEachWireT, circuit, parents);
        //funcTime(getPotentiallyObfuscatedGates, bristolCircuit, po);
        //auto circuit = transformBristolCircuitToTransformedCircuitN(bristolCircuit);
        funcTime("identifying potentially obfuscated fixed gates", getPotentiallyObfuscatedFixedGatesT, circuit, po);
        funcTime("identifying potentially intermediary gates", getPotentiallyIntermediaryGatesFromOutput, circuit->details, po, parents);
    }
    else
    {
        funcTime("getting Parents of each Wire", getPrevofEachWireMTTransformed, circuit, parents, numThreads);
        funcTime("identifying potentially obfuscated fixed gates", getPotentiallyObfuscatedFixedGatesT, circuit, po);
        funcTime("identifying potentially intermediary gates", getPotentiallyIntermediaryGatesFromOutputMT, circuit->details, po, parents, numThreads);
    }
    uint_fast64_t poc = -circuit->details.bitlengthInputA;
    for (auto i = 0; i < circuit->details.numWires; i++)
    {
        poc += po[i];
    }
    std::cout << "---INFO--- potentially obfuscated and integrity-breaking gates: " << poc << '\n';

    std::vector<uint_fast64_t> leakedInputs;
    //funcTime(getLeakedInputsFromOutput, circuit, po, &leakedInputs);

    if(circuitFormat == "emp" || fileFormat == "cpp")
       funcTime("predict leaked inputs", getLeakedInputsFromOutputSortedMT, circuit, po, &leakedInputs, numThreads);
    else
    {
    auto circuitLineOfWireIndex = new uint_fast64_t[circuit->details.numWires];
    //funcTime("get line of each index", getCircuitLineofWireIndex, circuit, circuitLineOfWireIndex);
    //funcTime("predict leaked inputs", getLeakedInputsFromOutputUnsorted, circuit, po, &leakedInputs, circuitLineOfWireIndex);
    funcTime("get line of each index", getCircuitLineofWireIndexMT, circuit, circuitLineOfWireIndex, numThreads);
    
        
    funcTime("predict leaked inputs", getLeakedInputsFromOutputUnsortedMT, circuit, po, &leakedInputs, circuitLineOfWireIndex, numThreads);

    }    

    std::cout << "---INFO--- " << leakedInputs.size() << " leaked inputs: " << '\n';
    for (auto i = 0; i < leakedInputs.size(); i++)
    {
        std::cout << leakedInputs[i] << '\n';
        ;
    }

    delete[] po;
}

void evaluateCircuit(TransformedCircuit *circuit, uint_fast64_t numThreads, int argc, char *argv[], bool *inputA, bool *inputB, bool *output, std::string circuitName, std::string circuitFormat, std::string fileFormat)
{
    //auto circuit = importBristolCircuitExNot(filepath+ ".txt", details);

    std::string filepath = CIRCUITPATH + circuitName;
    if (argc > 3)
    {
        uint_fast64_t a;
        if (check_number(argv[3]))
        {
            uint_fast64_t a = std::stoul(argv[3]);
            converIntToBoolArr(a, circuit->details.bitlengthInputA, inputA);
        }
        else
        {
            if (argv[3] == std::string("r"))
                generateRandomInput(circuit->details.bitlengthInputA, inputA);
            else
                importBinaryInput(filepath + "_inputA.txt", circuit->details.bitlengthInputA, inputA);
        }
    }
    else
        generateRandomInput(circuit->details.bitlengthInputA, inputA);

    if (argc > 4)
    {
        uint_fast64_t b;
        if (check_number(argv[4]))
        {
            uint_fast64_t b = std::stoul(argv[4]);
            converIntToBoolArr(b, circuit->details.bitlengthInputB, inputB);
        }
        else
        {
            if (argv[4] == std::string("r"))
                generateRandomInput(circuit->details.bitlengthInputB, inputB);
            else
                importBinaryInput(filepath + "_inputB.txt", circuit->details.bitlengthInputB, inputB);
        }
    }
    else
        generateRandomInput(circuit->details.bitlengthInputB, inputB);


    
    if(circuitFormat == "emp" || fileFormat == "cpp")
        funcTime("evaluate circuit", evaluateSortedTransformedCircuit, circuit, inputA, inputB, output);
    else
        funcTime("evaluate circuit", evaluateTransformedCircuit, circuit, inputA, inputB, output);



    


    int inA = convertBoolArrToInt(inputA, circuit->details.bitlengthInputA);
    int inB = convertBoolArrToInt(inputB, circuit->details.bitlengthInputB);
    int iout = convertBoolArrToInt(output, circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";
}

void obfuscateCircuit(TransformedCircuit *circuit, bool *inputA, uint_fast64_t *parents, bool *obfuscatedValArr, uint_fast64_t numThreads)
{

    bool *flipped = new bool[circuit->details.numWires];

    obfuscateInput(inputA, obfuscatedValArr, flipped, circuit->details);
    if (numThreads == 1)
        funcTime("flip circuit", getFlippedCircuit, circuit, flipped);
    else
        funcTime("flip circuit", getFlippedCircuit, circuit, flipped);
    delete[] flipped;

    bool *isObfuscated = new bool[circuit->details.numWires]();

    funcTime("identify fixed Gates", indentifyFixedGatesArr, circuit, obfuscatedValArr, isObfuscated);


    uint_fast64_t counter = 0;
    for (auto i = circuit->details.bitlengthInputA + circuit->details.bitlengthInputB; i < circuit->details.numWires; i++)
    {

        if (isObfuscated[i])
        {
            counter++;
            //std::cout << "obfuscatedInd" << i << '\n';
        }
    }
    std::cout << "---INFO--- obfuscated gates: " << counter << '\n';
    if (numThreads == 1)
        funcTime("identify intermediary gates", getIntermediaryGatesfromOutput, circuit->details, isObfuscated, parents);
    else
        funcTime("identify intermediary gates", getIntermediaryGatesfromOutputMT2, circuit->details, isObfuscated, parents, numThreads);

    delete[] parents;

    auto poc = -circuit->details.bitlengthInputA;
    for (auto i = 0; i < circuit->details.numWires; i++)
    {
        poc += isObfuscated[i];
    }
    std::cout << "---INFO--- obfuscated and integrity-breaking gates: " << poc << '\n';

    if (numThreads == 1)
        funcTime("obfuscate gates", regenerateGates, circuit, isObfuscated);
    else
        funcTime("obfuscate gates", regenerateGatesMT, circuit, isObfuscated, numThreads);

    delete[] isObfuscated;
}

void verifyIntegrityOfObfuscatedCircuit(TransformedCircuit *circuit, bool *obfuscatedValArr, bool *inputA, bool *inputB, bool *output, uint_fast64_t numThreads)
{
    auto outputRGC = new bool[circuit->details.bitlengthOutputs * circuit->details.numOutputs];
    
    evaluateTransformedCircuit(circuit, obfuscatedValArr, inputB, outputRGC);

    if (equalBoolArr(outputRGC, output, circuit->details.bitlengthOutputs))
        std::cout << "---Success--- Evaluation of original circuit and constructed RGC are equal" << '\n';
    else
        std::cout << "---Warning--- Evaluation of original circuit and constructed RGC are not equal" << '\n';

    auto inA = convertBoolArrToInt(obfuscatedValArr, circuit->details.bitlengthInputA);
    auto inB = convertBoolArrToInt(inputB, circuit->details.bitlengthInputB);
    auto iout = convertBoolArrToInt(outputRGC, circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";

    delete[] outputRGC;
}

void verifyIntegrityOfExportedRGC(TransformedCircuit *circuit, bool *outputRGC, bool *inputB, uint_fast64_t numThreads)
{
    std::string filepath = CIRCUITPATH;
    auto newDetails = importBristolCircuitDetails(filepath + "_rgc_details.txt", "rgc");
    auto newCircuit = importTransformedCircuit(filepath + "_rgc.txt", newDetails);

    auto newInputA = new bool[newCircuit->details.bitlengthInputA];
    importBinaryInput(filepath + "_rgc_inputA.txt", newCircuit->details.bitlengthInputA, newInputA);
    auto newOut2 = new bool[newCircuit->details.bitlengthOutputs * newCircuit->details.numOutputs];

    evaluateTransformedCircuit(newCircuit, newInputA, inputB, newOut2);

    if (equalBoolArr(outputRGC, newOut2, circuit->details.bitlengthOutputs))
        std::cout << "---Success--- Evaluation of original circuit and constructed RGC are equal" << '\n';
    else
        std::cout << "---Warning--- Evaluation of original circuit and constructed RGC are not equal" << '\n';

    auto inA = convertBoolArrToInt(newInputA, newDetails.bitlengthInputA);
    auto inB = convertBoolArrToInt(inputB, newDetails.bitlengthInputB);
    auto iout2 = convertBoolArrToInt(newOut2, newDetails.bitlengthOutputs);
}

int main(int argc, char *argv[])
{

    uint_fast64_t numThreads = 1;    
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



    auto circuit = loadTransformedCircuit(circuitName, fileFormat, circuitFormat);

    auto parents = new uint_fast64_t[circuit->details.numWires * 2]();

    predictLeakage(circuit, numThreads, parents, circuitFormat, fileFormat);

    bool *inputA = new bool[circuit->details.bitlengthInputA];
    bool *inputB = new bool[circuit->details.bitlengthInputB];
    bool *output = new bool[circuit->details.numOutputs * circuit->details.bitlengthOutputs];

    evaluateCircuit(circuit, numThreads, argc, argv, inputA, inputB, output, circuitName, circuitFormat, fileFormat);

    bool *obfuscatedValArr = new bool[circuit->details.bitlengthInputA];
    obfuscateCircuit(circuit, inputA, parents, obfuscatedValArr, numThreads);

    verifyIntegrityOfObfuscatedCircuit(circuit, obfuscatedValArr, inputA, inputB, output, numThreads);

    auto originalCircuit = loadTransformedCircuit(circuitName, fileFormat, circuitFormat);
    funcTime("compare circuit similarity", compareCircuitSimilarityMT, originalCircuit, circuit, numThreads);

    //deleteRevealGates(circuit, circuitLineOfWireIndex);

    //exportCircuitSeparateFiles(circuit, CIRCUITPATH);
    //exportObfuscatedInput(obfuscatedValArr, circuit->details, CIRCUITPATH);

    //verifyIntegrityOfExportedRGC(circuit, output, inputB, numThreads);

    return 0;
}
