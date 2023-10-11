#include "matInv.h"

void matInv_inv(COMPLEX A[dim][dim], COMPLEX B[dim][dim], int P_in[dim+1], int P_out[dim+1]){

	int i, j, k;

	COMPLEX sl[dim-1];
	COMPLEX su[dim-1];
	COMPLEX slr[dim-1];
	COMPLEX sur[dim-1];
	int p[dim];
	//const unsigned char k=0;

	for(k=0;k<dim-1;k++){

		// copy the row to s
		luinv_inv_label0:for(i=k;i<dim-1;i++){
			sl[i]=-(A[i+1][k]);
			su[i]=-(A[k][i+1]);
		}

		//iteratively update s
		loop_column: for(i=k+1;i<dim-1;i++){

			COMPLEX this_sl = -(sl[i-1]);
			COMPLEX this_su = -(su[i-1]);

			luinv_inv_label1:for(j=i;j<dim-1;j++){
				COMPLEX l=this_sl*A[j+1][i];
				COMPLEX u=this_su * A[i][j+1];
				slr[j] = (sl[j]+l);
				sur[j] = (su[j]+u);
			}

			luinv_inv_label2:for(j=i;j<dim-1;j++){
				sl[j]=slr[j];
				su[j]=sur[j];
			}
		}

		luinv_inv_label3:for(i=k;i<dim-1;i++){
			B[i+1][k] = sl[i];
			B[k][i+1] = su[i];
		}
	}

	// copy B to A
	matInv_inv_label1:for(i=0;i<dim;i++) B[i][i] = A[i][i];

	// permute p and output
	for(i=0;i<dim;i++) p[i]=i;
	matInv_inv_label2:for(i=0;i<dim;i++){
		// exchange positions
		int m = P_in[i];
		int q = p[i];
		p[i] = p[m];
		p[m] = q;
		P_out[p[i]]=i;
	}
	P_out[dim] = P_in[dim];

}
