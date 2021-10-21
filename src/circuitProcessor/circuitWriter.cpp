#include "include/circuitWriter.h"
#include "include/circuitStructs.h"
#include "include/circuitCompressor.h"
//include "include/circuitTransfer.h"
//#include "emp-tool/emp-tool.h"
#include "include/circuitHighSpeedNetIO.h"
#include <iostream>
#include <fstream>
#include <vector>


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

template <typename IO>
void Gen<IO>::exportObfuscatedInput(bool* valArr, const CircuitDetails &details, std::string destinationPath)
{
    if(!destinationPath.empty()){
        std::ofstream inputFile (destinationPath + "_rgc_inputA.txt");
        for (auto i = 0; i < details.bitlengthInputA; i++)
        {
            inputFile << valArr[i];
        }
        inputFile.close();
    }
    else
        send_data_gen( valArr, details.bitlengthInputA*sizeof(bool) );
    
}

template <typename IO>
void Gen<IO>::exportCompressedCircuit( ShrinkedCircuit* cir, bool* valArr, int thr_enc){
    //chrono::time_point<std::chrono::system_clock> start, end_enc, end_write;

    vector<unsigned char*> bufs(thr_enc*2+2);
    vector<uint32_t> bufLens(thr_enc*2+2);
    unsigned char* inputBuf;
    uint32_t inputBufLen;
    //start = std::chrono::system_clock::now();

    compressShrinkedCircuit(cir, bufs, bufLens, thr_enc);
    if(valArr) compressObfuscatedInput(valArr, cir->details.bitlengthInputA, inputBuf, inputBufLen);
    //end_enc = std::chrono::system_clock::now();

    //if(chrono::duration_cast<chrono::microseconds>(end_enc - start).count()>1000000)
    //t_enc_sum += chrono::duration_cast<chrono::microseconds>(end_enc - start).count();
    //t_enc_sum += chrono::duration_cast<chrono::milliseconds>(end_enc - start).count();
    //cout<<"finished enc, elapsed seconds: "<<chrono::duration_cast<chrono::milliseconds>(end_enc - start).count()<<endl;


    size_t len=bufLens.size()-1;
    //fwrite(&len,(size_t),1,enc);
    // io->send_data( &len, sizeof(size_t) );
    send_data_gen( &len, sizeof(size_t) );

    //fwrite(&(bufLens.back()),sizeof(bufLens.back()),1,enc);
    // io->send_data( &(bufLens.back()), sizeof(bufLens.back()) );
    send_data_gen( &(bufLens.back()), sizeof(bufLens.back()) );

    //cout<<bufLens.back()<<endl;
    //fwrite(bufs.back(),sizeof(bufs.back()[0]), bufLens.back(), enc);
    // io->send_data( bufs.back(), sizeof(bufs.back()[0])*bufLens.back() );
    send_data_gen( bufs.back(), sizeof(bufs.back()[0])*bufLens.back() );

    delete [] bufs.back();
    bufs.back()=nullptr;
    bufs.pop_back();
    bufLens.pop_back();
    cout<<"detail write finish"<<endl;
    for(int i=0;i<len;i++){
        //fwrite(bufs[i],sizeof(bufs[i][0]), bufLens[i], enc);
        // io->send_data( bufs[i], sizeof(bufs[i][0])*bufLens[i] );
        send_data_gen( bufs[i], sizeof(bufs[i][0])*bufLens[i] );
        delete [] bufs[i];
        bufs[i]=nullptr;
    }
    send_data_gen(&inputBufLen, sizeof(uint32_t) );
    send_data_gen(inputBuf, sizeof(inputBuf[0])*inputBufLen);
    delete [] inputBuf;
    inputBuf = nullptr;
    //fclose(enc);

    //end_write = std::chrono::system_clock::now();
    //elapsed_seconds = end_write - start;
    //t_write_sum += chrono::duration_cast<chrono::microseconds>(end_write - start).count();
    //t_write_sum += chrono::duration_cast<chrono::milliseconds>(end_write - start).count();
    //cout<<"finished write, elapsed seconds:" << chrono::duration_cast<chrono::milliseconds>(end_write - start).count() <<endl;
}

template <typename IO>
void Gen<IO>::exportBin(ShrinkedCircuit* circuit, bool* valArr){
    //std::chrono::time_point<std::chrono::system_clock> start, end;
    //start = std::chrono::system_clock::now();

    //FILE *f;
    //f = fopen((destinationPath+".bin").c_str(),"w");
    uint64_t cir_param[6];
    cir_param[0] = circuit->details.numWires;
	cir_param[1] = circuit->details.numGates;
    cir_param[2] = circuit->details.numOutputs;
    cir_param[3] = circuit->details.bitlengthInputA;
    cir_param[4] = circuit->details.bitlengthInputB;    
    cir_param[5] = circuit->details.bitlengthOutputs;
    cout<<"tmp"<<circuit->gates[0].leftParentID<<endl;

    //io->send_data( cir_param, 6*sizeof(uint64_t) );
    send_data_gen( cir_param, (size_t)6*sizeof(uint64_t) );
    //fwrite(cir_param, 1, 6*sizeof(uint64_t), f);
    //std::cout<<circuit->details.numGates<<std::endl;
    //for(int i=0;i<circuit->details.numGates;i++){
        //fwrite(&(circuit->gates[i].leftParentID),1,sizeof(uint64_t),f);
        //fwrite(&(circuit->gates[i].rightParentOffset),1,sizeof(uint64_t),f);
        //fwrite(circuit->gates[i].truthTable,1,1,f);
        //fwrite(circuit->gates, 1, circuit->details.numGates*sizeof(ShrinkedGate), f);

        //send_data_gen(&(circuit->gates[i]), sizeof(ShrinkedGate) );
    //}


    // io->send_data(circuit->gates, circuit->details.numGates*sizeof(ShrinkedGate) );

    send_data_gen(circuit->gates, (size_t)circuit->details.numGates*sizeof(ShrinkedGate) );
    send_data_gen(valArr, (size_t)circuit->details.bitlengthInputA*sizeof(bool) );
    
    //end = std::chrono::system_clock::now();
    //std::chrono::duration<double> elapsed_seconds = end - start;

    //std::cout << "finished export bin, elapsed ms:" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() <<std::endl;
        
    
    //fclose(f);
    return;
}
