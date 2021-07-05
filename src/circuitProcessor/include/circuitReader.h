#include "circuitStructs.h"
#include <fstream>
#include<vector>


void splitString(std::string s, std::vector<std::string> &v);	





CircuitDetails importBristolCircuitDetails(std::string filepath, std::string circuitFormat="bristol");
   

TransformedCircuit* importBristolCircuitExNot(std::string filepath, CircuitDetails details);

BristolCircuit* importBristolCircuitExNotForLeakagePrediction(std::string filepath, CircuitDetails details);

BristolCircuit* importBristolCircuitExNotForLeakagePredictionFromRAM(std::vector<BristolGate>* gateVec, CircuitDetails details);

void importInput(std::string filepath, uint_fast64_t bitlength, bool* valArr);

TransformedCircuit* importTransformedCircuit(std::string filepath, CircuitDetails details);

void transformBristolCircuitToTransformedCircuit(BristolCircuit *bristolCircuit, TransformedCircuit *circuit);