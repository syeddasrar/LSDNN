#ifndef _NEURALNETWORKHARDWARE_H_
#define _NEURALNETWORKHARDWARE_H_

#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_math.h>
#include <complex>

//#include "ap_fixed.h"

//typedef ap_fixed<16,8> dataType; //normalized error: 0.012021
//typedef ap_fixed<32,8> dataType; // normalized error: 0.000022

typedef float dataType;   // normalized error: 0.000022

//#include "hls_half.h"
//typedef half dataType;   // normalized error: 0.000024



#define dim 52

#define layer1Size 52
#define layer2Size 104
#define layer1InputSize 104

typedef ap_axiu<32,0,0,0> intSdCh;

typedef union{
	unsigned int i;
	float f;
}conv;

typedef std::complex<dataType> COMPLEX;

#endif
