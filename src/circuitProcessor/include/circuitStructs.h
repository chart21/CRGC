#include <string>

typedef struct {
	int leftParentID;
	int rightParentID;
    int outPutID;
    bool truthTable[2][2];
}TransformedGate;


typedef struct {
	int leftParentID;
	int rightParentID;
    int outPutID;
    std::string gateType;
}BristolGate;


typedef struct {
	int numWires;
	int numGates;
    int numOutputs;
    int bitlengthInputA;
    int bitlengthInputB;    
    int bitlengthOutputs;
}CircuitDetails;

typedef struct {
CircuitDetails *details;
TransformedGate* gates;
}TransformedCircuit;

typedef struct {
CircuitDetails* details;
TransformedGate* gates;
}BristolCircuit;


