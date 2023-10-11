#include "mmse.h"


void matAdd(COMPLEX Rh[dim][dim], D snr, COMPLEX tmp_Rh[dim][dim]){
//void matAdd(D snr, COMPLEX tmp_Rh[dim][dim]){
	int k =0;
	loop1: for(int i=0;i<dim;i++){
		loop2: for (int j=0;j<dim;j++){
			if(i==j){
				tmp_Rh[j][i].real(Rh[i][j].real() + (1/snr));
			}
			else{
				tmp_Rh[j][i].real(Rh[i][j].real());
			}
			tmp_Rh[j][i].imag(Rh[i][j].imag());
			//k=k+1;
		}
	}
}
