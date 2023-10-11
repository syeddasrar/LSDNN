#include "mmse.h"


//void calcW(COMPLEX Rh[dim][dim],COMPLEX xp[dim],D snr,COMPLEX W[dim][dim]);
void calcW(COMPLEX xp[dim],D snr,COMPLEX W[dim][dim]);
void calcHfe(COMPLEX Yin[dim],COMPLEX W[dim][dim],COMPLEX MMSEOut[dim]);

void MMSED(hls::stream<intSdCh>&in_stream, hls::stream<intSdCh>&out_stream)
{
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS
#pragma HLS INTERFACE axis 		port=in_stream
#pragma HLS INTERFACE axis 		port=out_stream


	//to store received signal(Yin), pilot symbols (Xin), and LS estimated values LSOut
	COMPLEX Yin[dim];
	COMPLEX Xin[dim];
//	COMPLEX Rh[dim][dim];
	COMPLEX W[dim][dim];
	COMPLEX MMSEOut[dim];

	D snr;

	// to store streaming data
	intSdCh valIn;
	// for type conversion between AXI stream default datatype (unsigned integer) and the design specified datatype (float)
	conv y,x,l,r,s;

	// store the first 104 streamed data in Yin
	loop_y_real: for(int i=0;i<dim;i++){
		valIn = in_stream.read();
		y.i = valIn.data;
		Yin[i].real(y.f);
	}
	loop_y_imag: for(int i=0;i<dim;i++){
		valIn = in_stream.read();
		y.i = valIn.data;
		Yin[i].imag(y.f);
	}
	// store the next 104 streamed data in Xin
	loop_x_real: for(int i=0;i<dim;i++){
		valIn = in_stream.read();
		x.i = valIn.data;
		Xin[i].real(x.f);
	}
	loop_x_imag: for(int i=0;i<dim;i++){
			valIn = in_stream.read();
			x.i = valIn.data;
			Xin[i].imag(x.f);
		}
//	loop_Rh_real: for(int i=0;i<dim;i++){
//		loop_Rh_r1: for(int j=0;j<dim;j++){
//			valIn = in_stream.read();
//			r.i = valIn.data;
//			Rh[i][j].real(r.f);
//		}
//	}
//	loop_Rh_imag: for(int i=0;i<dim;i++){
//		loop_Rh_r2: for(int j=0;j<dim;j++){
//			valIn = in_stream.read();
//			r.i = valIn.data;
//			Rh[i][j].imag(r.f);
//		}
//	}
	valIn = in_stream.read();
	s.i = valIn.data;
	snr = s.f;


///////////////// MMSE estimation /////////////////////////////


//	calcW(Rh,Xin,snr,W);
	calcW(Xin,snr,W);
	calcHfe(Yin,W,MMSEOut);

//////////////////////////////////////////////////////////////////////

	// stream out the MMSE estimated values
	loop_out_real: for(int i=0;i<dim;i++){
		intSdCh valOut;
		l.f = MMSEOut[i].real();
		valOut.data = l.i;

		// generating the last signal and strobe signal
		valOut.last = 0;
		valOut.strb = -1;
		valOut.keep = 255;
		out_stream.write(valOut);
		}
	loop_out_imag: for(int i=0;i<dim;i++){
		intSdCh valOut;
		l.f = MMSEOut[i].imag();
		valOut.data = l.i;

		// generating the last signal and strobe signal
		valOut.last = (i==dim-1)?1:0;
		valOut.strb = -1;
		valOut.keep = 255;
		out_stream.write(valOut);
		}

}
