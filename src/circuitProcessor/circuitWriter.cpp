#include "include/circuitWriter.h"
#include "include/circuitStructs.h"
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


std::mutex mtx_send;
std::mutex mtx_id;
vector<unsigned char*> bufs_send;
vector<uint32_t> bufLens_send;
vector<uint32_t> id_send;
int last_id=-1;
vector<condition_variable> cd_send(100);

void encThread( ShrinkedGate* gates_in, size_t l, size_t offset, bool store ){

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
    int cur_id = 0;
    //bufs_send[offset*2+1] = encHlp64(in,l*2,&olg1,TYPE);
    //bufLens_send[offset*2+1] = (uint32_t)olg1;
    unsigned char* buf_tmp1 = encHlp64(in,l*2,&olg1,TYPE);
    unsigned char* buf_tmp2 = encHlp8(inTable,l%2==0?l>>1:(l>>1)+1,&olg2,TYPE);

    if(store==false) {
        mtx_id.lock();
        last_id++;
        cur_id = last_id;
        mtx_id.unlock();
    }
    else cur_id=offset;

    id_send[cur_id] = offset;
    bufs_send[cur_id*2] = buf_tmp1;
    bufLens_send[cur_id*2] = (uint32_t)olg1;
    //cd_send[offset*2+1].notify_one();

    bufs_send[cur_id*2+1] = buf_tmp2;
    bufLens_send[cur_id*2+1] = (uint32_t)olg2;
    if(store==false) { 
        id_send[cur_id] = offset;
        cd_send[cur_id].notify_one();
    }

    delete [] in;
    delete [] inTable;
}


void compressObfuscatedInput(bool *valArr, uint_fast64_t lenA, int len){
    // size_t l;
    // uint8_t *inValArr = new uint8_t[ROUND_UP(lenA,64)];
    // std::copy(valArr,valArr+lenA,inValArr);

    // bufs_send[len-1] = encHlp8(inValArr,lenA,&l,TYPE);
    // bufLens_send[len-1] = (uint32_t)l;
    // cd_send[len-1].notify_one();
    // delete [] inValArr;
}


void compressShrinkedCircuit(ShrinkedCircuit* cir, int package){
    // uint64_t inDetails[ROUND_UP(DETAILS_NUM,64)];
    // inDetails[0] = cir->details.numWires;
	// inDetails[1] = cir->details.numGates;
    // inDetails[2] = cir->details.numOutputs;
    // inDetails[3] = cir->details.bitlengthInputA;
    // inDetails[4] = cir->details.bitlengthInputB;    
    // inDetails[5] = cir->details.bitlengthOutputs;

    // size_t old;

    // bufs_send[0] = encHlp64(inDetails,DETAILS_NUM,&old,TYPE);
    // bufLens_send[0] = old;
    // cd_send[0].notify_one();

    // size_t seg = SEG(cir->details.numGates,package);
    // seg = seg%2==0?seg:seg+1;            
    // vector<thread> threads;    
    // int ll = cir->details.numGates;
    
    // for(int i=0;i<package;i++) {
    //     size_t l = ll>seg?seg:ll;
    //     threads.push_back(thread(encThread, cir->gates+i*seg, l, i));
    //     ll-=seg;
    // }

    // for (auto &th:threads) {
    //     th.join();
    // }
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

}

