#include "matInv.h"

//matrix multiply
int matInv_mul(COMPLEX A[dim][dim], COMPLEX B[dim][dim], int P[dim+1]){

	int i, j, k;
	COMPLEX C[dim][dim];
//	unsigned short addr=0;

	// invU * invD
	for(i=0;i<dim;i++){

		COMPLEX x = A[i][i];

		C[i][i] = x;

		luinv_mul_label2:for(j=0;j<i;j++)
			C[j][i] = (x * A[j][i]);

	}

	// * invL
	for(i=0;i<dim;i++){

		COMPLEX z[dim];
		COMPLEX s[dim];
		COMPLEX sr[dim];

		COMPLEX zz[dim];
		COMPLEX ss[dim];
		COMPLEX ssr[dim];

		// compute A(1, :)*B
		luinv_mul_label5:for(j=i;j<dim;j++)  s[j]=z[j]=C[i][j];

		luinv_mul_label8:for(j=1;j<dim-i;j++){

			luinv_mul_label0:for(k=i;k<dim-j;k++){

				COMPLEX this_s = (z[j+k]*A[j+k][k]);
				sr[k] = (s[k]+this_s);
			}
			// assign it back to s
			luinv_mul_label3:for(k=i;k<dim-j;k++) s[k]=sr[k];
		}
		// write it out
		luinv_mul_label4:for(j=i;j<dim;j++) C[i][j] = s[j];
		// compute A*B(:, 1)
		luinv_mul_label6:for(j=i+1;j<dim;j++){
			zz[j]=A[j][i];
			ss[j] = 0;
		}

		for(j=0;j<dim-i;j++){

			luinv_mul_label1:for(k=i+1;k<dim-j;k++){
				COMPLEX this_s = (C[k][j+k]*zz[j+k]);
				ssr[k] = (ss[k]+ this_s);
			}

			// assign it back to s
			luinv_mul_label7:for(k=i+1;k<dim-j;k++) ss[k]=ssr[k];
		}

		for(j=i+1;j<dim;j++) C[j][i] = ss[j];

	}

	// write out B
	save_data: for(i=0;i<dim;i++){
		for(j=0;j<dim;j++){
			B[i][j] = C[j][P[i]];
		}
	}

	// return status
	return(P[dim]);

}
