#include "include/examplePrograms.h"
#include "emp-sh2pc/emp-sh2pc.h"
#include "include/plain_circ_adj.h"
#include "include/plain_prot_adj.h"
#include "include/circuitStructs.h"
#include <iostream>
using namespace emp;



void mult3() {

int BITSIZE = 16;

Integer product(BITSIZE, 0, PUBLIC);


	Integer a;
	Integer b;

    a = Integer(BITSIZE, 0, ALICE); //dummy inputs


    b = Integer(BITSIZE, 0, BOB); //dummy inputs



  std::cout << (a+b).reveal<uint64_t>();
  std::cout << endl;
}


void setIntersect()
{
    uint_fast64_t LEN = 40000;
    uint_fast64_t BITSIZE = 32;

    //Integer product(BITSIZE, 1);
    Integer intersect[LEN];

    Integer a[LEN];
    Integer b[LEN];
    Integer mixed[LEN * 2];
    for (uint_fast64_t i = 0; i < LEN; i++)
    {
        mixed[i] = Integer(BITSIZE, 0, ALICE); //dummy inputs
        mixed[LEN + i] = Integer(BITSIZE, 0, BOB); //dummy inputs
    }

    for (uint_fast64_t i = 0; i < LEN; i++)
        intersect[i] = Integer(BITSIZE, 0, PUBLIC);

    sort(mixed, LEN * 2);
    

    uint_fast64_t j = 0;
    for (uint_fast64_t i = 0; i < LEN - 1; i += 1)
    {

        //Bit res = mixed[j].equal(mixed[j+1]) | mixed[j+1].equal(mixed[j+2]);
        Integer bitwiseXNOR1 = Integer(BITSIZE, 0, PUBLIC);
        Integer bitwiseXNOR2 = Integer(BITSIZE, 0, PUBLIC);
        Bit sigToOneAND1 = Bit(true);
        Bit sigToOneAND2 = Bit(true);

        for (uint_fast64_t z = 0; z < BITSIZE; z++)
        {
            bitwiseXNOR1[z] = mixed[j][z] == mixed[j + 1][z];
            bitwiseXNOR2[z] = mixed[j + 1][z] == mixed[j + 2][z];
            sigToOneAND1 = sigToOneAND1 & bitwiseXNOR1[z];
            sigToOneAND2 = sigToOneAND2 & bitwiseXNOR2[z];
        }

        Bit OR = sigToOneAND1 | sigToOneAND2; //OR = 1 -> matching found

        for (uint_fast64_t z = 0; z < BITSIZE; z++)
        {
            intersect[i][z] = mixed[j + 1][z] & OR;
        }

        //std::cout << "index " << i << ": " << intersect[i].reveal<int>() << endl;
        intersect[i].reveal<uint_fast64_t>();

        j += 2;
    }

    //Additional case need if circuit is uneven
    Integer bitwiseXNOR = Integer(BITSIZE, 0, PUBLIC);
    Bit sigToOneAND = Bit(true);
    for (uint_fast64_t z = 0; z < BITSIZE; z++)
    {
        bitwiseXNOR[z] = mixed[LEN * 2 - 1][z] == mixed[LEN * 2 - 2][z];
        sigToOneAND = sigToOneAND & bitwiseXNOR[z];
        intersect[LEN - 1][z] = mixed[LEN * 2 - 1][z] & sigToOneAND;
    }

    //std::cout << "index " << LEN-1 << ": " << intersect[LEN-1].reveal<int>() << endl;
    intersect[LEN - 1].reveal<uint_fast64_t>();
}





//Find maximum element in an interval range of a 2D arr
void maxIn2DArr() {  
  uint_fast64_t LEN = 386;
  uint_fast64_t BITSIZE = 32; 

  Integer coordinates[2];

	Integer a[LEN][LEN]; // 2D arr
	Integer b[4]; // xmin, xmax. ymin. ymax

  Integer maxVal = Integer(BITSIZE,0);

  for (uint_fast64_t i=0; i<LEN; i++)
  {
      for(uint_fast64_t j=0; j<LEN; j++)
        a[i][j] = Integer(BITSIZE, 0, ALICE);  // dummy input A     
  }

  Integer indexArr[LEN];
  for (uint_fast64_t i=0; i<LEN; i++)
    indexArr[i] = Integer(BITSIZE,0,PUBLIC); // array that stores all indices
    
  for (uint_fast64_t i=0; i<4; i++)  
  {    
    b[i] = Integer(BITSIZE, 0, BOB); // dummy input B
  }

  for (uint_fast64_t i=0; i<2; i++)
    coordinates[i] = Integer(BITSIZE, 0, PUBLIC); //coordniates of max elements
    
	for(uint_fast64_t i=0; i <LEN; i++ ) 
  {   
    for(uint_fast64_t j = 0; j < LEN; j++)
    {
      Bit cond1 = indexArr[i].geq(b[0]);
      Bit cond2 = b[1].geq(indexArr[i]);
      Bit cond3 = indexArr[j].geq(b[2]);
      Bit cond4 = b[3].geq(indexArr[j]);
      Bit allcond = cond1 & cond2 & cond3 & cond4;

      maxVal = If(allcond & a[i][j].geq(maxVal), a[i][j], maxVal);
      coordinates[0] = If(allcond & a[i][j].geq(maxVal), indexArr[i], coordinates[0]);
      coordinates[1] = If(allcond & a[i][j].geq(maxVal), indexArr[j], coordinates[0]);                
    }
  }
      
    coordinates[0].reveal<uint_fast64_t>();
    coordinates[1].reveal<uint_fast64_t>();             
    
}

