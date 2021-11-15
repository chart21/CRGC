#include "include/circuitCompressor.h"

#include <iostream>
#include <stdio.h>
#include "../../TurboPFor-Integer-Compression/vp4.h"
#include "../../TurboPFor-Integer-Compression/circuitutil.h"
#include "include/helperFunctions.h"
#include "include/circuitStructs.h"
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

#define TBL_SHR
//#define BP
#define SEG(n,th) (n+th-n%th)/th
#define ROUND_UP(_n_, _a_) (((_n_) + ((_a_)-1)) & ~((_a_)-1))
#define P4NENC_BOUND(n, size) ((n + 127) / 128 + (n + 32) * (size))
#define P4NDEC_BOUND(n, size) (ROUND_UP(n, 32) * (size))

#define TYPE 1


#define DETAILS_NUM 6
#define UNT 8
#define LIMIT 500000

void encThread( ShrinkedGate* gates_in, vector<unsigned char*>::iterator bufs_it, vector<uint32_t>::iterator bufLens_it, 
                uint64_t* in, uint8_t* inTable, uint64_t gate_num, size_t l, size_t offset){
    chrono::time_point<std::chrono::system_clock> a, b, c, d;
    //a = chrono::system_clock::now();
    in = new uint64_t[ROUND_UP(l*2,64)];
    inTable = new uint8_t[ROUND_UP((l>>1)+1,8)];

    uint8_t gate=0;

    for(uint64_t i=0;i<l;i++){

        in[i] = gates_in[i].leftParentID;
        in[i+l] = gates_in[i].rightParentOffset;

        
        if(i%2==0){
            gate=0;

            gate ^= gates_in[i].truthTable[0][0];
            gate ^= gates_in[i].truthTable[0][1]<<1;
            gate ^= gates_in[i].truthTable[1][0]<<2;
            gate ^= gates_in[i].truthTable[1][1]<<3;
        }
        else{
            gate ^= gates_in[i].truthTable[0][0]<<4;
            gate ^= gates_in[i].truthTable[0][1]<<5;
            gate ^= gates_in[i].truthTable[1][0]<<6;
            gate ^= gates_in[i].truthTable[1][1]<<7;

            inTable[i>>1]=gate;
        }

    }
    if(l%2!=0) inTable[l>>1]=gate;
    //b = chrono::system_clock::now();

    size_t olg;
    *bufs_it = encHlp64(in,l*2,&olg,TYPE);
    *bufLens_it = (uint32_t)olg;

    *(bufs_it+1) = encHlp8(inTable,l%2==0?l>>1:(l>>1)+1,&olg,TYPE);
    *(bufLens_it+1) = (uint32_t)olg;
    delete [] in;
    delete [] inTable;
    //c = chrono::system_clock::now();
}

void compressObfuscatedInput(bool *valArr, uint_fast64_t lenA, unsigned char* &inputBuf, uint32_t &inputBufLen){
    size_t l;
    uint8_t *inValArr = new uint8_t[ROUND_UP(lenA,64)];
    std::copy(valArr,valArr+lenA,inValArr);
    inputBuf = encHlp8(inValArr,lenA,&l,TYPE);
    inputBufLen = (uint32_t)l;
    delete [] inValArr;
}

