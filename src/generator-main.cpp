#include <iostream>
#include <getopt.h>
#include <vector>

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
#include "circuitProcessor/circuitReader.cpp"
#include "circuitProcessor/circuitWriter.cpp"
#include "circuitProcessor/circuitCompressor.cpp"

#include "circuitProcessor/include/leakagePredictor.h"

#include "circuitProcessor/include/circuitHighSpeedNetIO.h"
#include "circuitProcessor/include/circuitNetIO.h"

#include <emp-tool/emp-tool.h>
// #include "programs/include/mult3.h"
// #include "programs/include/paperoptm.h"
#include "programs/include/examplePrograms.h"
#include "programs/include/circuitLinker.h"

#include <chrono>
//#define DEBUG
#define time_S t1 = startClock();
#define time_E stopClock(t1);

//#define CIRCUITPATH "/home/christopher/Documents/C-RGCG/src/circuits/"
#define CIRCUITPATH "../src/circuits/"
#define PORT 8080

/*
template <typename F, typename... Args>
auto funcTime(std::string printText, F func, Args &&...args)
{
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    func(std::forward<Args>(args)...);
#ifdef US
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - t1)
                    .count();
    std::cout << "---TIMING--- " << time << "us " << printText << '\n';
#else
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - t1)
                    .count();
    std::cout << "---TIMING--- " << time << "ms " << printText << '\n';
#endif
    return time;
}
*/
struct Agency{
    uint_fast64_t numThreads = 1; 
    int compressThreads = 1;    
    std::string fileFormat = "cpp";
    std::string circuitName = "adder64";
    std::string circuitFormat = "bristol";
    std::string transferFormat = "bin";
    int party = 0;

    bool network = true;

    TransformedCircuit *circuit = nullptr;
    ShrinkedCircuit *scir = nullptr;
    bool *inputA = nullptr;
    bool *inputB = nullptr;
    bool *output = nullptr;
    bool *obfuscatedValArr = nullptr;

    ~Agency(){
        if(scir) delete scir;
        if(circuit) delete circuit;
        if(inputA) delete [] inputA;
        if(inputB) delete [] inputB;
        if(output) delete [] output;
        if(obfuscatedValArr) delete [] obfuscatedValArr;
    }
    void loadTransformedCircuit();
    void loadTransferredCircuit(ShrinkedCircuit* scir, bool *valArr);
    void loadTransferredCircuit(TransformedCircuit* cir, bool *valArr);
    void predictLeakage(uint_fast64_t *parents);
    void evaluateCircuit();
    void obfuscateCircuit(uint_fast64_t *parents);
    void verifyIntegrityOfObfuscatedCircuit();
    void verifyIntegrityOfExportedRGC();
    void evaluateObfuscatedCircuit();
    void importCircuit();
    void exportCircuit();
};

// void generateCircuitRAM(std::vector<BristolGate> *gateVec, CircuitDetails *details, bool print, std::string programName)
// {
//     if (programName == "mult3")
//         generateCircuitRAMMult(gateVec, details, print);

//     else if (programName == "paperoptm")
//         generateCircuitRAMPaper(gateVec, details, print);
//     else
//         std::cout << "Program not found, make sure you added it in CircuitLinker.cpp." << '\n';
// }

