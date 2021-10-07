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
typedef Gate<uint_fast8_t> UintGate;



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
Circuit(CircuitDetails dts=CircuitDetails()):details(dts){
    gates = new T[dts.numGates];
}
~Circuit(){delete [] gates;}
};

typedef Circuit<TransformedGate> TransformedCircuit;
typedef Circuit<BristolGate> BristolCircuit;
typedef Circuit<UintGate> UintCircuit;

struct ShrinkedGate
{
    uint_fast64_t leftParentID;
    uint_fast64_t rightParentOffset;
    bool truthTable[2][2];
};

struct ShrinkedCircuit
{
    CircuitDetails details;
    ShrinkedGate* gates;
    ShrinkedCircuit(CircuitDetails dts=CircuitDetails()):details(dts){
        gates = new ShrinkedGate[dts.numGates];
    }
    ~ShrinkedCircuit(){delete [] gates;}
};


#endif