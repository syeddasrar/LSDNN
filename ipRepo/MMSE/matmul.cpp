#include "mmse.h"


void matMul(COMPLEX inv_Rh[dim][dim],COMPLEX Rh[dim][dim],COMPLEX mul_Rh[dim][dim]){
//void matMul(COMPLEX inv_Rh[dim][dim],COMPLEX mul_Rh[dim][dim]){
	loopr: for(int i=0;i<dim;i++){
		loopc: for (int j=0;j<dim;j++){
			COMPLEX result = 0;
			loopm: for (int k=0;k<dim;k++){
				result += inv_Rh[i][k] * Rh[k][j];
			}
		mul_Rh[i][j] = result;
		}
	}
}
