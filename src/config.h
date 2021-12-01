#ifndef CONFIG_H__
#define CONFIG_H__

#define CIRCUITPATH "../src/circuits/" //path used to import local circuit files

#define CIRCUIT_NAME "mult3" // name of the cpp function or file to convert to a CRGC

#define NETWORK "uncompressed"
//Generator: Send an "uncompressed", or "compressed" CRGC to the evaluator via network sockets. "off": Do not send CRGC to the evaluator. 
//Evaluator: Receive an "uncompressed", or "compressed" CRGC from the evaluator via network sockets. "off": Do not recive a CRGC via network. 

#define IP "187.197.97.27" //Target IP to send CRGC to. Irrelevant if Network is set to "off".
#define PORT 8080 //Target Port to send CRGC to. Irrelevant if Network is set to "off".

#define STORE "off" 
//Generator: "off": Generator does not store CRGC locally after construction, "compress": Store CRGC as compressed file, "bin": Store CRGC as bin file, "txt": Store CRGC as txt file 
//Evaluator: If Network is set as "off": Import CRGC from "txt", "bin", or "compressed" file. If Network is not "off" store CRGC as "txt", "bin", or "compressed" file.


//Parameters only used for generator executable
#define NUM_THREADS 1 // number of threads to use for leakage prediction and circuit construction, does not affect evaluation
#define COMRPRESS_THREADS 1 // number of threads to use for compressing a CRGC
#define FILE_FORMAT "cpp" // "cpp": Convert cpp program to CRGC, "txt": convert bristol circuit stored as txt file to CRGC
#define CIRCUIT_FORMAT "bristol" // Only relevant for circuits imported from a file. "bristol": circuits  stored in bristol fashion (https://homes.esat.kuleuven.be/~nsmart/MPC/), "emp": circuits exported from EMP SH2PC 


#endif