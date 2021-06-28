#ifndef circuitStructs
#define circuitStructs





#include <string>

template <typename T>
struct Gate
{
  	uint_fast64_t leftParentID;
	uint_fast64_t rightParentID;
    uint_fast64_t outputID;
    T truthTable;  
};

typedef Gate<bool[2][2]> TransformedGate;
typedef Gate<char> BristolGate;



struct CircuitDetails{
	uint_fast64_t numWires;
	uint_fast64_t numGates;
    uint_fast64_t numOutputs;
    uint_fast64_t bitlengthInputA;
    uint_fast64_t bitlengthInputB;    
    uint_fast64_t bitlengthOutputs;
};


template <typename T> 
struct Circuit{
CircuitDetails details;
T* gates;
};

typedef Circuit<TransformedGate> TransformedCircuit;
typedef Circuit<BristolGate> BristolCircuit;


#endif