void Agency::loadTransformedCircuit()
{

    if (fileFormat == "cpp")
    {
        Eva<emp::FileIO> *local = new Eva<emp::FileIO>();
        std::vector<BristolGate> gateVec;
        CircuitDetails empDetails;
        //funcTime("converting program to circuit", generateCircuitRAMPaper, &gateVec, &empDetails);
        funcTime("converting program to circuit", generateCircuitRAM, &gateVec, &empDetails, false, circuitName);
        //auto flipped = new bool[empDetails.numWires];
        //auto bristolCircuit = importBristolCircuitExNotForLeakagePredictionFromRAM(&gateVec, empDetails, flipped);
        
        this->circuit = local->importTransformedCircuitExNotForLeakagePredictionFromRAM(&gateVec, empDetails);
        std::vector<BristolGate>().swap(gateVec); //deallocate memory of vector
        std::cout << "---INFO--- numGates: " << this->circuit->details.numGates << '\n';
        //auto unflippedCircuit = new UnflippedCircuit{bristolCircuit, flipped};
        delete local;
        // return circuit;
    }
    else if (fileFormat == "txt")
    {
        std::string filepath = CIRCUITPATH + circuitName;
        auto details = importBristolCircuitDetails(filepath + ".txt", circuitFormat);
        //auto flipped = new bool[details.numWires];
        //auto bristolCircuit = importBristolCircuitExNotForLeakagePrediction(filepath + ".txt", details, flipped);
        this->circuit = importBristolCircuitExNot(filepath + ".txt", details);
        //auto unflippedCircuit = new UnflippedCircuit{bristolCircuit, flipped};
        // return circuit;
    }
    this->inputA = new bool[this->circuit->details.bitlengthInputA];
    this->inputB = new bool[this->circuit->details.bitlengthInputB];
    this->output = new bool[this->circuit->details.numOutputs * this->circuit->details.bitlengthOutputs];
    this->obfuscatedValArr = new bool[this->circuit->details.bitlengthInputA];
}

void Agency::loadTransferredCircuit(ShrinkedCircuit* scir, bool *valArr)
{
    this->scir = scir;
    this->circuit = transformShrinkedCircuitToTransformedCircuit(this->scir);
    this->inputA = new bool[this->circuit->details.bitlengthInputA];
    this->inputB = new bool[this->circuit->details.bitlengthInputB];
    this->output = new bool[this->circuit->details.numOutputs * this->circuit->details.bitlengthOutputs];
    this->obfuscatedValArr = valArr;
}

void Agency::loadTransferredCircuit(TransformedCircuit* cir, bool *valArr)
{
    this->circuit = cir;
    this->scir = transformCircuitToShrinkedCircuit(this->circuit);
    this->inputA = new bool[this->circuit->details.bitlengthInputA];
    this->inputB = new bool[this->circuit->details.bitlengthInputB];
    this->output = new bool[this->circuit->details.numOutputs * this->circuit->details.bitlengthOutputs];
    this->obfuscatedValArr = valArr;
}

void Agency::predictLeakage(uint_fast64_t *parents)
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
    std::cout << "---INFO--- potentially obfuscated fixed and intermediary gates: " << poc << '\n';

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

void Agency::evaluateCircuit()
{
    //auto circuit = importBristolCircuitExNot(filepath+ ".txt", details);

    std::string filepath = CIRCUITPATH + circuitName;

    if(circuitFormat == "emp" || fileFormat == "cpp")
        funcTime("evaluate circuit", evaluateSortedTransformedCircuit, this->circuit, this->inputA, this->inputB, this->output);
    else
        funcTime("evaluate circuit", evaluateTransformedCircuit, this->circuit, this->inputA, this->inputB, this->output);


    int inA = convertBoolArrToInt(this->inputA, circuit->details.bitlengthInputA);
    int inB = convertBoolArrToInt(this->inputB, circuit->details.bitlengthInputB);
    int iout = convertBoolArrToInt(this->output, circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";
}

void Agency::obfuscateCircuit(uint_fast64_t *parents)
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
    std::cout << "---INFO--- obfuscated fixed and intermediary gates: " << poc << '\n';

    if (numThreads == 1)
        funcTime("obfuscate gates", regenerateGates, circuit, isObfuscated);
    else
        funcTime("obfuscate gates", regenerateGatesMT, circuit, isObfuscated, numThreads);

    delete[] isObfuscated;
}

