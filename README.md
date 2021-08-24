# Completely Reusable Garbled Circuit (CRGC) Generator C++ Version 


Transform C++ functions and boolean circuits stored in [Bristol Fashion](https://homes.esat.kuleuven.be/~nsmart/MPC/) into CRGC. Our program can transform high-level source code to a CRGC using:
1. Any C++ file imported into this library
2. Any boolean circuit compiled from C++ code using [emp-toolkit](https://github.com/emp-toolkit)
3. Any boolean circuit compiled from C code using [CBMC-GC-2](https://gitlab.com/securityengineering/CBMC-GC-2/-/tree/master)


Reusable garbled circuits constructed by this program do not rely on oblivious transfer or double encryption of each gate's truth table. Instead, we use different obfuscation techniques to obfuscate the circuit and the generator's secret input. 

Compared to Yao's Garbled Circuit protocol, the advantages are more efficient circuit evaluation and reusability of the constructed circuit for an arbitrary amount of evaluator inputs. The disadvantage of our approach is that a constructed circuit may leak some secret input bits and unobfuscated gates of the circuit to the evaluator. 

The leakage from a reusable garbled circuit can be predicted by the generator, assuming the worst case that the evaluator knows the exact construction of the boolean circuit beforehand. 
The generator executable (party A) performs the following steps:

1. It imports a C++ function and converts it to a boolean circuit or imports a boolean circuit deirectly.
2. It analyzes the leakage that a CRGC will have. It calculates how many gates can be perfectly obfuscated (zero-knowledge) and which input bits may be leaked when an evaluator obtains the transformed circuit. 
3. It evaluates the boolean circuit with two sample inputs and stores the result.
4. It transforms the boolean circuit into a CRGC through different obfuscation techniques. It obfuscates party A's input.
5. It evaluates the CRGC with party A's obfuscated input and the sample plain input for party B.
6. It assesses whether the circuit was generated correctly (i.e., the output of step 2 and step 4 match).
7. It provides further analysis of the CRGC constructed specifically for party A's input.
8. It sends the CRGC to the evaluator via network sockets

The evaluator (party B) performs the following steps:

1. It receives or imports a CRGC and obfuscated input of party A.
2. It evaluates the CRGC with any plain input from party B.

## Results

| Function                    | Perfectly obfuscated gates    | Secret input bits of A leaked | Circuit similarity (varies) |
|-----------------------------|-------------------------------|-------------------------------|-----------------------------|
| 64-bit Adder                | 249 out of 376 (66.22%)       | 1 out of 64 (1.56%)           | 23.94%                      |
| 64-bit Subtract             | 312 out of 439 (71.07%)       | 1 out of 64 (1.56%)           | 26.65%                      |
| 64x64 -> 64 bit Multiplier  | 13611 out of 13675 (99.53%)   | 2 out of 64 (3.13%)           | 31.84%                      |
| AES-256(k,m)                | 9367 out of 50666 (18.49%)    | 0 out of 256 (0%)             | 35.51%                      |
| SHA256                      | 39760 out of 135073 (29.44%)  | 0 out of 512 (0%)             | 33.88%                      |
| SHA512                      | 102704 out of 349617 (30.79%) | 0 out of 1024 (0%)            | 33.17%                      |


| Function                    | Explanation | Perfectly obfuscated gates    | Secret input bits of A leaked | Circuit similarity (varies) |
|-----------------------------|-------------------------------|-------------------------------|-----------------------------|-----------------------------|
| Set-Intersect 200x200 8-bit| A has 200 8-bit inputs. B has 200 8-bit inputs and wants to find the intersect of both Arrays | 267900 out of 269500 (99.41%) | 0 out of 1024 (0%)            | 6.09%                       |
| Linear Search and arithmetics 4096 8-bit| A has an array of 4096 indices with 3 values each. B wants to find a specific index and run arithmetics on the three values | 2437057 out of 2437089 (> 99.99%) | 0 out of 131072 (0%)            | 22.15%                     |
| Max element in a search window of a 64x64 8-bit 2D Array|  A has a 64x64 8-bit 2D Array with values. B wants to find the maximum value in a specified search window | 1202392 out of 1202408 (> 99.99%) | 1 out of 32768 (<0.01%)            | 28.98%                       |


- **Perfectly obfuscated gates**: Perfectly obfuscated gates are gates that may be obfuscated completely at random for at least one secret input without destroying the circuit's integrity. A reusable garbled circuit provides improved obfuscation and circuit privacy with a higher proportion of those gates. 
- **Secret input bits of A leaked**: Any non-perfectly obfuscated gate's truth table may leak information of the circuit's construction. In some cases, this information can be used to learn a secret input of party A with certainty. A reusable garbled circuit provides improved input privacy with a lower proportion of input leakage. Note that inputs are only leaked if the evaluator knows the original boolean circuit's exact (or almost exact) construction.
- **Circuit similarity**:  A reusable garbled circuit can only be constructed dependent on one secret input of party A (but supports arbitrary inputs of party B). After constructing a circuit based on a specific input, one can analyze the similarity of the constructed circuit and the original boolean circuit. A percentage of 30% means that given a random gate in the reusable garbled circuit, it has a 30% chance to have an identical truth table as the same gate in the original boolean circuit. A lower percentage of circuit similarity provides improved obfuscation and circuit privacy.  

## Getting Started

First, install emp toolkit [emp-toolkit](https://github.com/emp-toolkit). Afterward, clone our repo, cd into it and run the following commands:


```
cmake . -B ./build
cd build
make -j
```




``./build/generator adder64 txt`` converts the adder64 circuit and a random input for party A into a CRGC. It exports the CRGC and the obfuscated input of party A to the circuits folder.

``./build/evaluator adder64 txt`` reads an adder64 RGC and obfuscated input of party A from the circuit folder. It evaluates the circuit with a random input for party B.


### Optional arguments

#### generator executable

``1st param`` specify the filename for the circuit or CPP function you want to execute. Circuit file has to be located in **./src/circuits**. C++ file has **./src/programs**. (default: adder64)

``2nd param`` specify the file format: cpp/txt/bin (default: txt) 

``3rd param`` set the first input of the circuit. Use **r** for random input. (default: Random input)

``4th param`` set the second input of the circuit. Use **r** for random input. (default: Random input)

``5th param`` specify the format of the circuit (emp/bristol). Use emp for circuit files generated gnerated by emp. (default: bristol)

``6th param`` pset the number of threads that should be used. Many of our functions benefit from multithreading. (default: 1)







#### evalator executable




### Example 
```
./build/generator myCPPFunction cpp 200 300 bristol 40
```

Transforms my **myCPPFunction.cpp** from the program folder with secret input **200** of party A into a CRGC using **40** Threads. It tests the integrity of the circuit's logic with an exemplary input of **300** that party B might query. The bristol keyword is irrelevant for imported C++ functions.

```
./build/generator myCircuit txt r r emp 1
```
Transorms the circuit file **./circuits/myCircuit.txt** generated by **emp** with a random secret input using **1** Thread. It evaluates the circuit with a random input for party B. 


#### Example Outputs


> ./build/generator myCPPFunction cpp 200 300 bristol 40


```
---TIMING--- 7073ms converting program to circuit
---INFO--- numGates: 79918167
---TIMING--- 224ms getting Parents of each Wire
---TIMING--- 502ms identifying potentially obfuscated fixed gates
---TIMING--- 547ms identifying potentially intermediary gates
---INFO--- potentially obfuscated and integrity-breaking gates: 6888560
---TIMING--- 64ms predict leaked inputs
---INFO--- 0 leaked inputs: 
---TIMING--- 405ms evaluate circuit
---Evaluation--- inA200
---Evaluation--- inB300
---Evaluation--- out0
---TIMING--- 942ms flip circuit
---TIMING--- 503ms identify fixed Gates
---INFO--- obfuscated gates: 50990467
---TIMING--- 190ms identify intermediary gates
---INFO--- obfuscated and integrity-breaking gates: 79838167
---TIMING--- 437ms obfuscate gates
---Success--- Evaluation of original circuit and constructed RGC are equal
---Evaluation--- inA18446744073709551615
---Evaluation--- inB300
---Evaluation--- out0
---TIMING--- 7038ms converting program to circuit
---INFO--- numGates: 79918167
Ratio of identical gates compared to original circuit: 0.00100102
---TIMING--- 175ms compare circuit similarity
```


### Compiling a C++ function to a reusable garbled circuit using our library

You can direclty convert C++ functions to a boolean circuit and a CRGC using our library. We use a slightly modified version of [emp-toolkit](https://github.com/emp-toolkit). Simply follow these steps:

1. Add your cpp file and header to the **program folder**. Look at the existing files for inspiration.
2. Link your file to our library by adding it to the target_link_libraries of line 61 in **CMakeLists.txt**.


Run cmake, make and the following command to convert your program into a CRGC:  
```
./build/generator *FILENAME OF YOUR CPP FILE* cpp *inputA* *inputB* bristo; *number of Threads*
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
./build/generator *FILENAME OF YOUR CIRCUIT FILE* txt *inputA* *inputB* emp *number of Threads*
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
./build/generator *FILENAME OF YOUR CIRCUIT FILE* txt *inputA* *inputB* bristol *number of Threads*
```

