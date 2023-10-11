#include "mmse.h"

void eleMatMul(COMPLEX mul_Rh[dim][dim],COMPLEX xp[dim],COMPLEX W[dim][dim]){
	loop1: for(int i=0;i<dim;i++){
		loop2: for (int j=0;j<dim;j++){
			W[i][j] =	mul_Rh[i][j] * xp[j];
		}
	}
}
