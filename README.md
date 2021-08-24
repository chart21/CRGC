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


- **Perfectly obfuscated gates**: Perfectly obfuscated gates are gates that may be obfuscated completely at random for at least one secret input without destroying the circuit's integrity. A reusable garbled circuit provides improved circuit privacy with a higher proportion of those gates. 
- **Secret input bits of A leaked**: Any non-perfectly obfuscated gate's truth table may leak information of the circuit's construction. In some cases, this information can be used to learn a secret input of party A with certainty. A reusable garbled circuit provides improved input privacy with a lower proportion of input leakage. Note that inputs are only leaked if the evaluator knows the original boolean circuit's exact (or almost exact) construction.
- **Circuit similarity**:  A reusable garbled circuit can only be constructed dependent on one secret input of party A (but supports arbitrary inputs of party B). After constructing a circuit based on a specific input, one can analyze the similarity of the constructed circuit and the original boolean circuit. A percentage of 30% means that given a random gate in the reusable garbled circuit, it has a 30% chance to have an identical truth table as the same gate in the original boolean circuit. A lower percentage of circuit similarity provides improved circuit privacy.  

## Getting Started

``python3 generator.py`` converts the adder64 circuit and a random input for party A into a reusable garbled circuit. It exports the transformed circuit (RGC) and the obfuscated input of party A to the circuits folder.

``python3 evaluator.py`` reads an adder64 RGC and obfuscated input of party A from the circuit folder. It evaluates the circuit with a random input for party B.


### Optional arguments

#### generator.py

``-f`` specify the filename for the circuit you want to execute. Circuit file has to be located in **./circuits** (default: adder64)

``-a`` set the first input of the circuit (default: Random input)

``-b`` set the second input of the circuit (default: Random input)

``-cf`` specify the format of the circuit (emp/bristol) (default: bristol)

``-ex`` eliminate NOT Gates in the circuit (flag, if set NOT gates get eliminated)






#### evaluator.py

``-f`` specify the filename for the RGC you want to execute. Circuit file has to be located in **./circuits** (default: adder64)

``-b`` set the input of party B for the evaluation of the circuit (default: Random input)


### Example 
```
python generator.py -f sub64 -a 5555 -b 222
```

Transforms the **bristol** circuit file **./circuits/sub64.txt** with secret input **5555** of party A into a reusable garbled circuit. It tests the integrity of the circuit's logic with an exemplary input of **222** that party B might query. 

```
python evaluator.py -f sub64 -b 222
```
Reads the circuit file **./circuits/sub64_rgc.txt** with secret input **./circuits/sub64_rgc_inputA.txt**. It evaluates the circuit with an input **222** from party B. 


#### Example Outputs


> python3 generator.py -f sub64 -a 5555 -b 222


```
Transforming boolean circuit sub64 to a reusable garbled circuit (RGC) ...

Analyzing leakage ...
Number of potentially obfuscated gates: 250 / 439
Proportion of potentially obfuscated gates: 0.5694760820045558 (optimal: 1.0)
Additionally obfuscated gates by breaking integrity of gates: 62 / 312
Number of potentially integrity braking gates (zero-knowledge): 312 / 439
Proportion of potentially integrity braking gates (zero-knowledge) (optimal: 1.0): 0.7107061503416856
Amount of input bits of party A that may be leaked (optimal: 0): 1 / 64
Proportion of input bits of party A that may be leaked (optimal: 0): 0.015625
The following indices of party A's input may be leaked:
0
leakage prediction finished in 0.009666400000000006 seconds

Transforming circuit into a different format ...

Evaluating boolean circuit with chosen inputs...
Finished evaluating circuit in 0.0005581000000000058 seconds
ina 5555
inb 222
circuit output 0:  5333

Transforming boolean circuit to RGC ...
Finished creating RGC in 0.006738800000000003 seconds

Evaluating RGC ...
Finished evaluating circuit in 0.0005952000000000041 seconds
ina 18264005428931638032
inb 222
circuit output 0:  5333
Success: Result of boolean circuit and RGC match
Exported RGC to ./circuits/sub64_rgc.txt
Exported party A's obfuscated input to ./circuits/sub64_rgc_inputA.txt

Analyzing constructed RGC ...
Number of obfuscated gates: 67 / 439
Number of integrity-breaking gates: 68 / 439
Integrity-breaking gates that were not obfuscated before:  1
Circuit Similarity:  0.28018223234624146
Circuit Similarity (Ex Gate 1, Ex Not) (naive approach: 0.5, optimal: 0.07142857142857142):  0.2980769230769231
Circuit Structural Similarity (naive approach: 1, optimal: 0.14285714285714285): 0.6634615384615384
```


> python3 evaluator.py -f sub64 -b 222

```
Finished evaluating circuit in 0.00042330000000000145 seconds
ina 18264005428931638032
inb 222
circuit output 0:  5333
```


### Compiling a C++ function to a reusable garbled circuit

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
python3 generator.py -f "FILENAME OF YOUR CIRCUIT FILE" -cf emp
```

### Compiling a C function to a reusable garbled circuit

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

This saves a bristol_circuit.txt file that you can use to generate a reusable garbled circuit. Save it to the **circuits folder** of this project and run:

```
python3 generator.py -f "FILENAME OF YOUR CIRCUIT FILE" -cf cbmc
```
