#include "emp-sh2pc/emp-sh2pc.h"
#include <chrono>
#include "include/plain_circ_adj.h"
#include "include/plain_prot_adj.h"
#include "include/circuitStructs.h"
#include <string>
using namespace emp;

int LEN = 2000;
int BITSIZE = 16;

Integer dubSelect3(Integer &a, Integer &b, Integer &c)
{
}
void test_setIntersectAlt3(std::string inputs_a[], std::string inputs_b[])
{
    //Integer product(BITSIZE, 1);
    Integer intersect[LEN];

    Integer a[LEN];
    Integer b[LEN];
    Integer mixed[LEN * 2];
    for (int i = 0; i < LEN; i++)
    {
        mixed[i] = Integer(BITSIZE, stoi(inputs_a[i]), ALICE);
        mixed[LEN + i] = Integer(BITSIZE, stoi(inputs_b[i]), BOB);
    }

    for (int i = 0; i < LEN; i++)
        intersect[i] = Integer(BITSIZE, 0, PUBLIC);

    sort(mixed, LEN * 2);

    int j = 0;
    for (int i = 0; i < LEN - 1; i += 1)
    {

        //Bit res = mixed[j].equal(mixed[j+1]) | mixed[j+1].equal(mixed[j+2]);
        Integer bitwiseXNOR1 = Integer(BITSIZE, 0, PUBLIC);
        Integer bitwiseXNOR2 = Integer(BITSIZE, 0, PUBLIC);
        Bit sigToOneAND1 = Bit(true);
        Bit sigToOneAND2 = Bit(true);

        for (int z = 0; z < BITSIZE; z++)
        {
            bitwiseXNOR1[z] = mixed[j][z] == mixed[j + 1][z];
            bitwiseXNOR2[z] = mixed[j + 1][z] == mixed[j + 2][z];
            sigToOneAND1 = sigToOneAND1 & bitwiseXNOR1[z];
            sigToOneAND2 = sigToOneAND2 & bitwiseXNOR2[z];
        }

        Bit OR = sigToOneAND1 | sigToOneAND2; //OR = 1 -> matching found

        for (int z = 0; z < BITSIZE; z++)
        {
            intersect[i][z] = mixed[j + 1][z] & OR;
        }

        //std::cout << "index " << i << ": " << intersect[i].reveal<int>() << endl;
        intersect[i].reveal<int>();

        j += 2;
    }

    //Additional case need if circuit is uneven
    Integer bitwiseXNOR = Integer(BITSIZE, 0, PUBLIC);
    Bit sigToOneAND = Bit(true);
    for (int z = 0; z < BITSIZE; z++)
    {
        bitwiseXNOR[z] = mixed[LEN * 2 - 1][z] == mixed[LEN * 2 - 2][z];
        sigToOneAND = sigToOneAND & bitwiseXNOR[z];
        intersect[LEN - 1][z] = mixed[LEN * 2 - 1][z] & sigToOneAND;
    }

    //std::cout << "index " << LEN-1 << ": " << intersect[LEN-1].reveal<int>() << endl;
    intersect[LEN - 1].reveal<int>();
}

void generateCircuitRAMPaper(std::vector<BristolGate>* gateVec, CircuitDetails* details, bool print)
{

    setup_plain_prot_adj(print, "setIntersectAlt3.circuit.txt", gateVec, details);
    std::string inputs[LEN];
    for (int i = 0; i < LEN; i++)
    {
        inputs[i] = "0";
    }
    test_setIntersectAlt3(inputs, inputs);
    finalize_plain_prot_adj(print, gateVec, details);
}