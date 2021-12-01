#include "circuitProcessor/include/circuitReader.h"
#include "circuitProcessor/include/circuitWriter.h"
#include "partyConfig.h"
#include <emp-tool/io/file_io_channel.h>


//#define DEBUG
#define time_S t1 = startClock();
#define time_E stopClock(t1);






class Generator: public Party {
public:
    void loadTransformedCircuit();

    void predictLeakage(uint_fast64_t *parents);
    void evaluateCircuit();
    void obfuscateCircuit(uint_fast64_t *parents);
    void verifyIntegrityOfObfuscatedCircuit();
    void verifyIntegrityOfExportedRGC();
};


void Generator::loadTransformedCircuit()
{

    if (fileFormat == "cpp")
    {
        std::vector<BristolGate> gateVec;
        CircuitDetails empDetails;
        funcTime("converting program to circuit", generateCircuitRAM, &gateVec, &empDetails, false, circuitName);
        circuitData.circuit = importTransformedCircuitExNotForLeakagePredictionFromRAM(&gateVec, empDetails);
        std::vector<BristolGate>().swap(gateVec); //deallocate memory of vector
        std::cout << "---INFO--- numGates: " << circuitData.circuit->details.numGates << '\n';
    }
    else if (fileFormat == "txt")
    {
        std::string filepath = CIRCUITPATH + circuitName;
        auto details = importBristolCircuitDetails(filepath + ".txt", circuitFormat);
        circuitData.circuit = importBristolCircuitExNot(filepath + ".txt", details);

    }
    circuitData.inputA = new bool[circuitData.circuit->details.bitlengthInputA];
    circuitData.inputB = new bool[circuitData.circuit->details.bitlengthInputB];
    circuitData.output = new bool[circuitData.circuit->details.numOutputs * circuitData.circuit->details.bitlengthOutputs];
    circuitData.obfuscatedValArr = new bool[circuitData.circuit->details.bitlengthInputA];
}

void Generator::predictLeakage(uint_fast64_t *parents)
{

    auto po = new bool[circuitData.circuit->details.numWires];
    if (numThreads == 1)
    {
        funcTime("getting Parents of each Wire", getPrevofEachWireT, circuitData.circuit, parents);
        funcTime("identifying potentially obfuscated fixed gates", getPotentiallyObfuscatedFixedGatesT, circuitData.circuit, po);
        funcTime("identifying potentially intermediary gates", getPotentiallyIntermediaryGatesFromOutput, circuitData.circuit->details, po, parents);
    }
    else
    {
        funcTime("getting Parents of each Wire", getPrevofEachWireMTTransformed, circuitData.circuit, parents, numThreads);
        funcTime("identifying potentially obfuscated fixed gates", getPotentiallyObfuscatedFixedGatesT, circuitData.circuit, po);
        funcTime("identifying potentially intermediary gates", getPotentiallyIntermediaryGatesFromOutputMT, circuitData.circuit->details, po, parents, numThreads);
    }
    uint_fast64_t poc = -circuitData.circuit->details.bitlengthInputA;
    for (auto i = 0; i < circuitData.circuit->details.numWires; i++)
    {
        poc += po[i];
    }
    std::cout << "---INFO--- potentially obfuscated fixed and intermediary gates: " << poc << '\n';

    std::vector<uint_fast64_t> leakedInputs;
    //funcTime(getLeakedInputsFromOutput, circuit, po, &leakedInputs);

    if(circuitFormat == "emp" || fileFormat == "cpp")
    {
        if (numThreads == 1)
            funcTime("predict leaked inputs", getLeakedInputsFromOutputSorted, circuitData.circuit, po, &leakedInputs);
        else
            funcTime("predict leaked inputs", getLeakedInputsFromOutputSortedMT, circuitData.circuit, po, &leakedInputs, numThreads);
    }     
    
    else
    {
        auto circuitLineOfWireIndex = new uint_fast64_t[circuitData.circuit->details.numWires];

        if (numThreads == 1)
        {
            funcTime("get line of each index", getCircuitLineofWireIndex, circuitData.circuit, circuitLineOfWireIndex);
            funcTime("predict leaked inputs", getLeakedInputsFromOutputUnsorted, circuitData.circuit, po, &leakedInputs, circuitLineOfWireIndex);
        }
        else
        {
            funcTime("get line of each index", getCircuitLineofWireIndexMT, circuitData.circuit, circuitLineOfWireIndex, numThreads);
            funcTime("predict leaked inputs", getLeakedInputsFromOutputUnsortedMT, circuitData.circuit, po, &leakedInputs, circuitLineOfWireIndex, numThreads);
        }
    }    
    std::cout << "---INFO--- " << leakedInputs.size() << " leaked inputs: " << '\n';
    for (auto i = 0; i < leakedInputs.size(); i++)
    {
        std::cout << leakedInputs[i] << '\n';
    }
    delete[] po;
}