template <typename IO>
void Writer<IO>::exportCompressedCircuit( ShrinkedCircuit* cir, bool* valArr, int package){

    bufs_send.assign(package*2,nullptr);
    bufLens_send.assign(package*2,0);
    id_send.assign(package,0);

    //size_t len = package*2+2;

    send_data_gen( &package, sizeof(int) );

    uint64_t inDetails[ROUND_UP(DETAILS_NUM,64)];
    inDetails[0] = cir->details.numWires;
	inDetails[1] = cir->details.numGates;
    inDetails[2] = cir->details.numOutputs;
    inDetails[3] = cir->details.bitlengthInputA;
    inDetails[4] = cir->details.bitlengthInputB;    
    inDetails[5] = cir->details.bitlengthOutputs;

    size_t old;

    unsigned char *buf_detail = encHlp64(inDetails,DETAILS_NUM,&old,TYPE);
    uint32_t bufLen_detail = old;
    send_data_gen( &bufLen_detail, sizeof(bufLen_detail) );
    send_data_gen( buf_detail, sizeof(buf_detail[0])*bufLen_detail );

    //delete [] buf_detail;
    //buf_detail = nullptr;

    size_t seg = SEG(cir->details.numGates,package);
    seg = seg%2==0?seg:seg+1; 
    //size_t residue = cir->details.numGates%seg;

    vector<thread> threads;    
    int ll = cir->details.numGates;
    
    for(int i=0;i<package;i++) {
        size_t l = ll>seg?seg:ll;
        threads.push_back(thread(encThread, cir->gates+i*seg, l, i,store));
        ll-=seg;
    }
    if(store==true) {
        for (auto &th:threads) {
            th.join();
        }
        for(int i=0;i<package;i++){      
            send_data_gen( &(bufLens_send[i*2]), sizeof(bufLens_send[i*2]) );
            send_data_gen( bufs_send[i*2], sizeof(bufs_send[i*2][0])*bufLens_send[i*2] );
            delete [] bufs_send[i*2];
            bufs_send[i*2] = nullptr;

            send_data_gen( &(bufLens_send[i*2+1]), sizeof(bufLens_send[i*2+1]) );
            send_data_gen( bufs_send[i*2+1], sizeof(bufs_send[i*2][0])*bufLens_send[i*2+1] );
            delete [] bufs_send[i*2+1];
            bufs_send[i*2+1] = nullptr;
        }
    }
    else {
        thread sendThread([&]() {
            for(int i=0;i<package;i++){
                    std::unique_lock<std::mutex> lck(mtx_send);
                    while(bufs_send[i*2+1]==nullptr) cd_send[i].wait(lck);
                    send_data_gen( &(id_send[i]), sizeof(id_send[i]) );  

                send_data_gen( &(bufLens_send[i*2]), sizeof(bufLens_send[i*2]) );
                send_data_gen( bufs_send[i*2], sizeof(bufs_send[i*2][0])*bufLens_send[i*2] );
                delete [] bufs_send[i*2];
                bufs_send[i*2] = nullptr;

                send_data_gen( &(bufLens_send[i*2+1]), sizeof(bufLens_send[i*2+1]) );
                send_data_gen( bufs_send[i*2+1], sizeof(bufs_send[i*2][0])*bufLens_send[i*2+1] );
                delete [] bufs_send[i*2+1];
                bufs_send[i*2+1] = nullptr;
            }

        });
        for (auto &th:threads) {
            th.join();
        }
        sendThread.join();
    }
    

    size_t l;
    uint8_t *inValArr = new uint8_t[ROUND_UP(cir->details.bitlengthInputA,64)];
    std::copy(valArr,valArr+cir->details.bitlengthInputA,inValArr);
    unsigned char *buf_inputa = encHlp8(inValArr,cir->details.bitlengthInputA,&l,TYPE);
    uint32_t bufLen_inputa = (uint32_t)l;
    delete [] inValArr;
    inValArr = nullptr;

    send_data_gen( &bufLen_inputa, sizeof(bufLen_inputa) );
    send_data_gen( buf_inputa, sizeof(buf_inputa[0])*bufLen_inputa );
    delete [] buf_inputa;
    buf_inputa = nullptr;
  
}


template <typename IO>
void Writer<IO>::exportBin(ShrinkedCircuit* circuit, bool* valArr){

    uint64_t cir_param[6];
    cir_param[0] = circuit->details.numWires;
	cir_param[1] = circuit->details.numGates;
    cir_param[2] = circuit->details.numOutputs;
    cir_param[3] = circuit->details.bitlengthInputA;
    cir_param[4] = circuit->details.bitlengthInputB;    
    cir_param[5] = circuit->details.bitlengthOutputs;
    
    send_data_gen( cir_param, (size_t)6*sizeof(uint64_t) );


    send_data_gen(circuit->gates, (size_t)circuit->details.numGates*sizeof(ShrinkedGate) );
    send_data_gen(valArr, (size_t)circuit->details.bitlengthInputA*sizeof(bool) );
    
    return;
}
