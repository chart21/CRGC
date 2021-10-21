#ifndef CIRCUIT_COMPRESSOR_H__
#define CIRCUIT_COMPRESSOR_H__

#include <iostream>
#include <vector>
#include "circuitStructs.h"

#define SEG(n,th) (n+th-n%th)/th
#define ROUND_UP(_n_, _a_) (((_n_) + ((_a_)-1)) & ~((_a_)-1))
#define P4NENC_BOUND(n, size) ((n + 127) / 128 + (n + 32) * (size))
#define P4NDEC_BOUND(n, size) (ROUND_UP(n, 32) * (size))

#define TYPE 1

#define DETAILS_NUM 6
#define UNT 8
#define LIMIT 500000
using namespace std;


/* compress a shrinked Circuit using one of the integer and bit compression libraries */
void compressShrinkedCircuit(ShrinkedCircuit* cir, vector<unsigned char*> &bufs, vector<uint32_t> &bufLens, int thr_enc);
void compressObfuscatedInput(bool *valArr, uint_fast64_t lenA, unsigned char* &inputBuf, uint32_t &inputBufLen);

void decompressShrinkedCircuit(vector<unsigned char*> &bufGates, vector<unsigned char*> &bufTables, vector<size_t> &dataLens, ShrinkedCircuit* scir,int thr_enc, int thr_dec, size_t seg);
void decompressObfuscatedInput(unsigned char* bufInput, size_t dataInputLen, bool* &valArr);
/* export compressed circuit to HDD */
//void exportCompressedCircuit( ShrinkedCircuit* cir, int thr_enc=1);


/* import compressed Circuit */
//ShrinkedCircuit* importCompressedCircuit( int thr_dec);


/* check if the imported, reconstructed compressed circuit is equal to the orginal one */
bool areCircuitsEqual(TransformedCircuit* importedCircuit, TransformedCircuit* originalCircuit);
bool areShrinkedCircuitsEqual(ShrinkedCircuit* importedCircuit, ShrinkedCircuit* originalCircuit);
#endif