//Check if value b is in array a
void query() {
  uint_fast64_t LEN = 140000; //Specify length of input A
  uint_fast64_t BITSIZE = 32; //Specify bitlength of inputs
  
  Bit result = Bit(false);

	Integer a[LEN];

	Integer b;
  for (uint_fast64_t i=0; i<LEN; i++)
  {
    a[i] = Integer(BITSIZE, 0, ALICE); //create dunny integers for party A (only needed to generate circuit)
  }

  b = Integer(BITSIZE, 0, BOB); //create dunny integer for party B (only needed to generate circuit)

  for (uint_fast64_t i=0; i<LEN; i++)
    result = If(a[i].equal(b), Bit(true),result); //check if b is contained in a

  result.reveal(); //mark output gate
}

void insurance() {
  uint_fast64_t LEN = 100;
  uint_fast64_t BITSIZE = 8;
  //Integer product(BITSIZE, 1);
  Integer result = Integer(BITSIZE*4, 0, PUBLIC);

	Integer a[4][LEN];

	Integer b[4];
  for (int i=0; i<LEN; i++)
  {
    a[0][i] = Integer(BITSIZE, 0, ALICE); //dummy inputs
    a[1][i] = Integer(BITSIZE, 0, ALICE); 
    a[2][i] = Integer(BITSIZE, 0, ALICE);
    a[3][i] = Integer(BITSIZE, 0, ALICE);
  }


  for (int i=0; i<4; i++)
    b[i] = Integer(BITSIZE, 0, BOB);

  for (int i=0; i<LEN; i++)
    //result = If(a[i][0].equal(b[0]), a[i][1]*b[1],result);
    result = If(a[0][i].equal(b[0]), (a[1][i]*b[1] + a[2][i]*b[2] + a[3][i]*b[3]),result);

  result.reveal<int>();
}

void billboardAd() {
  //Integer product(BITSIZE, 1);
  uint_fast64_t LEN = 1000;
  uint_fast64_t BITSIZE = 8;

  Integer coordinates[2];

	//Integer a[LEN][LEN];
  vector<vector<Integer>> a(LEN,vector<Integer>(LEN));
	Integer b[4];

  Integer maxVal = Integer(BITSIZE,0);

  for (int i=0; i<LEN; i++)
  {
      for(int j=0; j<LEN; j++)
        a[i][j] = Integer(BITSIZE, 0, ALICE); //dummy inputs
      //a[i] = Integer(BITSIZE, stoi(inputs_a[i]), ALICE);
  }

  Integer indexArr[LEN];
  
  for (int i=0; i<LEN; i++)
    indexArr[i] = Integer(BITSIZE,i,PUBLIC);
    

  for (int i=0; i<4; i++)  
  {    
    b[i] = Integer(BITSIZE, 0, BOB);
  }
  for (int i=0; i<2; i++)
    coordinates[i] = Integer(BITSIZE, 0, PUBLIC);
    
  
	for(int i=0; i <LEN; i++ ) 
  {   
    for(int j = 0; j < LEN; j++)
    {
      Bit cond1 = indexArr[i].geq(b[0]);
      Bit cond2 = b[1].geq(indexArr[i]);
      Bit cond3 = indexArr[j].geq(b[2]);
      Bit cond4 = b[3].geq(indexArr[j]);
      Bit allcond = cond1 & cond2 & cond3 & cond4;
    
      maxVal = If(allcond & a[i][j].geq(maxVal), a[i][j], maxVal);
      coordinates[0] = If(allcond & a[i][j].geq(maxVal), indexArr[i], coordinates[0]);
      coordinates[1] = If(allcond & a[i][j].geq(maxVal), indexArr[j], coordinates[0]);
    }
  }
    coordinates[0].reveal<int>();
    coordinates[1].reveal<int>();  
}