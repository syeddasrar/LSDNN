#ifndef _WMAT_H_
#define _WMAT_H_

#include <stdio.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_math.h>
#include <complex>


#define dim 52

typedef double D;

typedef ap_axiu<64,0,0,0> intSdCh;
typedef std::complex<double> COMPLEX;

typedef union{
	unsigned long long int i;
	double f;
}conv;

#endif
