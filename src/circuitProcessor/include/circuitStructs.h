#ifndef circuitStructs
#define circuitStructs





#include <string>

struct TransformedGate{
	int leftParentID;
	int rightParentID;
    int outputID;
    bool truthTable[2][2];
};


struct BristolGate{
	int leftParentID;
	int rightParentID;
    int outputID;
    std::string gateType;
};


struct CircuitDetails{
	int numWires;
	int numGates;
    int numOutputs;
    int bitlengthInputA;
    int bitlengthInputB;    
    int bitlengthOutputs;
};

struct TransformedCircuit{
CircuitDetails details;
TransformedGate* gates;
};

struct BristolCircuit{
CircuitDetails details;
TransformedGate* gates;
};


#endif