void compressShrinkedCircuit(ShrinkedCircuit* cir, vector<unsigned char*> &bufs, vector<uint32_t> &bufLens, int thr_enc){
    //uint_fast64_t len = cir->details.numGates;
    chrono::time_point<std::chrono::system_clock> a, b, c, d, e, f;
    f = chrono::system_clock::now();

    uint64_t inDetails[ROUND_UP(DETAILS_NUM,64)];
    inDetails[0] = cir->details.numWires;
	inDetails[1] = cir->details.numGates;
    inDetails[2] = cir->details.numOutputs;
    inDetails[3] = cir->details.bitlengthInputA;
    inDetails[4] = cir->details.bitlengthInputB;    
    inDetails[5] = cir->details.bitlengthOutputs;
    size_t old;

    bufs[0] = encHlp64(inDetails,DETAILS_NUM,&old,TYPE);
    bufLens[0] = (uint32_t)old;
    
    size_t seg = SEG(cir->details.numGates,thr_enc);
    //size_t seg = ROUND_UP(cir->details.numGates,thr_enc)/thr_enc;
    seg = seg%2==0?seg:seg+1;            

    vector<thread> threads;    
    int ll = cir->details.numGates;
    
    // uint64_t* in = new uint64_t[ROUND_UP(ll*2,64)];
    // uint8_t* inTable = new uint8_t[ROUND_UP((ll>>1)+1,8)];
    
    
    for(int t=0;t<thr_enc;t++) {
        size_t l = ll>seg?seg:ll;
        size_t off = seg*t;
        vector<unsigned char*>::iterator bufs_it=bufs.begin()+t*2+1;
        vector<uint32_t>::iterator bufLens_it=bufLens.begin()+t*2+1;
        uint64_t* in = nullptr;
        uint8_t* inTable = nullptr;
        // threads.push_back(thread(encThread, cir->gates+off, bufs_it, bufLens_it, in+off*2, inTable+(off>>1), cir->details.numGates, l, t));
        threads.push_back(thread(encThread, cir->gates+off, bufs_it, bufLens_it, in, inTable, cir->details.numGates, l, t));
        ll-=seg;

    }


    for (auto &th:threads) {
        th.join();
    }

    // delete [] in;
    // delete [] inTable;
    
    size_t oll;
    bufs.back()=(encHlp32(bufLens.data(),bufLens.size()-1,&oll,TYPE));
    bufLens.back()=(oll);
    
    
    // cout << "pre info time: " << chrono::duration_cast<chrono::microseconds>(a-f).count() << ", malloc time: " << chrono::duration_cast<chrono::microseconds>(b-a).count() << ", create thread time: " << chrono::duration_cast<chrono::microseconds>(c-b).count()
    //      << ", thread join time: " << chrono::duration_cast<chrono::microseconds>(e-d).count() << endl;
    //cout<<"finished enc"<<endl;
}


void decThread(ShrinkedGate* gates, vector<unsigned char*>::iterator bufGate_it, vector<unsigned char*>::iterator bufTable_it, 
                vector<size_t>::iterator dataLens_it, int offset, int ss, int seg){
    size_t olg=0;
    size_t g_pr=0;
    size_t t_pr=0;
    for(int i=0;i<ss;i++){

        size_t l = *dataLens_it;
        size_t tbll = l%2==0 ? l>>1 : (l>>1)+1;

        uint64_t* outGates=decHlp64( *(bufGate_it+i), l*2, &olg,TYPE);
        delete [] *(bufGate_it+i);

        uint8_t* outTable=decHlp8( *(bufTable_it+i), tbll, &olg,TYPE);
        delete [] *(bufTable_it+i);

        
        int off = i*seg;
        for(int j=0;j<l;j++){
            gates[j+off].leftParentID = outGates[j];
            gates[j+off].rightParentOffset = outGates[j+l];

            if(j%2==0){
                gates[j+off].truthTable[0][0] = outTable[j>>1]&0b01;
                gates[j+off].truthTable[0][1] = outTable[j>>1]&0b10;
                gates[j+off].truthTable[1][0] = outTable[j>>1]&0b100;
                gates[j+off].truthTable[1][1] = outTable[j>>1]&0b1000;

            }
            else{
                gates[j+off].truthTable[0][0] = outTable[j>>1]&0b10000;
                gates[j+off].truthTable[0][1] = outTable[j>>1]&0b100000;
                gates[j+off].truthTable[1][0] = outTable[j>>1]&0b1000000;
                gates[j+off].truthTable[1][1] = outTable[j>>1]&0b10000000;

            }
        }

        delete [] outGates;
        delete [] outTable;
    }

}

void decompressObfuscatedInput(unsigned char* bufInput, size_t dataInputLen, bool* &valArr){
    size_t l;
    uint8_t* outInput = decHlp8( bufInput, dataInputLen, &l,TYPE);
    std::copy(outInput,outInput+dataInputLen,valArr);
    delete [] bufInput;
    delete [] outInput;
}

void decompressShrinkedCircuit(vector<unsigned char*> &bufGates, vector<unsigned char*> &bufTables, 
                                vector<size_t> &dataLens, ShrinkedCircuit* scir, int thr_enc, int thr_dec, size_t seg) {
    vector<thread> threads;
    int seg_vec = ROUND_UP(thr_enc, thr_dec)/thr_dec;
    int sss = thr_enc;
    size_t gate_pr=0;
    size_t table_pr=0;

    for(int i=0;i<thr_enc;i+=seg_vec){
        int ss=sss>seg_vec?seg_vec:sss;
        vector<size_t>::iterator dataLens_it=dataLens.begin()+i;

        threads.push_back(thread(decThread,scir->gates+i*seg,bufGates.begin()+i,bufTables.begin()+i,dataLens_it,i,ss,seg));
        //cout<<"start dec thread: "<<threads.back().get_id()<<endl;

        sss-=seg_vec;
    }
    
    for(auto &th:threads) {
        th.join();
    }
}


