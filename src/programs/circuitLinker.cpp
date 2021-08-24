#include "include/circuitLinker.h"
#include "include/mult3.h"
#include "include/paperoptm.h"

#include <vector>
#include <string>
#include <iostream>




void generateCircuitRAM(std::vector<BristolGate> *gateVec, CircuitDetails *details, bool print, std::string programName)
{
    if (programName == "mult3")
        generateCircuitRAMMult(gateVec, details, print);

    else if (programName == "paperoptm")
        generateCircuitRAMPaper(gateVec, details, print);
    // add additional programs here
    //  else if (programName == "YourProgram")
    else
        std::cout << "Program not found, make sure you added it in CircuitLinker.cpp." << '\n';
}