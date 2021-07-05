#include <emp-tool/emp-tool.h>
#include "include/plain_circ_adj.h"
#include "include/plain_prot_adj.h"
#include "include/circuitStructs.h"
#include <string>

void test_mult3(int bitsize, std::string inputs_a[], std::string inputs_b[]);
void generateCircuit();
void generateCircuitRAM(std::vector<BristolGate>* gateVec, CircuitDetails* details);