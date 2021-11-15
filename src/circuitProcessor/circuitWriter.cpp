#include "include/circuitWriter.h"
#include "include/circuitStructs.h"
// #include "include/circuitCompressor.h"
#include "include/circuitHighSpeedNetIO.h"
#include "include/helperFunctions.h"

#include "../../TurboPFor-Integer-Compression/vp4.h"
#include "../../TurboPFor-Integer-Compression/circuitutil.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>

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

using namespace std;

// std::mutex mtx_send;
// vector<unsigned char*> bufs_send;
// vector<uint32_t> bufLens_send;
// vector<condition_variable> cd_send(202);

std::mutex mtx_send;
std::mutex mtx_write;
unsigned char* bufs_send;
uint32_t bufLens_send;
condition_variable cd_sent;
condition_variable cd_wrote;
int id_sent=-1;
int id_wrote=-1;

// vector<unique_ptr<condition_variable>> cd_send(100,unique_ptr<condition_variable>(new condition_variable));


void encThread( ShrinkedGate* gates_in, size_t l, size_t offset ){

    uint64_t* in = new uint64_t[ROUND_UP(l*2,64)];
    uint8_t* inTable = new uint8_t[ROUND_UP((l>>1)+1,8)];

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

    size_t olg1, olg2;
    unsigned char* bufs_tmp1 = encHlp64(in,l*2,&olg1,TYPE);
    unsigned char* bufs_tmp2 = encHlp8(inTable,l%2==0?l>>1:(l>>1)+1,&olg2,TYPE);

    std::unique_lock<std::mutex> lck(mtx_write);
    while(id_sent!=offset*2) cd_sent.wait(lck);
    bufs_send = bufs_tmp1;
    bufLens_send = (uint32_t)olg1;
    id_wrote++;

    cd_wrote.notify_one();

    while(id_sent!=offset*2+1) cd_sent.wait(lck);
    bufs_send = bufs_tmp2;
    bufLens_send = (uint32_t)olg2;
    id_wrote++;

    cd_wrote.notify_one();

    delete [] in;
    delete [] inTable;
}


void compressObfuscatedInput(bool *valArr, uint_fast64_t lenA, int len){
    size_t l;
    uint8_t *inValArr = new uint8_t[ROUND_UP(lenA,64)];
    std::copy(valArr,valArr+lenA,inValArr);
    // inputBuf = encHlp8(inValArr,lenA,&l,TYPE);
    // inputBufLen = (uint32_t)l;
    unsigned char* bufs_tmp = encHlp8(inValArr,lenA,&l,TYPE);

    std::unique_lock<std::mutex> lck(mtx_write);
    while(id_sent!=len-2) cd_sent.wait(lck);
    bufs_send = bufs_tmp;
    bufLens_send = (uint32_t)l;
    id_wrote++;

    cd_wrote.notify_one();
    delete [] inValArr;
}


void compressShrinkedCircuit(ShrinkedCircuit* cir, int package){
    uint64_t inDetails[ROUND_UP(DETAILS_NUM,64)];
    inDetails[0] = cir->details.numWires;
	inDetails[1] = cir->details.numGates;
    inDetails[2] = cir->details.numOutputs;
    inDetails[3] = cir->details.bitlengthInputA;
    inDetails[4] = cir->details.bitlengthInputB;    
    inDetails[5] = cir->details.bitlengthOutputs;

    size_t old;
    unsigned char* bufs_tmp = encHlp64(inDetails,DETAILS_NUM,&old,TYPE);
    bufs_send = bufs_tmp;
    bufLens_send = old;
    id_wrote = 0;
    cd_sent.notify_one();

    size_t seg = SEG(cir->details.numGates,package);
    seg = seg%2==0?seg:seg+1;            
    vector<thread> threads;    
    int ll = cir->details.numGates;
    
    for(int i=0;i<package;i++) {
        size_t l = ll>seg?seg:ll;
        threads.push_back(thread(encThread, cir->gates+i*seg, l, i));
        ll-=seg;
    }

    for (auto &th:threads) {
        th.join();
    }
}


