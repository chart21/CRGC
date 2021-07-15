#include <emp-tool/emp-tool.h>
#include "plain_circ_adj.h"
#include "plain_prot_adj.h"
#include "circuitStructs.h"
#include <string>

void test_mult3(int bitsize, std::string inputs_a[], std::string inputs_b[]);
void generateCircuitMult();
void generateCircuitRAMMult(std::vector<BristolGate>* gateVec, CircuitDetails* details, bool print);