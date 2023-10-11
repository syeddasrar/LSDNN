#include "calcW.h"

//void calcW(COMPLEX Rh[dim][dim],COMPLEX xp[dim],D snr,COMPLEX W[dim][dim]){
void calcW(COMPLEX xp[dim],D snr,COMPLEX W[dim][dim]){
	COMPLEX tmp_Rh[dim][dim];
	COMPLEX inv_Rh[dim][dim];
	COMPLEX mul_Rh[dim][dim];

	matAdd(Rh,snr,tmp_Rh);
//	matAdd(snr,tmp_Rh);
	matInv(tmp_Rh,inv_Rh);
	matMul(inv_Rh,Rh,mul_Rh);
//	matMul(inv_Rh,mul_Rh);
	eleMatMul(mul_Rh,xp,W);
}
