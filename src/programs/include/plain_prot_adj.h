#ifndef EMP_PLAIN_ENV_H__Adj
#define EMP_PLAIN_ENV_H__Adj
#include "emp-tool/emp-tool.h"
#include "circuitStructs.h"
#include <iostream>
#include <fstream>
#include <vector>
using std::endl;
using std::fstream;

namespace emp {
class PlainProtModified: public ProtocolExecution { 
public:
	uint_fast64_t n1 = 0, n2 = 0, n3 = 0;
	//std::vector<BristolCircuit> circuit;
	bool print;
	string filename;
	PlainCircExecModified * cast_circ_exec;
	std::vector<uint_fast64_t>output_vec;
	std::vector<BristolGate>* gateVec;

	PlainProtModified(bool _print, string _filename, std::vector<BristolGate>* gateVec) : print(_print), 
	filename(_filename) {
	 this->gateVec = gateVec;
	 cast_circ_exec = static_cast<PlainCircExecModified *> (CircuitExecution::circ_exec);
	}

	void finalize() override {
		if(print) {
			fstream fout(filename, std::fstream::in | std::fstream::out);
			fout<<cast_circ_exec->gates<<" "<<cast_circ_exec->gid<<endl;
			fout<<n1<<" "<<n2<<" "<<n3<<endl;
			fout.close();
		}
		
	}

	void feed(block * label, int party, const bool* b, int length) override {
		for(int i = 0; i < length; ++i)
			label[i] = cast_circ_exec->private_label(b[i]);

		if (party == ALICE) n1+=length;
		else n2+=length;
	}

	void reveal(bool* b, int party, const block * label, int length) override {
		for (int i = 0; i < length; ++i) {
			uint64_t *arr = (uint64_t*) (&label[i]);
			output_vec.push_back(arr[1]);
			b[i] = cast_circ_exec->get_value(label[i]);
		}
		n3+=length;
	}
};

inline void setup_plain_prot_adj(bool print, string filename, std::vector<BristolGate>* gateVec) {
	std::cout << "HI";
	CircuitExecution::circ_exec = new PlainCircExecModified(print, filename,  gateVec);
	ProtocolExecution::prot_exec = new PlainProtModified(print, filename, gateVec);
}

inline void finalize_plain_prot_adj (std::vector<BristolGate>* gateVec) {
	std::cout << "Tschau";
	PlainCircExecModified * cast_circ_exec = static_cast<PlainCircExecModified *> (CircuitExecution::circ_exec);
	PlainProtModified * cast_prot_exec = static_cast<PlainProtModified*> (ProtocolExecution::prot_exec);
	uint_fast64_t z_index = cast_circ_exec->gid++;
	cast_circ_exec->fout<<2<<" "<<1<<" "<<0<<" "<<0<<" "<<z_index<<" XOR"<<endl;
	for (auto v : cast_prot_exec->output_vec) {
		cast_circ_exec->fout<<2<<" "<<1<<" "<<z_index<<" "<<v<<" "<<cast_circ_exec->gid++<<" XOR"<<endl;
		gateVec->push_back(BristolGate{z_index,v, cast_circ_exec->gid++,'X'});
	}
	cast_circ_exec->gates += (1+cast_prot_exec->output_vec.size());
	cast_circ_exec->finalize();

	ProtocolExecution::prot_exec->finalize();
	delete PlainCircExecModified::circ_exec;
	delete ProtocolExecution::prot_exec;
}
}
#endif 
