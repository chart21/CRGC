#include "include/circuitLinker.h"
#include "include/circuitStructs.h"
#include "emp-sh2pc/emp-sh2pc.h"
#include "include/plain_circ_adj.h"
#include "include/plain_prot_adj.h"
#include "include/examplePrograms.h"
// include your header here
// #include "include/YourHeader.h"

#include <vector>
#include <string>
#include <iostream>





void generateCircuitRAM(std::vector<BristolGate> *gateVec, CircuitDetails *details, bool print, std::string programName)
{
    std::string circuitName = programName + ".txt";
    setup_plain_prot_adj(print, circuitName, gateVec, details);  
    
    
    if (programName == "mult3")
        mult3();
    else if (programName == "setIntersect")
        setIntersect();
    else if (programName == "query")
        query();
    else if (programName == "maxIn2DArr")
        maxIn2DArr();  
    else if (programName == "insurance")
        insurance();
    else if (programName == "billboardAd")
        billboardAd(); 

    // add additional programs here
    //  else if (programName == "YourProgram")
    
    
    else
        std::cout << "Program not found, make sure you added it in CircuitLinker.cpp." << '\n';


    finalize_plain_prot_adj(print, gateVec, details);
}