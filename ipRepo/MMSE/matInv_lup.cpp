#include "matInv.h"

void matInv_lup(COMPLEX C[dim][dim], COMPLEX B[dim][dim], int P[dim+1]){

	int i, j, k;

//	cfloat_t C[NL][NL];
	COMPLEX u[dim];
	COMPLEX l[dim];
	COMPLEX v[dim];
	int stat = 0;
	unsigned short addr=0;


	// copy computed results.
//	load_data: for(i=0;i<NL;i++)  for(j=0;j<NL;j++) C[j][i] = A[j][i];

	// loop for submatrix
	for(k=0; k<dim; k++){

		D big=-2.0;
		D pwr;
		int pos;
		COMPLEX invx;

		//pivoting
		seek_max: for(i=k;i<dim;i++){
			COMPLEX tmp;
		  	D pwrr, pwri;
			tmp = C[i][k];
			pwrr = tmp.real()*tmp.real();
			pwri = tmp.imag()*tmp.imag();
			pwr = pwrr+pwri;

			if(pwr>big){
				pos = i;
				big = pwr;
			}

		}

		// check weather big is 0
		if(big==0.0){
			pos=k;
			big=1.0;
			stat=-1;
		}else{

			// swap the rows if needed
			big=1.0/big;

			if (pos!=k){
				// swap pos and k
				for(i=0;i<dim;i++){
					COMPLEX tmp=C[pos][i];
					C[pos][i] = C[k][i];
					C[k][i] = tmp;
				}
			}
		}

		// write down the shift
		P[k]=pos;

		// inv
		invx.real(C[k][k].real()*big);
		invx.imag(-C[k][k].imag()*big);

		// extract the vectors u and l first
		luinv_lu_label0:for(i=k+1;i<dim;i++){
			COMPLEX tmp = C[k][i];
			u[i] = tmp;
			v[i] = invx*tmp;

			tmp  = invx*C[i][k];
			l[i] = tmp;
			C[i][k] = tmp;
		}


		// write out the kth row
		luinv_lu_label2:for(i=0;i<dim;i++){
			B[k][i] = (i<k)? C[k][i] : (i==k)? invx : v[i];
		}

		// start processing the (19-k)x(19-k) sub-matrix
		// column by column
		luinv_lu_label3:for(i=k+1;i<dim;i++){

			COMPLEX w[dim];

			luinv_lu_label4:for(j=k+1;j<dim;j++){
				COMPLEX tmp = l[j]*u[i];
				w[j] = C[j][i]- tmp;
			}

			// below is direct pass through
			luinv_lu_label5:for(j=k+1;j<dim;j++) C[j][i]=w[j];

		}

	}// end of loop for sub-matrix

	// write out status
	P[dim]=stat;

}
