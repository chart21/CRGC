#include <emp-tool/emp-tool.h>
//#include "emp-sh2pc/emp-sh2pc.h"
#include "include/plain_circ_adj.h"
#include "include/plain_prot_adj.h"
#include "include/circuitStructs.h"
#include <string>

using namespace emp;



void test_mult3(int bitsize, std::string inputs_a[], std::string inputs_b[]) {
  Integer product(bitsize, 1);


	Integer a[3];
	Integer b[3];
  for (int i=0; i<3; i++)
    a[i] = Integer(bitsize, stoi(inputs_a[i]), ALICE);

  for (int i=0; i<3; i++)
    b[i] = Integer(bitsize, stoi(inputs_b[i]), BOB);

	for( int i=0; i < 3; i++ ) {

    // reconstruct "secret shared" inputs
    a[i] = a[i] + b[i];
    // multiply value into product
    product = product * a[i];
  }

  std::cout << "Product (binary notation): ";
  for(int i=bitsize-1; i>=0; i--) {
     std::cout << product[i].reveal();
  }
  std::cout << endl;
}

void generateCircuitMult()
{
  int bitsize; 
  setup_plain_prot(true, "mult3.circuit.txt");
  bitsize = 16;
  string inputs[3] = {"0","0","0"};
  test_mult3(bitsize, inputs, inputs);
  finalize_plain_prot();
}

void generateCircuitRAMMult(std::vector<BristolGate>* gateVec, CircuitDetails* details, bool print)
{
  int bitsize; 
  setup_plain_prot_adj(print, "mult3.circuit.txt", gateVec, details);
  bitsize = 16;
  string inputs[3] = {"0","0","0"};
  test_mult3(bitsize, inputs, inputs);
  finalize_plain_prot_adj(print, gateVec, details);
}
