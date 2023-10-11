#include "mmse.h"
//#include "mData.h"
#include "data1.h"

void MMSED(hls::stream<intSdCh>&in_stream, hls::stream<intSdCh>&out_stream);

int main(){
	D snr= 100000;
	//float snr= 7;
		hls::stream<intSdCh> inputStream;
		hls::stream<intSdCh> outputStream;

		conv c;
		intSdCh valIn;
		for(int i = 0; i<dim;i++){
	//		c.f = realYin1[i];
			c.f = Y21[i][0];
			valIn.data = c.i;
			valIn.keep = 0;
			valIn.strb = 0;
			valIn.last= 0;
			inputStream.write(valIn);
		}
		for(int i = 0; i<dim;i++){
	//		c.f = imagYin1[i];
			c.f = Y21[52+i][0];
			valIn.data = c.i;
			valIn.keep = 0;
			valIn.strb = 0;
			valIn.last= 0;
			inputStream.write(valIn);
		}
		for(int i = 0; i<dim;i++){
			c.f = realXin[i];
			valIn.data = c.i;
			valIn.keep = 0;
			valIn.strb = 0;
			valIn.last= 0;
			inputStream.write(valIn);
		}
		for(int i = 0; i<dim;i++){
			c.f = imagXin[i];
			valIn.data = c.i;
			valIn.keep = 0;
			valIn.strb = 0;
			valIn.last= 0;
			inputStream.write(valIn);
		}
//		for(int i = 0; i<dim;i++){
//			for(int j = 0;j<dim;j++){
//				c.f = realRh[i][j];
//				valIn.data = c.i;
//				valIn.keep = 0;
//				valIn.strb = 0;
//				valIn.last= 0;
//				inputStream.write(valIn);
//			}
//		}
//		for(int i = 0; i<dim;i++){
//			for(int j = 0;j<dim;j++){
//				c.f = imagRh[i][j];
//				valIn.data = c.i;
//				valIn.keep = 0;
//				valIn.strb = 0;
//				valIn.last= 0;
//				inputStream.write(valIn);
//			}
//		}

		c.f = snr;
		valIn.data = c.i;
		valIn.keep = 0;
		valIn.strb = 0;
		valIn.last= 1;
		inputStream.write(valIn);
/////////////////////////////////////////////////////////////////////////////////
		MMSED(inputStream,outputStream);
/////////////////////////////////////////////////////////////////////////////////
		conv l;
		D mmseEstR[dim];
		D mmseEstI[dim];
		int las,str;
//		for(int n=0;n<dim;n++){
		for(int i=0;i<dim;i++){
			intSdCh valOut;
			outputStream.read(valOut);
			str = valOut.strb;
			las = valOut.last;
			l.i = valOut.data;
			mmseEstR[i] = l.f;
//			printf("\t [%f] ",mmseEstR[i]);
			}
			printf("\n");
//		}
		printf("\n\n MMSE Imag\n");
//		for(int n=0;n<dim;n++){
		for(int i=0;i<dim;i++){
			intSdCh valOut;
			outputStream.read(valOut);
			str = valOut.strb;
			las = valOut.last;
			l.i = valOut.data;
			mmseEstI[i] = l.f;
			}
			printf("\n");
//		}

/*		resultCheck: for(int i=0;i<dim;i++){
			if (((abs(mmseEstR[i]-realGold[i]))>= 0.00001) || (abs(mmseEstI[i] -imagGold[i]) >= 0.00001)){
				printf("error = %f||%f: gold = %f+%f || calc = %f+%f",abs(mmseEstR[i]-realGold[i]),abs(mmseEstI[i] -imagGold[i]),realGold[i],imagGold[i], mmseEstR[i],mmseEstI[i] );
				return -1;
			}
		}
*/

		printf("MMSE Estimate : \n");
		for(int i=0;i<52;i++){
			printf("\t [%lf+%lf] ",mmseEstR[i],mmseEstI[i]);
			printf("\n");
		}

		return 0;
	}

