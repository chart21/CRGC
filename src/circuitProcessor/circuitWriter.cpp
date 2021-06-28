#include "include/circuitWriter.h"
#include "include/circuitStructs.h"
#include <iostream>
#include <fstream>


void exportCircuitSeparateFiles(TransformedCircuit* circuit, std::string destinationPath)
{
    std::ofstream detailsFile (destinationPath + "_rgc_details.txt");

    detailsFile << circuit->details.numGates << ' ' << circuit->details.numWires << '\n';
    detailsFile << circuit->details.bitlengthInputA << ' ' << circuit->details.bitlengthInputB << '\n';
    detailsFile << circuit->details.numOutputs << ' ' << circuit->details.bitlengthOutputs;

    detailsFile.close();

    std::ofstream circuitFile (destinationPath + "_rgc.txt");

    for(auto i = 0; i < circuit->details.numGates; i++)
    {
        circuitFile << circuit->gates[i].leftParentID << ' ';
        circuitFile << circuit->gates[i].rightParentID << ' ';
        circuitFile << circuit->gates[i].outputID << ' ';
        circuitFile << circuit->gates[i].truthTable[0][0];
        circuitFile << circuit->gates[i].truthTable[0][1];
        circuitFile << circuit->gates[i].truthTable[1][0];
        circuitFile << circuit->gates[i].truthTable[1][1];
        circuitFile << '\n';          
    }
    circuitFile.close();

}

void exportObfuscatedInput(bool* valArr, CircuitDetails details, std::string destinationPath)
{
    std::ofstream inputFile (destinationPath + "_rgc_inputA.txt");
    for (auto i = 0; i < details.bitlengthInputA; i++)
    {
        inputFile << valArr[i];
    }
    inputFile.close();
    
} 