void Agency::verifyIntegrityOfObfuscatedCircuit()
{
    auto outputRGC = new bool[this->circuit->details.bitlengthOutputs * circuit->details.numOutputs];
    
    
    evaluateTransformedCircuit(this->circuit, obfuscatedValArr, inputB, outputRGC);
    

    if (equalBoolArr(outputRGC, this->output, this->circuit->details.bitlengthOutputs))
        std::cout << "---Success--- Evaluation of original circuit and constructed RGC are equal" << '\n';
    else
        std::cout << "---Warning--- Evaluation of original circuit and constructed RGC are not equal" << '\n';

    auto inA = convertBoolArrToInt(obfuscatedValArr, this->circuit->details.bitlengthInputA);
    auto inB = convertBoolArrToInt(inputB, this->circuit->details.bitlengthInputB);
    auto iout = convertBoolArrToInt(outputRGC, this->circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";

    delete[] outputRGC;
}

void Agency::verifyIntegrityOfExportedRGC()
{
    std::string filepath = CIRCUITPATH;

    auto newDetails = importBristolCircuitDetails(filepath + "_rgc_details.txt", "rgc");
    auto newCircuit = importTransformedCircuit(filepath + "_rgc.txt", newDetails);

    auto newInputA = new bool[newCircuit->details.bitlengthInputA];
    importBinaryInput(filepath + "_rgc_inputA.txt", newCircuit->details.bitlengthInputA, newInputA);
    auto newOut2 = new bool[newCircuit->details.bitlengthOutputs * newCircuit->details.numOutputs];

    evaluateTransformedCircuit(newCircuit, newInputA, inputB, newOut2);

    if (equalBoolArr(output, newOut2, circuit->details.bitlengthOutputs))
        std::cout << "---Success--- Evaluation of original circuit and constructed RGC are equal" << '\n';
    else
        std::cout << "---Warning--- Evaluation of original circuit and constructed RGC are not equal" << '\n';

    auto inA = convertBoolArrToInt(newInputA, newDetails.bitlengthInputA);
    auto inB = convertBoolArrToInt(inputB, newDetails.bitlengthInputB);
    auto iout2 = convertBoolArrToInt(newOut2, newDetails.bitlengthOutputs);
}

void Agency::evaluateObfuscatedCircuit(){
    // evaluateTransformedCircuit(this->circuit, this->obfuscatedValArr, this->inputB, this->output);
    if(circuitFormat == "emp" || fileFormat == "cpp")
        funcTime("evaluate circuit", evaluateSortedTransformedCircuit, this->circuit, this->obfuscatedValArr, this->inputB, this->output);
    else
        funcTime("evaluate circuit", evaluateTransformedCircuit, this->circuit, this->obfuscatedValArr, this->inputB, this->output);

    auto inA = convertBoolArrToInt(this->obfuscatedValArr, this->circuit->details.bitlengthInputA);
    auto inB = convertBoolArrToInt(this->inputB, this->circuit->details.bitlengthInputB);
    auto iout = convertBoolArrToInt(this->output, this->circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";
}

/* following 2 funcs, need data in agency for export circuit and inputs, while have no relation to agency when import */

template <typename T>
void forwarderEx(Gen<T> *obj, const Agency* agency, bool bin){
    if(bin) 
        obj->exportBin(agency->scir,agency->obfuscatedValArr);
    else {
        obj->exportCompressedCircuit(agency->scir,agency->obfuscatedValArr,agency->compressThreads);
        // obj->exportCompressedCircuit(agency->scir,agency->obfuscatedValArr);
    }
    //obj->exportObfuscatedInput(agency->obfuscatedValArr,agency->scir->details,inApath);
    return;
}

template <typename T>
void forwarderIm(Eva<T> *obj, ShrinkedCircuit* &scir, bool* &valArr, int thr, bool bin){
    if(bin)
        obj->importBin(scir,valArr);
    else
        obj->importCompressedCircuit(scir,valArr);
    
    //obj->importObfuscatedInput(valArr, scir->details,inApath);
    return;
}

void compressBenchmark( Agency* &agency){
    bool bin = agency->transferFormat=="bin";
    ShrinkedCircuit* imported;
    //int circuitThread = 10;
    uint64_t ctime=0;
    uint64_t dtime=0;
    int len = bin==true?1:1;
    std::string filepath = CIRCUITPATH + agency->circuitName + (bin==true? ".bin" : "_compressed.dat");
    std::string intApath = CIRCUITPATH + agency->circuitName;
    for(int i=0;i<len;i++){
        emp::FileIO *cio = new emp::FileIO( filepath.c_str(),false );
        Gen<emp::FileIO> *gen = new Gen<emp::FileIO>(cio);
        ctime += funcTime( "compress", forwarderEx<emp::FileIO>, gen, agency, bin);
        cio->flush();
        delete cio;
        delete gen;
        cout<<"finish compress"<<endl;

        emp::FileIO *dio = new emp::FileIO( filepath.c_str(),true );
        Eva<emp::FileIO> *eva = new Eva<emp::FileIO>(dio);
        bool* valArr = nullptr;
        dtime += funcTime( "decompress", forwarderIm<emp::FileIO>, eva, imported, valArr, agency->compressThreads, bin);
        dio->flush();
        if(valArr) delete [] valArr;
        delete dio;
        delete eva;
    }
    cout << areShrinkedCircuitsEqual(imported, agency->scir) << ": " << ctime/len << ", " << dtime/len << endl;
}

void Agency::exportCircuit(){
    std::string filepath = CIRCUITPATH + this->circuitName;
    bool bin = this->transferFormat=="bin";
    if(this->network){
        emp::NetIO * io = new emp::NetIO( nullptr, PORT); //generator doesn't need server address
        Gen<emp::NetIO> *gen = new Gen<emp::NetIO>(io);
        funcTime( "send", forwarderEx<emp::NetIO>, gen, this, bin);
        delete io;
        delete gen;
    }
    else if(this->transferFormat=="txt"){
            exportCircuitSeparateFiles(this->circuit, filepath);
    }
    else {
        filepath += (bin==true? ".bin" : "_compressed.dat");
        emp::FileIO *fio = new emp::FileIO( filepath.c_str(),false );
        Gen<emp::FileIO> *gen = new Gen<emp::FileIO>(fio);
        funcTime( "compress", forwarderEx<emp::FileIO>, gen, this, bin);
        fio->flush();
        delete fio;
        delete gen;
    }
}

void Agency::importCircuit()
{
    bool* valArr;
    ShrinkedCircuit *scir;
    TransformedCircuit *cir;
    std::string filepath = CIRCUITPATH + this->circuitName;
    bool bin = this->transferFormat=="bin";
    if(this->network){
        emp::NetIO * io = new emp::NetIO( "127.0.0.1", PORT); //assume server as local
        Eva<emp::NetIO> *eva = new Eva<emp::NetIO>(io);
        funcTime( "receive", forwarderIm<emp::NetIO>, eva, scir, valArr, this->compressThreads, bin);
        this->loadTransferredCircuit(scir,valArr);
            //exportCircuitSeparateFiles(agency->circuit,CIRCUITPATH+agency->circuitName+"2");
        delete io;
        delete eva;
    }
    else if(this->transferFormat=="txt") {
        CircuitDetails details = importBristolCircuitDetails(filepath+"_rgc_details.txt","rgc");
        cir = importTransformedCircuit(filepath+"_rgc.txt", details);
        importObfuscatedInput(valArr, details, filepath);
        this->loadTransferredCircuit(cir,valArr);
    }
    else{
        filepath += (bin==true? ".bin" : "_compressed.dat");

        emp::FileIO *fio = new emp::FileIO( filepath.c_str(),true );
        Eva<emp::FileIO> *eva = new Eva<emp::FileIO>(fio);
        funcTime( "read", forwarderIm<emp::FileIO>, eva, scir, valArr, this->compressThreads, bin);
        this->loadTransferredCircuit(scir,valArr);
        delete fio;
        delete eva;
    }
}

void parse( int argc, char *argv[], Agency* &agency){
    if (argc > 1)
    {
        agency->circuitName = argv[1];
    }

    if (argc > 2)
    {
        agency->fileFormat = argv[2];
    }

    
    if (argc > 5)
    {
        agency->circuitFormat = argv[5];
    }

    if (argc > 6)
    {
        agency->numThreads = std::stoul(argv[6]);
    }

    if (argc > 7)
    {
        agency->party = std::stoi(argv[7]);
    }

    if (argc > 8)
    {
        agency->compressThreads = std::stoi(argv[8]);
    }


}

void parseOption(int argc, char *argv[], Agency* &agency, vector<char*> &inputs){
    static struct option long_options[] =
    {
        {"circuit", required_argument, NULL, 'c'},
        {"type", required_argument, NULL, 't'},
        {"inputa", required_argument, NULL, 'a'},
        {"inputb", required_argument, NULL, 'b'},
        {"format", required_argument, NULL, 'f'},
        {"thread", required_argument, NULL, 'n'},
        {"party", required_argument, NULL, 'p'},
        {"compression", required_argument, NULL, 'k'},
        {"bin", no_argument, NULL, 'x'},
        {"compress", no_argument, NULL, 'y'},
        {"txt", no_argument, NULL, 'z'},
        {"disk", no_argument, NULL, 'w'},
        {NULL, 0, NULL, 0}
    };
    int opt = getopt_long(argc, argv, "c:t:a:b:f:p:n:k:xyzw", long_options, NULL);
    while (opt != -1) {
        switch (opt) {
        case 'c':
            agency->circuitName = optarg;
            break;

        case 't':
            agency->fileFormat = optarg;
            break;

        case 'a':
            inputs[0] = optarg;
            break;

        case 'b':
            inputs[1] = optarg;
            break;

        case 'f':
            agency->circuitFormat = optarg;
            break;

        case 'n':
            agency->numThreads = std::stoul(optarg);
            break;

        case 'p':
            agency->party = std::stoi(optarg);
            break;

        case 'k':
            agency->compressThreads = std::stoi(optarg);
            break;

        case 'w':
            agency->network = false;
            break;

        case 'x':
            agency->transferFormat = "bin";
            break;
        
        case 'y':
            agency->transferFormat = "compress";
            break;

        case 'z':
            agency->transferFormat = "txt";
            break;

        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
        opt = getopt_long(argc, argv, "c:t:a:b:f:p:n:k:", long_options, NULL);
    }
    
}
void parseOptionInput(vector<char*> inputs, Agency* &agency){
    std::string filepath = CIRCUITPATH + agency->circuitName;
    if (inputs[0])
    {
        uint_fast64_t a;
        if (check_number(inputs[0]))
        {
            uint_fast64_t a = std::stoul(inputs[0]);
            converIntToBoolArr(a, agency->circuit->details.bitlengthInputA, agency->inputA);
        }
        else
        {
            if (inputs[0] == std::string("r"))
                generateRandomInput(agency->circuit->details.bitlengthInputA, agency->inputA);
            else
                importBinaryInput(filepath + "_inputA.txt", agency->circuit->details.bitlengthInputA, agency->inputA);
        }
    }
    else
        generateRandomInput(agency->circuit->details.bitlengthInputA, agency->inputA);

    if (inputs[1])
    {
        uint_fast64_t b;
        if (check_number(inputs[1]))
        {
            uint_fast64_t b = std::stoul(inputs[1]);
            converIntToBoolArr(b, agency->circuit->details.bitlengthInputB, agency->inputB);
        }
        else
        {
            if (inputs[1] == std::string("r"))
                generateRandomInput(agency->circuit->details.bitlengthInputB, agency->inputB);
            else
                importBinaryInput(filepath + "_inputB.txt", agency->circuit->details.bitlengthInputB, agency->inputB);
        }
    }
    else
        generateRandomInput(agency->circuit->details.bitlengthInputB, agency->inputB);
}

void parseInput(int argc, char *argv[], Agency* &agency){
    std::string filepath = CIRCUITPATH + agency->circuitName;
    if (argc > 3)
    {
        uint_fast64_t a;
        if (check_number(argv[3]))
        {
            uint_fast64_t a = std::stoul(argv[3]);
            converIntToBoolArr(a, agency->circuit->details.bitlengthInputA, agency->inputA);
        }
        else
        {
            if (argv[3] == std::string("r"))
                generateRandomInput(agency->circuit->details.bitlengthInputA, agency->inputA);
            else
                importBinaryInput(filepath + "_inputA.txt", agency->circuit->details.bitlengthInputA, agency->inputA);
        }
    }
    else
        generateRandomInput(agency->circuit->details.bitlengthInputA, agency->inputA);

    if (argc > 4)
    {
        uint_fast64_t b;
        if (check_number(argv[4]))
        {
            uint_fast64_t b = std::stoul(argv[4]);
            converIntToBoolArr(b, agency->circuit->details.bitlengthInputB, agency->inputB);
        }
        else
        {
            if (argv[4] == std::string("r"))
                generateRandomInput(agency->circuit->details.bitlengthInputB, agency->inputB);
            else
                importBinaryInput(filepath + "_inputB.txt", agency->circuit->details.bitlengthInputB, agency->inputB);
        }
    }
    else
        generateRandomInput(agency->circuit->details.bitlengthInputB, agency->inputB);
}

int main(int argc, char *argv[])
{

    Agency* agency = new Agency();
    int port = 8080;
    vector<char*> inputs(2,nullptr);
    parseOption(argc, argv, agency,inputs);

    
    if(agency->party!=2){
    /* to do: for cpp format, load input */
        agency->loadTransformedCircuit();

        auto parents = new uint_fast64_t[agency->circuit->details.numWires * 2]();
        agency->predictLeakage(parents);

        //parseInput(argc, argv, agency);
        parseOptionInput(inputs,agency);
        agency->evaluateCircuit();
        
        agency->obfuscateCircuit(parents);
        
        agency->verifyIntegrityOfObfuscatedCircuit();

        //exportCircuitSeparateFiles(agency->circuit,CIRCUITPATH+agency->circuitName+"1");
        agency->scir = transformCircuitToShrinkedCircuit(agency->circuit);
        
    }

    if( agency->party==0 ){
        compressBenchmark( agency);
        return 0;
    }
    // emp::HighSpeedNetIO * io = new emp::HighSpeedNetIO(party==1 ? nullptr : "192.168.23.100", 6112, 8080); //assume server as local   
    if( agency->party==1 ){
        agency->exportCircuit();
    }
    else {
        agency->importCircuit();  
        //parseInput(argc, argv, agency);
        parseOptionInput(inputs,agency);
        agency->evaluateObfuscatedCircuit();

    }
    //delete io;
    delete agency;
    // auto originalCircuit = loadTransformedCircuit(circuitName, fileFormat, circuitFormat);
    // funcTime("compare circuit similarity", compareCircuitSimilarityMT, originalCircuit, circuit, numThreads);
    //std::cout<<areCircuitsEqual(circuit,originalCircuit)<<std::endl;
    //deleteRevealGates(circuit, circuitLineOfWireIndex);

    //exportCircuitSeparateFiles(circuit, CIRCUITPATH);
    //exportObfuscatedInput(obfuscatedValArr, circuit->details, CIRCUITPATH);

    //verifyIntegrityOfExportedRGC(circuit, output, inputB, numThreads);

    return 0;
}
