#include <iostream>
//#include "circuitProcessor/include/circuitStructs.h"
#include "circuitProcessor/include/circuitReader.h"
#include "circuitProcessor/include/circuitEvaluator.h"
//#include <./circuitProcessor/include/circuitStructs.h>
#include "circuitProcessor/include/helperFunctions.h"









int main(int argc, char *argv[])
{
    std::string circuitName = "adder64";
    if (argc > 1)
    {
        circuitName = argv[1]; 
    }
    std::string filepath = "./circuits/" +circuitName+ ".txt";

    auto details = getBristolCircuitDetails(filepath);


    auto inputA = generateRandomInput(details.bitlengthInputA);
    auto inputB = generateRandomInput(details.bitlengthInputA);
    if (argc > 2)
    {

        int a = std::stoi(argv[2]);
        inputA = converIntToBoolArr(a, details.bitlengthInputA);
    }
    if (argc > 3)
    {
        int b = std::stoi(argv[2]);
        inputB = converIntToBoolArr(b, details.bitlengthInputB);
    }

    auto circuit = readBristolCircuitExNot(filepath, details);
    auto out = evaluateTransformedCircuitN(circuit, inputA, inputB);

    

    
    return 0;
}
