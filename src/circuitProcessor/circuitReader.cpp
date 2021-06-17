#include "include/circuitStructs.h"
#include <fstream>
#include<vector>


void splitString(std::string s, std::vector<std::string> &v){
	
	std::string temp = "";
	for(int i=0;i<s.length();++i){
		
		if(s[i]==' '){
			v.push_back(temp);
			temp = "";
		}
		else{
			temp.push_back(s[i]);
		}
		
	}
	v.push_back(temp);
	
}

void swapLeftParent(bool (&arr)[2][2])
{
    auto temp = arr[0][0];
    arr[0][0] = arr[1][0];
    arr[1][0] = temp;

    auto temp = arr[0][1];
    arr[0][1] = arr[1][1];
    arr[1][1] = temp;

}

void swapRightParent(bool (&arr)[2][2])
{
    auto temp = arr[0][0];
    arr[0][0] = arr[0][1];
    arr[0][1] = temp;

    auto temp = arr[1][0];
    arr[1][0] = arr[1][1];
    arr[1][1] = temp;

}


CircuitDetails getBristolCircuitDetails(std::string filepath, std::string circuitFormat="bristol")
    {
        std::ifstream f(filepath);
        std::string line;
        int counter = 0;
        CircuitDetails details;
        //CircuitDetails details = {0,0,0,0,0,0};
        if(circuitFormat == "bristol")
        {   
            while (std::getline(f, line, '\n'))
            {
                std::vector<std::string> gateElements;
                splitString(line, gateElements );
                if(counter == 0)
                {
                    
                    details.numGates = std::stoi(gateElements[0]);
                    details.numWires = std::stoi(gateElements[1]);
                }


                else if(counter == 1)
                {
                    details.bitlengthInputA = std::stoi(gateElements[1]);
                    details.bitlengthInputB = std::stoi(gateElements[2]);
                }
            
                else if (counter == 2)
                {                
                    details.numOutputs = std::stoi(gateElements[0]);
                    details.bitlengthOutputs = std::stoi(gateElements[1]);
                }
                else if (counter > 2)
                    break;
                
                counter++;
            }
        }
        else if(circuitFormat == "emp")
        {   
            details.numOutputs = 1;
            while (std::getline(f, line, '\n'))
            {
                std::vector<std::string> gateElements;
                splitString(line, gateElements );
                if(counter == 0)
                {
                    
                    details.numGates = std::stoi(gateElements[0]);
                    details.numWires = std::stoi(gateElements[1]);
                }


                else if(counter == 1)
                {
                    details.bitlengthInputA = std::stoi(gateElements[0]);
                    details.bitlengthInputB = std::stoi(gateElements[1]);
                    details.bitlengthOutputs = std::stoi(gateElements[2]);
                }        

                else if (counter > 1)
                    break;
                
                counter++;
            }
        }
        else if(circuitFormat == "rgc")
        {   
            while (std::getline(f, line, '\n'))
            {
                std::vector<std::string> gateElements;
                splitString(line, gateElements );
                if(counter == 0)
                {
                    
                    details.numGates = std::stoi(gateElements[0]);
                    details.numWires = std::stoi(gateElements[1]);
                }


                else if(counter == 1)
                {
                    details.bitlengthInputA = std::stoi(gateElements[0]);
                    details.bitlengthInputB = std::stoi(gateElements[1]);
                }
            
                else if (counter == 2)
                {
                    details.numOutputs = std::stoi(gateElements[0]);
                    details.bitlengthOutputs = std::stoi(gateElements[1]);
                    
                }
                else if (counter > 2)
                    break;
                
                counter++;
            }
        }
    return details;
    }

TransformedCircuit* readBristolCircuitExNot(std::string filepath, CircuitDetails details)
{
    TransformedGate gates[details.numGates];
    int exchangeGate[details.numWires];
    bool flipped[details.numWires];
    for (int i = 0; i < details.numWires; i++)
    {
        exchangeGate[i] = i;
        flipped[i] = false;        
    }
    
    int gateCounter = 0;


    std::ifstream f(filepath);
    std::string line;
    while (std::getline(f, line, '\n'))
    {
        std::vector<std::string> gateElements;
        splitString(line, gateElements);
        if(gateElements.size() > 4 and gateElements.size() < 7)
        {
            if(gateElements.size() == 5) //not gate
            {
            int parent = std::stoi(gateElements[2]);            
            int output = std::stoi(gateElements[3]);
            std::string gateType = gateElements[4];

            if(output < details.numWires - details.numOutputs - details.bitlengthOutputs)
            {
                exchangeGate[output] = exchangeGate[parent];
                flipped[output] = ! flipped[output];
            }
            else
            {
                bool truthTable[2][2]  = { {1,1},{0,0} };

                if (flipped[parent] == 1)                            
                    swapLeftParent(truthTable);
                gates[gateCounter] = {exchangeGate[parent], exchangeGate[parent], output, truthTable};
                gateCounter++;
            }

            }

            else if(gateElements.size() == 6)
            {
            int leftParent = std::stoi(gateElements[2]);            
            int rightParent = std::stoi(gateElements[3]);
            int output = std::stoi(gateElements[4]);
            std::string gateType = gateElements[5];

            bool truthTable[2][2];
            if(gateType == "XOR")
                bool truthTable[2][2]  = { {0,1},{1,0} };
            else if(gateType == "AND")
                bool truthTable[2][2]  = { {0,0},{0,1} };
            else if(gateType == "OR")
                bool truthTable[2][2]  = { {0,1},{1,1} };

            if(flipped[leftParent] == 1)
                swapLeftParent(truthTable);
            
            if(flipped[rightParent] == 1)
                swapRightParent(truthTable);

            gates[gateCounter] = {exchangeGate[leftParent], exchangeGate[rightParent], output, truthTable};
            gateCounter++;
            }

        }   
    }

    int deleteCounter = 0;
    int adjustedWire[details.numWires];
    for(int i = 0; i <details.numWires;i++)
    {
        if(exchangeGate[i] != i)
        {
            deleteCounter++;
            details.numWires--;
            details.numGates--;
            
        }
        adjustedWire[i] = i - deleteCounter;
    }
    for(auto g: gates)
    {
        g.leftParentID = adjustedWire[g.leftParentID];
        g.rightParentID = adjustedWire[g.rightParentID];
        g.outputID = adjustedWire[g.outputID];
    }

    TransformedGate* transformedGates = new TransformedGate[details.numGates];    
    for(int i =0;i< details.numGates; i++)
    {
        transformedGates[i] = gates[i]; //does that really copy?
    }

    TransformedCircuit* circuit = new TransformedCircuit;    
    *circuit = {details, transformedGates};

    return circuit;
}