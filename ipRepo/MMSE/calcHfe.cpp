#include "mmse.h"

void calcHfe(COMPLEX Yin[dim],COMPLEX W[dim][dim],COMPLEX MMSEOut[dim]){
	COMPLEX result;
	loop1: for(int i=0;i<dim;i++){
		result = 0;
		loop2: for (int k=0;k<dim;k++){
			result += W[i][k] * Yin[k];
		}
	MMSEOut[i] = result;
	}
}
