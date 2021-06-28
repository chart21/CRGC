#include <string>

typedef struct {
	uint_fast64_t leftParentID;
	uint_fast64_t rightParentID;
    uint_fast64_t outputID;
    bool truthTable[2][2];
}TransformedGate;


typedef struct {
	uint_fast64_t leftParentID;
	uint_fast64_t rightParentID;
    uint_fast64_t outputID;
    std::string gateType;
}BristolGate;


typedef struct {
	uint_fast64_t numWires;
	uint_fast64_t numGates;
    uint_fast64_t numOutputs;
    uint_fast64_t bitlengthInputA;
    uint_fast64_t bitlengthInputB;    
    uint_fast64_t bitlengthOutputs;
}CircuitDetails;

typedef struct {
CircuitDetails details;
TransformedGate* gates;
}TransformedCircuit;

typedef struct {
CircuitDetails details;
TransformedGate* gates;
}BristolCircuit;