bool areCircuitsEqual(TransformedCircuit* importedCircuit, TransformedCircuit* originalCircuit){
    if(importedCircuit->details.numWires != originalCircuit->details.numWires) {cout<<"!wires"<<endl; return false;}
	if(importedCircuit->details.numGates != originalCircuit->details.numGates) {cout<<"!gates"<<endl; return false;}
    if(importedCircuit->details.numOutputs != originalCircuit->details.numOutputs) {cout<<"!outputs"<<endl; return false;}
    if(importedCircuit->details.bitlengthInputA != originalCircuit->details.bitlengthInputA) {cout<<"!lenA"<<endl; return false;}
    if(importedCircuit->details.bitlengthInputB != originalCircuit->details.bitlengthInputB) {cout<<"!lenB"<<endl; return false;}
    if(importedCircuit->details.bitlengthOutputs != originalCircuit->details.bitlengthOutputs) {cout<<"!lenOut"<<endl; return false;}
    for(int i=0;i<importedCircuit->details.numGates;i++){
        if(importedCircuit->gates[i].leftParentID != originalCircuit->gates[i].leftParentID) {cout<<i<<"!left: "<<importedCircuit->gates[i].leftParentID<<", "<<originalCircuit->gates[i].leftParentID<<endl; return false;}
        if(importedCircuit->gates[i].rightParentID != originalCircuit->gates[i].rightParentID) {cout<<i<<"!right"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[0][0] != originalCircuit->gates[i].truthTable[0][0]) {cout<<i<<"!gate00"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[0][1] != originalCircuit->gates[i].truthTable[0][1]) {cout<<i<<"!gate01"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[1][0] != originalCircuit->gates[i].truthTable[1][0]) {cout<<i<<"!gate10"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[1][1] != originalCircuit->gates[i].truthTable[1][1]) {cout<<i<<"!gate11"<<endl; return false;}
    }
    return true;
}

bool areShrinkedCircuitsEqual(ShrinkedCircuit* importedCircuit, ShrinkedCircuit* originalCircuit){
    if(importedCircuit->details.numWires != originalCircuit->details.numWires) {cout<<"!wires"<<endl; return false;}
	if(importedCircuit->details.numGates != originalCircuit->details.numGates) {cout<<"!gates"<<endl; return false;}
    if(importedCircuit->details.numOutputs != originalCircuit->details.numOutputs) {cout<<"!outputs"<<endl; return false;}
    if(importedCircuit->details.bitlengthInputA != originalCircuit->details.bitlengthInputA) {cout<<"!lenA"<<endl; return false;}
    if(importedCircuit->details.bitlengthInputB != originalCircuit->details.bitlengthInputB) {cout<<"!lenB"<<endl; return false;}
    if(importedCircuit->details.bitlengthOutputs != originalCircuit->details.bitlengthOutputs) {cout<<"!lenOut"<<endl; return false;}
    for(int i=0;i<importedCircuit->details.numGates;i++){
        if(importedCircuit->gates[i].leftParentID != originalCircuit->gates[i].leftParentID) {cout<<i<<"!left: "<<importedCircuit->gates[i].leftParentID<<", "<<originalCircuit->gates[i].leftParentID<<endl; return false;}
        if(importedCircuit->gates[i].rightParentOffset != originalCircuit->gates[i].rightParentOffset) {cout<<i<<"!right"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[0][0] != originalCircuit->gates[i].truthTable[0][0]) {cout<<i<<"!gate00"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[0][1] != originalCircuit->gates[i].truthTable[0][1]) {cout<<i<<"!gate01"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[1][0] != originalCircuit->gates[i].truthTable[1][0]) {cout<<i<<"!gate10"<<endl; return false;}
        if(importedCircuit->gates[i].truthTable[1][1] != originalCircuit->gates[i].truthTable[1][1]) {cout<<i<<"!gate11"<<endl; return false;}
    }
    return true;
}