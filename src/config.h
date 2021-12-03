#ifndef CONFIG_H__
#define CONFIG_H__

#define CIRCUITPATH "../src/circuits/" //Path used to import local circuit files from

#define CIRCUIT_NAME "mult3" // Name of the cpp function or file to convert to a CRGC

#define NETWORK "uncompressed"
//Generator: Send an "uncompressed" or "compressed" CRGC to the evaluator via network sockets. "off": Do not send CRGC to the evaluator. 
//Evaluator: Receive an "uncompressed" or "compressed" CRGC from the evaluator via network sockets. "off": Do not receive a CRGC via the network. 
 

#define IP "127.0.0.1" //Evaluator: Generator's IP address to receive the CRGC from. Irrelevant if --network is set to "off".
#define PORT 8080 //Generator's port that listens to connections. Irrelevant if Network is set to "off".

#define STORE "off" 
//Generator:  "off": Generator does not store CRGC locally after construction, "compressed": Store CRGC as compressed file after construction,"bin": Store CRGC as bin file, "txt": Store CRGC as txt file 
//Evaluator: If --network is set as "off": Import a CRGC from a local "txt", "bin", or "compressed" file. If --network is not "off": Store CRGC as "txt", "bin", or "compressed" file.

#define COMRPRESS_THREADS 1 // Number of threads to use for compressing the CRGC.

//Parameters only used for generator executable
#define NUM_THREADS 1 //  Number of threads to use for leakage prediction and circuit construction.
#define FILE_FORMAT "cpp" // "cpp": Convert cpp program to CRGC, "txt": convert bristol circuit stored as txt file to CRGC.
#define CIRCUIT_FORMAT "bristol" // Only relevant for circuits imported from a file. "bristol": circuits  stored in bristol fashion (https://homes.esat.kuleuven.be/~nsmart/MPC/), "emp": circuits exported from EMP SH2PC 


#endif