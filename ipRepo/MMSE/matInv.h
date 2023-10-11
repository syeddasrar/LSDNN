#ifndef _MATINV_H
#define _MATINV_H

#include <math.h>

#include <complex>
typedef std::complex<double> COMPLEX;
typedef double D;

// number of layers, or matrix size
#define dim 52

// LU decomposition with pivoting
void matInv_lup(COMPLEX C[dim][dim], COMPLEX B[dim][dim], int P[dim+1]);
void matInv_inv(COMPLEX A[dim][dim], COMPLEX B[dim][dim], int P[dim+1], int P_out[dim+1]);
int matInv_mul(COMPLEX A[dim][dim], COMPLEX B[dim][dim], int P[dim+1]);


#endif
