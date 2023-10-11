#ifndef _NEURALNETWORKHARDWARE_H_
#define _NEURALNETWORKHARDWARE_H_

#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_math.h>
#include <complex>

//#include "ap_fixed.h"

//typedef ap_fixed<24,8> dataType; //normalized error: 0.012021
//typedef ap_fixed<32,8> dataType; // normalized error: 0.000022
//typedef ap_fixed<32,8> dataTypeP; // normalized error: 0.000022


//typedef ap_fixed<16,8> dataType; // normalized error: 0.000022

//typedef ap_fixed<24,8> dataType;
//typedef ap_fixed<18,2> dataTypeP;


typedef float dataType;   // normalized error: 0.000022
typedef float dataTypeP;

//#include "hls_half.h"
//typedef half dataType;   // normalized error: 0.000024
//typedef half dataTypeP;


#define dim 52

#define layer1Size 52
#define layer2Size 104
#define layer1InputSize 104


// USE THE COMMENTED INTERFACE
// IN top FILE CHANGE THE loop_in AND loop_out to use the new interface

//struct intSdCh{
//	float data;
//	ap_uint<a> last;
//};

// COMMENT OUT THE TYPEDEF AND UNION
typedef ap_axiu<32,0,0,0> intSdCh;

typedef union{
	unsigned int i;
	float f;
}conv;

typedef std::complex<dataType> COMPLEX;

#endif
