#include "circuitStructs.h"
#include <fstream>
#include<vector>


void splitString(std::string s, std::vector<std::string> &v);	


void swapLeftParent(bool (&arr)[2][2]);

void swapRightParent(bool (&arr)[2][2]);


CircuitDetails getBristolCircuitDetails(std::string filepath, std::string circuitFormat="bristol");
   

TransformedCircuit* readBristolCircuitExNot(std::string filepath, CircuitDetails details);
