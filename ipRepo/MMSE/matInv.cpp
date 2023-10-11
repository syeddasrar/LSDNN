#include "matInv.h"

int matInv(COMPLEX A[dim][dim], COMPLEX Ainv[dim][dim]){

	 // to save LU decomposition result
	COMPLEX LU[dim][dim];

	 // to save the inverse of the matrix
	COMPLEX LUinv[dim][dim];

	 // row permutation vector
	 int  P[dim+1];

	 // adjusted permutation vector
	 int  Padj[dim+1];

	 // flag whether the matrix is ill-conditioned
	 int  stat;

	 // LU decomposition with Pivoting
	 matInv_lup(A,     LU,    P);
//	 matInv_lup(A,     Ainv,    P);

	 // Inverse the triangular matrices L and U
	 matInv_inv(LU,    LUinv, P,    Padj);

	 // Multiply the inverse of triangular matrices
	 stat = matInv_mul(LUinv, Ainv, Padj);

	 // stat=-1 for ill-conditioned matrix
	 return(stat);

}
