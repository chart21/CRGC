#include <emp-tool/emp-tool.h>
#include "plain_circ_adj.h"
#include "plain_prot_adj.h"
#include "circuitStructs.h"
#include <string>

void test_setIntersectAlt3(std::string inputs_a[], std::string inputs_b[]);

void generateCircuitRAMPaper(std::vector<BristolGate>* gateVec, CircuitDetails* details, bool print);