#include "mmse.h"
#include "rhFile.h"

void matAdd(COMPLEX Rh[dim][dim], D snr, COMPLEX tmp_Rh[dim][dim]);
//void matAdd( D snr, COMPLEX tmp_Rh[dim][dim]);
int matInv(COMPLEX A[dim][dim],  COMPLEX Ainv[dim][dim]);
void matMul(COMPLEX inv_Rh[dim][dim],COMPLEX Rh[dim][dim],COMPLEX mul_Rh[dim][dim]);
//void matMul(COMPLEX inv_Rh[dim][dim],COMPLEX mul_Rh[dim][dim]);
void eleMatMul(COMPLEX mul_Rh[dim][dim],COMPLEX xp[dim],COMPLEX W[dim][dim]);
