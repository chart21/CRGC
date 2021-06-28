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


#include <chrono>

#define time_S t1 = startClock();
#define time_E stopClock(t1);













template<typename F, typename... Args>
void funcTime(F func, Args&&... args){
    std::chrono::high_resolution_clock::time_point t1 = 
        std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now()-t1).count();
        
    std::cout << time << __func__ << '\n';
}



int main(int argc, char *argv[])
{
    std::string circuitName = "adder64";
    if (argc > 1)
    {
        circuitName = argv[1]; 
    }
    std::string filepath = "/home/christopher/Documents/C-RGCG/src/circuits/" +circuitName;

    std::string circuitFormat = "bristol";
    if (argc > 4)
    {
        circuitFormat = argv[4];
    }

    auto details = importBristolCircuitDetails(filepath + ".txt", circuitFormat);

   
    auto bristolCircuit = importBristolCircuitExNotForLeakagePrediction(filepath+ ".txt", details);
    
    



    // auto t1 = startClock();
    auto parents = new uint_fast64_t[bristolCircuit->details.numWires * 2]();    
    getPrevofEachWire(bristolCircuit, parents);
    
        
    auto successors = new std::vector<uint_fast64_t>[bristolCircuit->details.numWires];
    funcTime(getSuccessorsOfEachWire,bristolCircuit->details, parents, successors);
    

    auto po = new bool[bristolCircuit->details.numWires];    
    funcTime(getPotentiallyObfuscatedGates,bristolCircuit, po);
    //funcTime(getPotentiallyIntegrityBreakingGates,bristolCircuit->details, po, parents, successors);
    //funcTime(getPotentiallyIntegrityBreakingGatesFromOutput,bristolCircuit->details, po, parents);
    funcTime(getPotentiallyIntegrityBreakingGatesFromOutputMT,bristolCircuit->details, po, parents);
    

    int poc = - bristolCircuit->details.bitlengthInputA;
    for (auto i = 0; i < bristolCircuit->details.numWires; i++)
    {
        poc+= po[i];
    }
    std::cout << poc << '\n';

    delete[] po;    
    delete[] bristolCircuit->gates;
    delete[] bristolCircuit;      
    
 

    auto circuit = importBristolCircuitExNot(filepath+ ".txt", details);


    bool* inputA = new bool[circuit->details.bitlengthInputA];
    bool* inputB = new bool[circuit->details.bitlengthInputB];

    generateRandomInput(details.bitlengthInputA, inputA);
    generateRandomInput(details.bitlengthInputB, inputB);

    
    if (argc > 2)
    {        
        uint_fast64_t a = std::stoul(argv[2]);
        converIntToBoolArr(a, circuit->details.bitlengthInputA, inputA);
    }
    if (argc > 3)
    {
        uint_fast64_t b = std::stoul(argv[3]);
        converIntToBoolArr(b, circuit->details.bitlengthInputB, inputB);
    }
    



    auto levels = new uint_fast64_t[circuit->details.numWires];
    auto maxLevel = getLevelOfGates(circuit, levels);

    

    auto circuitByLevels = new std::vector<TransformedGate>[maxLevel]();

    

    getCircuitbyLevels(circuit, levels,circuitByLevels); 

    std::cout << "here" << '\n';
   

    

    bool* output = new bool[circuit->details.numOutputs * circuit->details.bitlengthOutputs];
    //funcTime(evaluateTransformedCircuitN,circuit, inputA, inputB, output);
    
    //funcTime(evaluateTransformedCircuitHackMp,circuit, inputA, inputB, output);
    //funcTime(evaluateTransformedCircuitHackMpUint,circuit, inputA, inputB, output);
    funcTime(evaluateTransformedCircuitHackMpBool,circuit, inputA, inputB, output);
    //funcTime(evaluateTransformedCircuitbyLevel,circuit, circuitByLevels, maxLevel,inputA,inputB,output);
    //funcTime(evaluateTransformedCircuitbyLevelMT,circuit, circuitByLevels, maxLevel,inputA,inputB,output);
    //funcTime(evaluateTransformedCircuitHackMpCond,circuit, inputA,inputB,output);

    delete[] levels;

    

    int inA = convertBoolArrToInt(inputA, details.bitlengthInputA);
    int inB = convertBoolArrToInt(inputB, details.bitlengthInputB);
    int iout = convertBoolArrToInt(output, details.bitlengthOutputs);
    std::cout << "inA" << inA <<"\n";    
    std::cout << "inB" << inB <<"\n"; 
    std::cout << "iout" << iout <<"\n"; 

    bool* obfuscatedValArr = new bool[details.bitlengthInputA];
    bool* flipped = new bool[details.numWires]; 

    

    obfuscateInput(inputA,obfuscatedValArr,flipped,circuit->details);
    funcTime(getFlippedCircuitWithoutOutputsN,circuit, flipped);
    delete[] flipped;

    //obfuscatedValArr = inputA;
    bool *isObfuscated = new bool[circuit->details.numWires]();
    funcTime(more_efficient_obfuscation4NArr,circuit, obfuscatedValArr, isObfuscated);
    
    //more_efficient_obfuscation4N(circuit, inputA);
    uint_fast64_t counter = 0;
    // for(auto i = circuit->details.bitlengthInputA+circuit->details.bitlengthInputB; i < circuit->details.numWires;i++)
    // {
        
    //     if(isObfuscated[i])
    //     {
    //         counter++;
    //         std::cout << "obfuscatedInd" << i << '\n';

    //     }
            

    // }
    // std::cout << "obfuscated" << counter << '\n';

    



        
    funcTime(getIntegrityBreakingGatesN4,circuit->details, isObfuscated, parents, successors);


    poc = - circuit->details.bitlengthInputA;
    for (auto i = 0; i < circuit->details.numWires; i++)
    {
        poc+= isObfuscated[i];
    }
    std::cout << poc << '\n';
    
    

    // for(auto i = 0; i < circuit->details.numWires;i++)
    // {
    //     for(auto s : successors[i])
    //         std::cout <<  i << " : " << s << '\n';

    // }
    counter = 0;
    // for(auto i = circuit->details.bitlengthInputA+circuit->details.bitlengthInputB; i < circuit->details.numWires;i++)
    // {
        
    //     if(isObfuscated[i])
    //     {
    //         counter++;
    //         std::cout << "obfuscatedInd" << i << '\n';

    //     }

    // }
    // std::cout << "obfuscated" << counter << '\n';

    delete[] parents;
    delete[] successors;        
    funcTime(breakIntegrityOfGates,circuit, isObfuscated);
    



    
    delete[] isObfuscated;
    
    
    auto outputRGC = new bool[circuit->details.bitlengthOutputs*circuit->details.numOutputs];
    evaluateTransformedCircuitN(circuit, obfuscatedValArr, inputB, outputRGC);
    std::cout << (*outputRGC == *output) << '\n';

    //printArr(successors,circuit->details.numWires);

    

    exportCircuitSeparateFiles(circuit, filepath);
    exportObfuscatedInput(obfuscatedValArr, circuit->details, filepath);

    auto newOut1 = new bool[circuit->details.bitlengthOutputs*circuit->details.numOutputs];
    evaluateTransformedCircuitN(circuit, obfuscatedValArr, inputB, newOut1);
    inA = convertBoolArrToInt(obfuscatedValArr, details.bitlengthInputA);
    inB = convertBoolArrToInt(inputB, details.bitlengthInputB);
    auto iout1 = convertBoolArrToInt(newOut1, details.bitlengthOutputs);
    // std::cout << "inA" << inA <<"\n";    
    // std::cout << "iout" << iout1 <<"\n";
    // std::cout << "inB" << inB <<"\n"; 
     



    //delete[] obfuscatedValArr;
    delete[] circuit->gates;
    delete[] circuit; 

    auto newDetails = importBristolCircuitDetails(filepath + "_rgc_details.txt", "rgc");

        
    
    auto newCircuit = importTransformedCircuit(filepath + "_rgc.txt", newDetails);

    

    auto newInputA = new bool[newCircuit->details.bitlengthInputA];
    importInput(filepath + "_rgc_inputA.txt",newCircuit->details.bitlengthInputA, newInputA);
    auto newOut2 = new bool[newCircuit->details.bitlengthOutputs*newCircuit->details.numOutputs];    
    evaluateTransformedCircuitN(newCircuit, newInputA, inputB, newOut2);

    inA = convertBoolArrToInt(newInputA, newDetails.bitlengthInputA);
    inB = convertBoolArrToInt(inputB, newDetails.bitlengthInputB);
    auto iout2 = convertBoolArrToInt(newOut2, newDetails.bitlengthOutputs);


    // for (auto i = 0; i < circuit->details.bitlengthInputA;i++)
    // {
    //     std::cout << obfuscatedValArr[i]; 
    // }
    // std::cout << '\n';
    // for (auto i = 0; i < circuit->details.bitlengthInputA;i++)
    // {
    //     std::cout << newInputA[i]; 
    // }
    // std::cout << '\n';

   



    

    

    
    return 0;
}
