#ifndef _NEURALNETWORKHARDWARE_H_
#define _NEURALNETWORKHARDWARE_H_

#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_math.h>
#include <complex>


typedef float dataType;   // normalized error: 0.000022

#define dim 52

#define layer1Size 52
#define layer2Size 104
#define layer1InputSize 104

template<int D>
	struct axi_stream {
	  ap_uint<D>       data;
	  ap_uint<(D+7)/8> keep;
	  ap_uint<1>       last;
	};

typedef axi_stream<32> intSdCh;

typedef union{
	unsigned int i;
	float f;
}conv;

typedef std::complex<dataType> COMPLEX;

#endif
