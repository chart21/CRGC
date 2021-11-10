# Completely Reusable Garbled Circuit (CRGC) Generator C++ Version 


Transform C++ functions and boolean circuits stored in [Bristol Fashion](https://homes.esat.kuleuven.be/~nsmart/MPC/) into CRGC. Our program can transform high-level source code to a CRGC using:
1. Any C++ file imported into this library
2. Any boolean circuit compiled from C++ code using [emp-toolkit](https://github.com/emp-toolkit)
3. Any boolean circuit compiled from C code using [CBMC-GC-2](https://gitlab.com/securityengineering/CBMC-GC-2/-/tree/master)


Reusable garbled circuits constructed by this program do not rely on oblivious transfer or double encryption of each gate's truth table. Instead, we use different obfuscation techniques to obfuscate the circuit and the generator's secret input. 

Compared to Yao's Garbled Circuit protocol, the advantages are more efficient circuit evaluation and reusability of the constructed circuit for an arbitrary amount of evaluator inputs. The disadvantage of our approach is that a constructed circuit may leak some secret input bits and unobfuscated gates of the circuit to the evaluator. 

The generator can predict the leakage from a CRGC, assuming the worst case that the evaluator knows the exact construction of the boolean circuit beforehand. 
The generator executable (party A) performs the following steps:

1. It imports a C++ function and converts it to a boolean circuit or imports a boolean circuit directly.
2. It analyzes the leakage that a CRGC will have. It calculates how many gates can be perfectly obfuscated (zero-knowledge) and which input bits may be leaked when an evaluator obtains the transformed circuit. 
3. It evaluates the boolean circuit with two sample inputs and stores the result.
4. It transforms the boolean circuit into a CRGC through different obfuscation techniques. It obfuscates party A's input.
5. It evaluates the CRGC with party A's obfuscated input and the sample plain input for party B.
6. It assesses whether the circuit was generated correctly (i.e., the output of step 2 and step 4 match).
7. It provides further analysis of the CRGC constructed specifically for party A's input.
8. It sends the CRGC to the evaluator via network sockets, or save it as local binary file.

The evaluator (party B) performs the following steps:

1. It receives or imports a CRGC and obfuscated input of party A.
2. It evaluates the CRGC with any plain input from party B.

## Results

| Function                    | Evaluate speed (gates/us)   |
|-----------------------------|-------------------------------|
| 64-bit Adder                | 46    |
| 64-bit Subtract             | 54        |
| 64x64 -> 64 bit Multiplier  | 187    |
| AES-256(k,m)                | 273     |
| SHA256                      | 252   |
| SHA512                      | 241  |


| Function                    | Explanation | Evaluate speed (million gates/s)    | 
|-----------------------------|-------------------------------|-------------------------------|
| Set-Intersect 40000,40000 32-bit| A has 40000 32-bit inputs. B has 40000 32-bit inputs and wants to find the intersect of both Arrays | 191.614 | 
| Linear Search and arithmetics 140000 32-bit| A has an array of 140000 indices. B wants to find a specific index. | 156.897 | 
| Max element in a search window of a 386x386 32-bit 2D Array|  A has a 386x386 32-bit 2D Array with values. B wants to find the maximum value in a specified search window | 185.076  |


- **Evaluate speed**: Benchmark two endpoints are AWS instance with type m5zn.metal, 100Gbits bandwidth. Use 100 compression threads through network transfer.


## Getting Started

First, install emp toolkit [emp-toolkit](https://github.com/emp-toolkit). Afterward, clone our repo, you can either run 

```
git clone --recurse-submodules https://github.com/chart21/Reusable-Garbled-Circuit-Generator-CPP.git
```

or initialize and fetch the [TurboPFor](https://github.com/firebolt007/TurboPFor-Integer-Compression.git) submodule, which are adapted from the repository [TurboPFor](https://github.com/powturbo/TurboPFor-Integer-Compression.git) manually after git clone

```
git submodule update --init --recursive
```

then run the following codes

```
cmake . -B ./build
cd build
make -j
```

### Optional arguments
``--party=< 0|1|2 >`` 1 for generator, 2 for evaluator 

``--circuit=< circuit name >``

``--type=< cpp|txt >`` only generator needs this option, to specify the original circuit format. cpp as default.

``--inputa=< input >`` generator must specify input a, or input a will be random. This option make no sense to evaluator

``--inputb=< input >`` evaluator can specify input b, or it will be random.

``--format=< emp|bristol >`` only make sense if circuit type is txt, apparently only generator needs this option.

``--thread=< thread >`` only generator needs this option, to specify how many threads are used to construct the circuit.

``--disk`` if specify, generator writes the generated circuit to hard disk instead of transfering through network, evaluator reads from the disk. Data format can be specified, bin as default.

``--bin`` if **--disk** is specified, ex/import circuit as binary, if **--disk** is not specified, transfer circuit through network in bin, in this case, generator and evaluator should both set as **--bin**. Be incompatible with **--txt** and **--compress**.

``--compress`` if **--disk** is specified, ex/import circuit as compressed file, if **--disk** is not specified, transfer circuit through network in compressed data, in this case, generator and evaluator should both set as **--compress**. Be incompatible with **--txt** and **--bin**.

``--txt`` if **--disk** is specified, ex/import circuit as rgc txt file, if **--disk** is not specified, this option makes no sense, circuit will be transferred through network in compressed format. Be incompatible with **--bin** and **--compress**.

``--compression=< compress threads >`` if **--comress** is set, thread number can be sepcified.

### Example 

#### generator executable
```
./build/generator --party=1 --circuit=myCPPFunction --type=cpp --inputa=200 --thread=40 --compress --compression=100
```

Sets this end as **generator**. Transforms my **myCPPFunction.cpp** from the program folder with secret input **200** of party A into a CRGC using **40** Threads. Tests the integrity of the circuit's logic with a random input supposed to be from party B. Then the generator listens to connection from an evaluator. If it's connected successfully, the generator transfers the generated obfuscated circuit with obfuscated input A in **compress** format. The data is compressed in **100** threads.

```
./build/generator --party=1 --circuit=myCircuit --disk --txt
```

Sets this end as **generator**. Transforms the circuit file **./circuits/myCircuit.txt** generated by **emp** with a random secret inputa using 1 Thread. Tests the integrity of the circuit's logic with a random input supposed to be from party B. Then export the generated obfuscated circuit and input A as **binary** file in **disk**.


#### evalator executable

```
./build/generator --party=2 --circuit=myCircuit --inputb=20 --format=bristol --disk --bin
```

Sets this end as **evaluator**. Import the binary file **./circuits/myCircuit.bin** in hard disk to the circuit struct. Evaluate the circuit with input **20** of Party B.


#### Example Outputs


> ./build/generator --party=1 --circuit=query --type=cpp --bin


```
---TIMING--- 1046064us converting program to circuit
read: 0
---INFO--- numGates: 9100000
---TIMING--- 41126us getting Parents of each Wire
---TIMING--- 63259us identifying potentially obfuscated fixed gates
---TIMING--- 320300us identifying potentially intermediary gates
---INFO--- potentially obfuscated fixed and intermediary gates: 3220032
---TIMING--- 21336us predict leaked inputs
---INFO--- 0 leaked inputs: 
---TIMING--- 49661us evaluate circuit
---Evaluation--- inA-1
---Evaluation--- inB1181228270
---Evaluation--- out0
---TIMING--- 127636us flip circuit
---TIMING--- 70419us identify fixed Gates
---INFO--- obfuscated gates: 1
---TIMING--- 451681us identify intermediary gates
---INFO--- obfuscated fixed and intermediary gates: 1
---TIMING--- 26443us obfuscate gates
---Success--- Evaluation of original circuit and constructed RGC are equal
---Evaluation--- inA18446744073709551615
---Evaluation--- inB1181228270
---Evaluation--- out0
connected
tmp0
---TIMING--- 129515us send
```
> ./build/generator --party=2 --circuit=query --bin

```
connected
---TIMING--- 133045us receive
---TIMING--- 53469us evaluate circuit
---Evaluation--- inA18446744073709551615
---Evaluation--- inB3287501720
---Evaluation--- out0
```

### Compiling a C++ function to a reusable garbled circuit using our library

You can directly convert C++ functions to a boolean circuit and a CRGC using our library. We use a slightly modified version of [emp-toolkit](https://github.com/emp-toolkit). Simply follow these steps:

1. Add your cpp file and header to the **program folder**. include your header in **circuitLinker.cpp**. Look at the existing files for inspiration. Alternatively you can add your function directly to the **circuitLinker.cpp** file.
2. Link your file to our library by adding it to both the the current folder's **CMakeLists.txt** aswell as the main directory's target_link_libraries (line 61 in **CMakeLists.txt**).


Run cmake, make, and the following command to convert your program into a CRGC:  
```
./build/generator --party=1 --circuit=<FILENAME OF YOUR CPP FILE> --type=cpp --inputa=<inputA> --thread=<number of Threads>
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
./build/generator --party=1 --circuit=<FILENAME OF YOUR CIRCUIT FILE> --type=txt --inputa=<inputA> --format=emp --thread=<number of Threads>
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
./build/generator --party=1 --circuit=<FILENAME OF YOUR CIRCUIT FILE> --type=txt --inputa=<inputA>  --format=bristol --thread=<number of Threads>
```