void exportCircuitSeparateFiles(TransformedCircuit* circuit, std::string destinationPath)
{
    std::ofstream detailsFile (destinationPath + "_rgc_details.txt");

    detailsFile << circuit->details.numGates << ' ' << circuit->details.numWires << '\n';
    detailsFile << circuit->details.bitlengthInputA << ' ' << circuit->details.bitlengthInputB << '\n';
    detailsFile << circuit->details.numOutputs << ' ' << circuit->details.bitlengthOutputs;

    detailsFile.close();

    std::ofstream circuitFile (destinationPath + "_rgc.txt");

    for(auto i = 0; i < circuit->details.numGates; i++)
    {
        circuitFile << circuit->gates[i].leftParentID << ' ';
        circuitFile << circuit->gates[i].rightParentID << ' ';
        circuitFile << circuit->gates[i].outputID << ' ';
        circuitFile << circuit->gates[i].truthTable[0][0];
        circuitFile << circuit->gates[i].truthTable[0][1];
        circuitFile << circuit->gates[i].truthTable[1][0];
        circuitFile << circuit->gates[i].truthTable[1][1];
        circuitFile << '\n';          
    }
    circuitFile.close();

}


void exportObfuscatedInput(bool* valArr, const CircuitDetails &details, std::string destinationPath)
{
    if(!destinationPath.empty()){
        std::ofstream inputFile (destinationPath + "_rgc_inputA.txt");
        for (auto i = 0; i < details.bitlengthInputA; i++)
        {
            inputFile << valArr[i];
        }
        inputFile.close();
    }
    //else
    //    send_data_gen( valArr, details.bitlengthInputA*sizeof(bool) );
    
}
/*
template <typename IO>
void Gen<IO>::sendThread() {
    size_t len = bufLens_send.size();
    
    for(int i=0;i<len;i++){

        std::unique_lock<std::mutex> lck(mtx_send);
        while(bufs_send[i]==nullptr) cd_send[i].wait(lck);

        send_data_gen( &(bufLens_send[i]), sizeof(bufLens_send[0]) );
        send_data_gen( bufs_send[i], sizeof(bufs_send[i][0])*bufLens_send[i] );
        delete [] bufs_send[i];
        bufs_send[i]=nullptr;
    }

}
*/
template <typename IO>
void Gen<IO>::exportCompressedCircuit( ShrinkedCircuit* cir, bool* valArr, int package){

    // bufs_send.assign(package*2+2,nullptr);
    // bufLens_send.assign(package*2+2,0);
    size_t len = package*2+2;

    // cd_send.assign(package*2+2,unique_ptr<condition_variable>(new condition_variable));
    cout<<"package: "<<package<<endl;
    send_data_gen( &package, sizeof(int) );

    thread sendThread([&]() {
        for(int i=0;i<len;i++){
            std::unique_lock<std::mutex> lck(mtx_send);
            while(id_wrote!=i) cd_wrote.wait(lck);

            send_data_gen( &(bufLens_send), sizeof(bufLens_send) );
            send_data_gen( bufs_send, sizeof(bufs_send[0])*bufLens_send );
            id_sent = i;
            delete [] bufs_send;
            bufs_send = nullptr;
            cd_sent.notify_all();

            //cout<<i<<"th buf len: "<<bufLens_send<<endl;
        }
    });

    compressShrinkedCircuit(cir, package);

    if(valArr) compressObfuscatedInput(valArr, cir->details.bitlengthInputA, len);
    else {bufLens_send = -1; }
    sendThread.join();
  
}

template <typename IO>
void Gen<IO>::exportBin(ShrinkedCircuit* circuit, bool* valArr){

    uint64_t cir_param[6];
    cir_param[0] = circuit->details.numWires;
	cir_param[1] = circuit->details.numGates;
    cir_param[2] = circuit->details.numOutputs;
    cir_param[3] = circuit->details.bitlengthInputA;
    cir_param[4] = circuit->details.bitlengthInputB;    
    cir_param[5] = circuit->details.bitlengthOutputs;
    cout<<"tmp"<<circuit->gates[0].leftParentID<<endl;

    send_data_gen( cir_param, (size_t)6*sizeof(uint64_t) );


    send_data_gen(circuit->gates, (size_t)circuit->details.numGates*sizeof(ShrinkedGate) );
    send_data_gen(valArr, (size_t)circuit->details.bitlengthInputA*sizeof(bool) );
    
    return;
}