void Generator::evaluateCircuit()
{
    std::string filepath = CIRCUITPATH + circuitName;
    if(circuitFormat == "emp" || fileFormat == "cpp")
        funcTime("evaluate circuit", evaluateSortedTransformedCircuit, circuitData.circuit, circuitData.inputA, circuitData.inputB, circuitData.output);
    else
        funcTime("evaluate circuit", evaluateTransformedCircuit, circuitData.circuit, circuitData.inputA, circuitData.inputB, circuitData.output);
    
    int inA = convertBoolArrToInt(circuitData.inputA, circuitData.circuit->details.bitlengthInputA);
    int inB = convertBoolArrToInt(circuitData.inputB, circuitData.circuit->details.bitlengthInputB);
    int iout = convertBoolArrToInt(circuitData.output, circuitData.circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";
}

void Generator::obfuscateCircuit(uint_fast64_t *parents)
{

    bool *flipped = new bool[circuitData.circuit->details.numWires];

    obfuscateInput(circuitData.inputA, circuitData.obfuscatedValArr, flipped, circuitData.circuit->details);
    if (numThreads == 1)
        funcTime("flip circuit", getFlippedCircuit, circuitData.circuit, flipped);
    else
        funcTime("flip circuit", getFlippedCircuit, circuitData.circuit, flipped);
    delete[] flipped;

    bool *isObfuscated = new bool[circuitData.circuit->details.numWires]();

    funcTime("identify fixed Gates", indentifyFixedGatesArr, circuitData.circuit, circuitData.obfuscatedValArr, isObfuscated);


    uint_fast64_t counter = 0;
    for (auto i = circuitData.circuit->details.bitlengthInputA + circuitData.circuit->details.bitlengthInputB; i < circuitData.circuit->details.numWires; i++)
    {

        if (isObfuscated[i])
        {
            counter++;
        }
    }
    std::cout << "---INFO--- obfuscated gates: " << counter << '\n';
    if (numThreads == 1)
        funcTime("identify intermediary gates", getIntermediaryGatesfromOutput, circuitData.circuit->details, isObfuscated, parents);
    else
        funcTime("identify intermediary gates", getIntermediaryGatesfromOutputMT2, circuitData.circuit->details, isObfuscated, parents, numThreads);

    delete[] parents;

    auto poc = -circuitData.circuit->details.bitlengthInputA;
    for (auto i = 0; i < circuitData.circuit->details.numWires; i++)
    {
        poc += isObfuscated[i];
    }
    std::cout << "---INFO--- obfuscated fixed and intermediary gates: " << poc << '\n';

    if (numThreads == 1)
        funcTime("obfuscate gates", regenerateGates, circuitData.circuit, isObfuscated);
    else
        funcTime("obfuscate gates", regenerateGatesMT, circuitData.circuit, isObfuscated, numThreads);

    delete[] isObfuscated;
}

void Generator::verifyIntegrityOfObfuscatedCircuit()
{
    auto outputRGC = new bool[circuitData.circuit->details.bitlengthOutputs * circuitData.circuit->details.numOutputs];
    evaluateTransformedCircuit(circuitData.circuit, circuitData.obfuscatedValArr, circuitData.inputB, outputRGC);
    if (equalBoolArr(outputRGC, circuitData.output, circuitData.circuit->details.bitlengthOutputs))
        std::cout << "---Success--- Evaluation of original circuit and constructed RGC are equal" << '\n';
    else
        std::cout << "---Warning--- Evaluation of original circuit and constructed RGC are not equal" << '\n';

    auto inA = convertBoolArrToInt(circuitData.obfuscatedValArr, circuitData.circuit->details.bitlengthInputA);
    auto inB = convertBoolArrToInt(circuitData.inputB, circuitData.circuit->details.bitlengthInputB);
    auto iout = convertBoolArrToInt(outputRGC, circuitData.circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";
    delete[] outputRGC;
}

void Generator::verifyIntegrityOfExportedRGC()
{
    std::string filepath = CIRCUITPATH;

    auto newDetails = importBristolCircuitDetails(filepath + "_rgc_details.txt", "rgc");
    auto newCircuit = importTransformedCircuit(filepath + "_rgc.txt", newDetails);

    auto newInputA = new bool[newCircuit->details.bitlengthInputA];
    importBinaryInput(filepath + "_rgc_inputA.txt", newCircuit->details.bitlengthInputA, newInputA);
    auto newOut2 = new bool[newCircuit->details.bitlengthOutputs * newCircuit->details.numOutputs];

    evaluateTransformedCircuit(newCircuit, newInputA, circuitData.inputB, newOut2);

    if (equalBoolArr(circuitData.output, newOut2, circuitData.circuit->details.bitlengthOutputs))
        std::cout << "---Success--- Evaluation of original circuit and constructed RGC are equal" << '\n';
    else
        std::cout << "---Warning--- Evaluation of original circuit and constructed RGC are not equal" << '\n';

    auto inA = convertBoolArrToInt(newInputA, newDetails.bitlengthInputA);
    auto inB = convertBoolArrToInt(circuitData.inputB, newDetails.bitlengthInputB);
    auto iout2 = convertBoolArrToInt(newOut2, newDetails.bitlengthOutputs);
}


int main(int argc, char *argv[])
{

    Generator* generator = new Generator();

    vector<char*> inputs(2,nullptr);
    parseCLIOptions(argc, argv, generator,inputs);

    
    
    generator->loadTransformedCircuit();

    auto parents = new uint_fast64_t[generator->circuitData.circuit->details.numWires * 2]();
    generator->predictLeakage(parents);  
    
    parseCLIArguments(inputs,generator);
    generator->evaluateCircuit();   
    auto parents_tmp = new uint_fast64_t[generator->circuitData.circuit->details.numWires * 2]();
    generator->obfuscateCircuit(parents);
   
    generator->verifyIntegrityOfObfuscatedCircuit();
    
    generator->circuitData.scir = transformCircuitToShrinkedCircuit(generator->circuitData.circuit);
    
    if(generator->network!="off") {
        emp::NetIO * io = new emp::NetIO( nullptr, generator->port); //generator doesn't need server address
        generator->writer = new Writer<emp::NetIO>(io);
        generator->writeCircuit(generator->network);
        delete io;
    }

    if(generator->disk!="off") {
        std::string filepath = CIRCUITPATH + generator->circuitName+(generator->disk=="bin"? ".bin" : "_compressed.dat");
        emp::FileIO *fio = new emp::FileIO( filepath.c_str(),false );
        generator->writer = new Writer<emp::FileIO>(fio);
        generator->writeCircuit(generator->disk); 
        delete fio;
    }
    delete generator;
    return 0;
}
