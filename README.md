# Completely Reusable Garbled Circuit (CRGC) Generator C++ Version 


Transform C++ functions and boolean circuits stored in [Bristol Fashion](https://homes.esat.kuleuven.be/~nsmart/MPC/) into CRGCs. Our program can transform high-level source code to a CRGC using:
1. Any C++ file imported into this library
2. Any boolean circuit compiled from C++ code using [emp-toolkit](https://github.com/emp-toolkit)
3. Any boolean circuit compiled from C code using [CBMC-GC-2](https://gitlab.com/securityengineering/CBMC-GC-2/-/tree/master)


Reusable garbled circuits constructed by this program do not rely on oblivious transfer or double encryption of each gate's truth table. Instead, we use different obfuscation techniques to obfuscate the circuit and the generator's secret input. 

Compared to Yao's Garbled Circuit protocol, the advantages are more efficient circuit evaluation and reusability of the constructed circuit for an arbitrary amount of evaluator inputs. The disadvantage of our approach is that a constructed circuit may leak some secret input bits and unobfuscated gates of the circuit to the evaluator. 

The generator can predict the leakage from a CRGC, assuming the worst case that the evaluator knows the exact construction of the boolean circuit beforehand. 
The generator executable (party A) performs the following steps:

1. It imports a C++ function and converts it to a boolean circuit or imports a boolean circuit directly.
2. It analyzes the leakage that a CRGC will have. It calculates how many gates can be perfectly obfuscated (zero-knowledge) and which input bits may be leaked when an evaluator obtains the transformed circuit. 
3. It transforms the boolean circuit into a CRGC through different obfuscation techniques. It obfuscates party A's input.
4. It evaluates the CRGC with party A's obfuscated input and the sample plain input for party B.
5. It sends the CRGC to the evaluator via network sockets or saves it as a local file. Optionally, it can compress the CRGC before sending or storing it.

The evaluator (party B) performs the following steps:

1. It receives or imports a CRGC and obfuscated input of party A.
2. It evaluates the CRGC with any plain input from party B.
3. It compresses and stores the CRGC for future use.

## Results

We provide a set of circuits as an example. The **src/circuits** folder contains basic boolean circuits stored in bristol fashion: **adder64**, **sub64**, and **sha256**. The **src/programs** folder contains C++ files implementing example programs such as Set Intersection and Linear Search.   

| Function                    | Perfectly obfuscated gates    | Secret input bits of A leaked | Evaluation Speed (µs) |
|-----------------------------|-------------------------------|-------------------------------|-----------------------------|
| 64-bit Adder                | 249 out of 376 (66.22%)       | 1 out of 64 (1.56%)           | 2                     |
| 64-bit Subtract             | 312 out of 439 (71.07%)       | 1 out of 64 (1.56%)           | 2                      |
| 64x64 -> 64 bit Multiplier  | 13611 out of 13675 (99.53%)   | 2 out of 64 (3.13%)           | 36                      |
| AES-256(k,m)                | 9367 out of 50666 (18.49%)    | 0 out of 256 (0%)             | 94                      |
| SHA256                      | 39760 out of 135073 (29.44%)  | 0 out of 512 (0%)             | 205                      |
| SHA512                      | 102704 out of 349617 (30.79%) | 0 out of 1024 (0%)            | 551                      |


| Function                    | Explanation | Evaluation speed (million gates/s)    | 
|-----------------------------|-------------------------------|-------------------------------|
| Set Intersection 40000,40000 32-bit| A has 40000 32-bit inputs. B has 40000 32-bit inputs and wants to find the intersect of both Arrays | 351 | 
| Linear Search 140000 32-bit| A has an array of 140000 indices. B wants to find a specific index. | 395 | 
| Max element in a search window of a 386x386 32-bit 2D Array|  A has a 386x386 32-bit 2D Array with values. B wants to find the maximum value in a specified search window | 329  |





## Getting Started

Commands may require sudo. Set up repository automatically:
```
./setup.sh
```
Alternatively, set up the repository in a Docker container:
```
docker build -t crgc:latest .
docker run -it crgc:latest
```
### Set up repository manually

First, install emp toolkit [emp-toolkit](https://github.com/emp-toolkit). Afterward, clone our repository.

```
git clone --recurse-submodules https://github.com/chart21/Reusable-Garbled-Circuit-Generator-CPP.git
```

In the project folder run:

```
cmake . -B ./build
cd build
make -j
```

### Optional arguments
Our library provides a generator and an evaluator executable that come with the following optional arguments. All parameters can also be modified in **src/config.h**.
#### Generator

``--circuit=< circuit name >`` Name of the cpp function or file to convert to a CRGC.

``--type=< cpp|txt >`` "cpp": Convert C++ program to a CRGC, "txt": Convert a bristol circuit stored as txt file to a CRGC.

``--format=< emp|bristol >`` Only relevant for circuits imported from a txt file. "bristol": Circuits stored in bristol fashion (https://homes.esat.kuleuven.be/~nsmart/MPC/), "emp": circuits exported from EMP SH2PC. 

``--threads=< NUMBER >`` Number of threads to use for leakage prediction and circuit construction.

``--inputa=< input >`` "r": Generate a random generator input, FILENAME: Import the generator input from a txt file (input is treated as binary value), Integer: Set argument as generator input (input is treated as integer value).

``--inputb=< input >`` Not relevant. Specify an exemplary input evaluator may query. "r": Generate a random evaluator input, FILENAME: Import the evaluator input from a txt file (input is treated as binary value), Integer: Set argument as evaluator input (input is treated as integer value).

``--network=< off|compressed|uncompressed >`` Send an "uncompressed" or "compressed" CRGC to the evaluator via network sockets. "off": Do not send CRGC to the evaluator. 

``--port=< port >`` Port to listen for the evaluator to connect. Irrelevant if --network is set to "off".

``--store=< off|compressed|bin|txt >`` "off": Generator does not store CRGC locally after construction, "compressed": Store CRGC as compressed file after construction,"bin": Store CRGC as bin file, "txt": Store CRGC as txt file 

``--compression=< NUMBER >`` Number of threads to use for compressing the CRGC.

#### Evaluator

``--ip=< ip address >`` Generator's IP address to receive the CRGC from. Irrelevant if --network is set to "off".

``--port=< port >`` Generator's Port to receive the CRGC from. Irrelevant if --network is set to "off".

``--store=< off|compressed|bin|txt >`` If --network is set as "off": Import a CRGC from a local "txt", "bin", or "compressed" file. If --network is not "off": Store CRGC as "txt", "bin", or "compressed" file.

``--network=< off|compressed|uncompressed >`` Receive an "uncompressed" or "compressed" CRGC from the evaluator via network sockets. "off": Do not receive a CRGC via the network. 

``--format=< emp|bristol >`` Only relevant for circuits that the generator compiled from a Bristol Fashion format. --format=bristol should be used with --store=txt

### Executables explained 

#### generator executable
```
./generator --circuit=myCPPFunction --type=cpp --inputa=200 --threads=40 --network=compressed --compression=30
```

Act as the **generator**. Transforms my **myCPPFunction.cpp** from the program folder with secret input **200** of party A into a CRGC using **40** Threads. Tests the integrity of the circuit's logic with a random input assumed to be from party B. Then, the generator listens to a connection from the evaluator. If it's connected successfully, the generator transfers the generated obfuscated circuit with its obfuscated input A in **compressed** format. The circuit is compressed using **30** threads.


```
./generator --circuit=adder64 --type=txt --format=bristol --network=compressed 
```

Import the boolean circuit **adder64** stored as a **txt** file in **bristol** fashion. Convert it to a CRGC, compress it, and send it over the network to the evaluator.

#### evalator executable

```
./evaluator --circuit=myCircuit --inputb=20 --network=compressed --store=bin
```
Act as the **evaluator**. Receive a **compressed** CRGC from the generator over the network. Evaluate the circuit with an evaluator input of **20**. Store the CRGC as a **.bin** file on the local hard drive.

```
./evaluator --circuit=myCircuit --inputb=30 --store=bin
```

Act as the **evaluator**. Import the CRGC and obfuscated generator input from the local binary file **./circuits/myCircuit.bin**. Evaluate the circuit with input **30** of Party B.


#### Example Outputs

Convert the **query** C++ function to a CRGC. Send it **uncompressed** over the network to the evaluator.
> ./generator --circuit=query --network=uncompressed


```
---TIMING--- 1084ms converting program to circuit
---INFO--- numGates: 9100000
---TIMING--- 39ms getting Parents of each Wire
---TIMING--- 62ms identifying potentially obfuscated fixed gates
---TIMING--- 326ms identifying potentially intermediary gates
---INFO--- potentially obfuscated fixed and intermediary gates: 3220032
---TIMING--- 9ms predict leaked inputs
---INFO--- 0 leaked inputs: 
---TIMING--- 47ms evaluate circuit
---Evaluation--- inA-1
---Evaluation--- inB-682676750
---Evaluation--- out0
---TIMING--- 122ms flip circuit
---TIMING--- 74ms identify fixed Gates
---INFO--- obfuscated gates: 1
---TIMING--- 448ms identify intermediary gates
---INFO--- obfuscated fixed and intermediary gates: 1
---TIMING--- 25ms obfuscate gates
---Success--- Evaluation of original circuit and constructed RGC are equal
---Evaluation--- inA18446744073709551615
---Evaluation--- inB3612290546
---Evaluation--- out0
connected
---TIMING--- 133ms sending
```

Receive an uncompressed CRGC over the network. Evaluate it and store it as an uncompressed **bin** file.
> ./evaluator --circuit=query --network=uncompressed --store=bin

```
connected
---TIMING--- 137ms receiving
---TIMING--- 54ms evaluate circuit
---Evaluation--- inA18446744073709551615
---Evaluation--- inB2324974971
---Evaluation--- out0
---TIMING--- 124ms exporting
```

Import the **query** CRGC stored as a **bin** file and evaluate it. 
> ./evaluator --circuit=query --network=off --store=bin

```
---TIMING--- 96ms importing
---TIMING--- 47ms evaluate circuit
---Evaluation--- inA18446744073709551615
---Evaluation--- inB20
---Evaluation--- out0
```



### Compiling a C++ function to a reusable garbled circuit using our library

You can directly convert C++ functions to a boolean circuit and a CRGC using our library. We use a slightly modified version of [emp-toolkit](https://github.com/emp-toolkit). Simply follow these steps:

1. Add your cpp file and header to the **program folder**. include your header in **circuitLinker.cpp**. Look at the existing files for inspiration. Alternatively, you can add your function directly to the **circuitLinker.cpp** file.
2. Link your file to our library by adding it to both the current folder's **CMakeLists.txt** as well as the main directory's target_link_libraries (line 61 in **CMakeLists.txt**).


Run cmake, make, and the following command to convert your program into a CRGC:  
```
./generator --circuit=<FILENAME OF YOUR CPP FILE> --type=cpp --inputa=<inputA> --threads=<number of Threads>
```



### Compiling a C++ function to a boolean circuit using emp toolkit

You can either follow the installation and setup instructions from [emp-toolkit](https://github.com/emp-toolkit) or easier clone [MPC-SoK](https://github.com/MPC-SoK/frameworks). MPC-Sok contains a Docker container that installs and sets up emp. 

Clone [MPC-SoK](https://github.com/MPC-SoK/frameworks) and run the following instructions in the **emp folder**.

```
docker build -t emp .
docker run -it --rm emp
cd sh_test
cd test
```
Add your C++ code in a separate file in the **test folder**.

```
$ echo "FILENAME OF YOUR CODE" >> CMakeLists.txt
$ make
```
From the **sh_test folder** run:
```
./bin/"FILENAME OF YOUR CODE" -m
```
This saves a boolean circuit.txt file that you can use to generate a reusable garbled circuit. Save it to the **circuits folder** of this project and run:


```
./generator --circuit=<FILENAME OF YOUR CIRCUIT FILE> --type=txt --inputa=<inputA> --format=emp --threads=<number of Threads>
```

### Compiling a C function to a reusable garbled circuit using CBMC-GC2

You can either follow the installation and setup instructions from [CBMC-GC-2](https://gitlab.com/securityengineering/CBMC-GC-2/-/tree/master) or clone [MPC-SoK](https://github.com/MPC-SoK/frameworks). In this case, both options are straightforward. MPC-Sok contains a Docker container that installs and sets up CBMC-GC-2.

Clone the [CBMC-GC-2 GitLab repo](https://gitlab.com/securityengineering/CBMC-GC-2/-/tree/master) and run the following instructions inside the **main folder**:

```
make minisat2-download
make
```

Cd into the **examples folder** and add a folder for your C program. Add your main.c file and this [Makefile](https://github.com/chart21/Reusable-Garbled-Circuit-Generator/blob/main/CBMC-GC-2_MAKEFILE/MAKEFILE) to the folder.

```
cd examples
cd YOURFOLDER
ADD main.c and MAKEFILE
make
```

This saves a bristol_circuit.txt file that you can use to generate a CRGC. Save it to the **circuits folder** of this project and run:

```
./generator --circuit=<FILENAME OF YOUR CIRCUIT FILE> --type=txt --inputa=<inputA>  --format=bristol --threads=<number of Threads>
```
