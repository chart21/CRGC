#include "include/circuitLinker.h"
#include <vector>
#include "include/mult3.h"
#include "include/paperoptm.h"




void generateCircuitRAM(std::vector<BristolGate>* gateVec, CircuitDetails* details, bool print, std::string programName)
{
    if (programName == "mult3")
        generateCircuitRAMMult(gateVec, details, print);
    else if (programName == "paperoptm")
        generateCircuitRAMPaper(gateVec, details, print);
    else
        std::cout << "Program not found, make sure you added it in CircuitLinker.cpp." << '\n';

}