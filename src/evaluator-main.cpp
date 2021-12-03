#include "party.h"

using std::string;

class Evaluator : public Party {
public:
    std::string ip_address = IP;

    void loadTransferredCircuit(ShrinkedCircuit* scir, bool *valArr);
    void loadTransferredCircuit(TransformedCircuit* cir, bool *valArr);
    void readCircuit(string format,string print);

    void evaluateObfuscatedCircuit();
};

void Evaluator::loadTransferredCircuit(ShrinkedCircuit* scir, bool *valArr)
{
    circuitData.scir = scir;
    circuitData.circuit = transformShrinkedCircuitToTransformedCircuit(circuitData.scir);
    circuitData.inputA = new bool[circuitData.circuit->details.bitlengthInputA];
    circuitData.inputB = new bool[circuitData.circuit->details.bitlengthInputB];
    circuitData.output = new bool[circuitData.circuit->details.numOutputs * circuitData.circuit->details.bitlengthOutputs];
    circuitData.obfuscatedValArr = valArr;
}

//Only used to import a CRGC stored as a txt file
void Evaluator::loadTransferredCircuit(TransformedCircuit* cir, bool *valArr)
{
    circuitData.circuit = cir;
    circuitData.scir = transformCircuitToShrinkedCircuit(circuitData.circuit);
    circuitData.inputA = new bool[circuitData.circuit->details.bitlengthInputA];
    circuitData.inputB = new bool[circuitData.circuit->details.bitlengthInputB];
    circuitData.output = new bool[circuitData.circuit->details.numOutputs * circuitData.circuit->details.bitlengthOutputs];
    circuitData.obfuscatedValArr = valArr;
}

void Evaluator::evaluateObfuscatedCircuit(){
    if(circuitFormat == "emp" || fileFormat == "cpp")
        funcTime("evaluate circuit", evaluateSortedTransformedCircuit, circuitData.circuit, circuitData.obfuscatedValArr, circuitData.inputB, circuitData.output);
    else
        funcTime("evaluate circuit", evaluateTransformedCircuit, circuitData.circuit, circuitData.obfuscatedValArr, circuitData.inputB, circuitData.output);

    auto inA = convertBoolArrToInt(circuitData.obfuscatedValArr, circuitData.circuit->details.bitlengthInputA);
    auto inB = convertBoolArrToInt(circuitData.inputB, circuitData.circuit->details.bitlengthInputB);
    auto iout = convertBoolArrToInt(circuitData.output, circuitData.circuit->details.bitlengthOutputs);
    std::cout << "---Evaluation--- inA" << inA << "\n";
    std::cout << "---Evaluation--- inB" << inB << "\n";
    std::cout << "---Evaluation--- out" << iout << "\n";
}

void forwardImportFunctions(Evaluator *eva, ShrinkedCircuit* &scir, bool* &valArr, bool bin){
    if(bin)
        eva->reader->importBin(scir, valArr);
    else
        eva->reader->importCompressedCircuit(scir,valArr);
    return;
}

void Evaluator::readCircuit(string format,string print) {
    bool *valArr;
    ShrinkedCircuit *scir;
    TransformedCircuit *cir;

    if (format == "off")
        return;
    if ( format == "bin" || format == "uncompressed"){
        funcTime( print, forwardImportFunctions, this, scir, valArr, true);
        this->loadTransferredCircuit(scir,valArr);
    }
    else if(format=="compressed"){
        funcTime( print, forwardImportFunctions, this, scir, valArr, false);
        this->loadTransferredCircuit(scir,valArr);
    }
    else if(format=="txt") {
        string filepath = CIRCUITPATH + this->circuitName;
        CircuitDetails details = importBristolCircuitDetails(filepath+"_rgc_details.txt","rgc");
        cir = importTransformedCircuit(filepath+"_rgc.txt", details);
        importObfuscatedInput(valArr, details, filepath);
        this->loadTransferredCircuit(cir,valArr);
    }
}

void parseEvaluatorCLIOptions(int argc, char *argv[], Evaluator* party, vector<char*> &inputs){
    static struct option long_options[] =
    {
        {"circuit", required_argument, NULL, 'c'},
        {"inputa", required_argument, NULL, 'a'},
        {"inputb", required_argument, NULL, 'b'},
        {"port", required_argument, NULL, 'p'},
        {"ip", required_argument, NULL, 'i'},
        {"compression", required_argument, NULL, 'k'},
        {"network", required_argument, NULL, 'y'},
        {"store", required_argument, NULL, 'z'},
        {NULL, 0, NULL, 0}
    };
    int opt = getopt_long(argc, argv, "c:a:b:p:i:k:y:z:", long_options, NULL);
    while (opt != -1) {
        switch (opt) {
        case 'c':
            party->circuitName = optarg;
            break;

        case 'a':
            inputs[0] = optarg;
            break;

        case 'b':
            inputs[1] = optarg;
            break;

        case 'p':
            party->port = std::stoi(optarg);
            break;

        case 'i':
            party->ip_address = optarg;
            break;

        case 'k':
            party->compressThreads = std::stoi(optarg);
            break;
        
        case 'y':
            party->network = optarg;
            break;

        case 'z':
            party->store = optarg;
            break;

        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
        opt = getopt_long(argc, argv, "c:a:b:p:i:k:y:z:", long_options, NULL);
    }
    
}

int main(int argc, char *argv[])
{

    Evaluator* evaluator = new Evaluator();
    int port = 8080;
    vector<char*> inputs(2,nullptr);
    parseEvaluatorCLIOptions(argc, argv, evaluator,inputs);
    if(evaluator->network=="off") {
        std::string filepath = CIRCUITPATH + evaluator->circuitName+(evaluator->store=="bin"? ".bin" : "_compressed.dat");
        emp::FileIO *fio = new emp::FileIO( filepath.c_str(),true );
        evaluator->reader = new Reader<emp::FileIO>(fio,true);
        evaluator->readCircuit(evaluator->store,"importing"); 
        delete fio;
        parseCLIArguments(inputs,evaluator);
        evaluator->evaluateObfuscatedCircuit();
        return 0;
        
    }
    else{
        emp::NetIO * io = new emp::NetIO( evaluator->ip_address.c_str(), PORT);
        //Reader<emp::NetIO> * r = new Reader<emp::NetIO>(io);
        evaluator->reader = new Reader<emp::NetIO>(io);
        evaluator->readCircuit(evaluator->network,"receiving"); 
        delete io;
        parseCLIArguments(inputs,evaluator);
        evaluator->evaluateObfuscatedCircuit();
    }
    

    if(evaluator->store=="off") return 0; 

    std::string filepath = CIRCUITPATH + evaluator->circuitName+(evaluator->store=="bin"? ".bin" : "_compressed.dat");
    emp::FileIO *fio = new emp::FileIO( filepath.c_str(),false );
    evaluator->writer = new Writer<emp::FileIO>(fio,true);
    evaluator->writeCircuit(evaluator->store,"exporting",evaluator->compressThreads); 
    delete fio;
    

    delete evaluator;
    return 0;
}
