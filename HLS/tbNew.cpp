#include "nn_hw.h"
#include <math.h>
#include "data1.h"

//typedef std::complex<double> COMPLEXD;
typedef std::complex<double> COMPLEXD;

void neuralNetworkHw(hls::stream<intSdCh> &in_stream, hls::stream<intSdCh> &out_stream);

int main(){
	float mean[104] = {-0.002808,-0.001326,0.000626,0.002345,0.003239,0.002896,0.001246,-0.001222,-0.003451,-0.004236,-0.002930,0.000023,0.003181,0.004859,0.004104,0.001257,-0.002237,-0.004620,-0.004741,-0.002591,0.000752,0.003690,0.004859,0.003752,0.000921,-0.002293,-0.004467,-0.002500,0.000579,0.003386,0.004681,0.003865,0.001238,-0.002077,-0.004539,-0.004866,-0.002738,0.000880,0.004109,0.005169,0.003463,0.000001,-0.003216,-0.004451,-0.003285,-0.000716,0.001671,0.002785,0.002531,0.001510,0.000341,-0.000731,0.001447,0.002818,0.003042,0.002103,0.000299,-0.001800,-0.003386,-0.003630,-0.002140,0.000629,0.003394,0.004656,0.003587,0.000604,-0.002793,-0.004841,-0.004487,-0.001927,0.001541,0.004225,0.004867,0.003213,0.000083,-0.003039,-0.004707,-0.004175,0.001466,0.003954,0.004630,0.003215,0.000339,-0.002742,-0.004644,-0.004428,-0.002060,0.001443,0.004366,0.005130,0.003217,-0.000402,-0.003737,-0.004946,-0.003476,-0.000361,0.002526,0.003711,0.002948,0.001097,-0.000692,-0.001760,-0.002095,-0.001931};
	float stdDev[104]  = {0.705393,0.712541,0.715544,0.709867,0.706533,0.711260,0.715011,0.711802,0.708324,0.710597,0.713940,0.712523,0.709451,0.710540,0.713497,0.712330,0.709155,0.710466,0.713859,0.712066,0.707861,0.709730,0.714688,0.712869,0.706931,0.708359,0.714862,0.707370,0.706656,0.714426,0.716525,0.708920,0.704986,0.711323,0.716129,0.710855,0.704407,0.707100,0.713564,0.712918,0.706221,0.704131,0.710105,0.714646,0.710161,0.703809,0.706814,0.714737,0.714054,0.705657,0.704174,0.712392,0.715201,0.708467,0.705799,0.711840,0.715439,0.710991,0.707433,0.710839,0.714450,0.712305,0.709040,0.710519,0.713607,0.712504,0.709485,0.710558,0.713594,0.712131,0.708558,0.710194,0.714244,0.712286,0.707261,0.709102,0.715087,0.713781,0.707534,0.715123,0.715865,0.708058,0.705772,0.713134,0.716639,0.709866,0.704464,0.709208,0.715051,0.711879,0.704976,0.705325,0.711860,0.713892,0.708034,0.703633,0.708405,0.714984,0.712271,0.704536,0.705378,0.713845,0.715294,0.707028};


	hls::stream<intSdCh> inputStream;
	hls::stream<intSdCh> outputStream;

	float ls[200][2*dim];

	for (int j=0;j<200;j++){
		for(int i = 0; i<4*dim;i++){
			conv c;
			if(i<2*dim)
				c.f = Yin[j][i];
			else
				c.f = Xin[i-2*dim];
			intSdCh valIn;
			valIn.data = c.i;
			valIn.keep = 0;
			valIn.strb = 0;
			valIn.last=(i==4*dim-1)?1:0;
			inputStream.write(valIn);
		}
		neuralNetworkHw(inputStream,outputStream);
		conv l;

				int las,str;
		for(int i=0;i<2*dim;i++){
			intSdCh valOut;
			outputStream.read(valOut);
			str = valOut.strb;
			las = valOut.last;
			l.i = valOut.data;
			ls[j][i] = l.f;//*stdDev[i] + mean[i];
			//printf("%f\t", ls[j][i]);
		}
		//printf("\n");
	}
	//double nrm  =0;
	//double phf  =0;
	//float diff;
	//double errNorm;

	double nrm  =0;
	double phf  =0;
	double errNorm;


	COMPLEXD lsOutC[200][52];
	COMPLEXD goldRefC[200][52];

	COMPLEXD diffC;
	for(int j=0;j<200;j++){
		for(int i=0;i<52;i++){
			lsOutC[j][i].real(ls[j][i]);
			lsOutC[j][i].imag(ls[j][52+i]);
			goldRefC[j][i].real(actin[j][i]);
			goldRefC[j][i].imag(actin[j][52+i]);

		//	printf("Input: %f + j%f",lsOutC[j][i].real(),lsOutC[j][i].imag());
		//	printf("\tGold: %f + j%f",goldRefC[j][i].real(),goldRefC[j][i].imag());
		}
		//printf("\n");
	}


//	for(int j=0;j<200;j++){
//		for(int i=0;i<52;i++){
//			diffC = (lsOutC[j][i]-goldRefC[j][i]) * (lsOutC[j][i]-goldRefC[j][i]) ;
//			printf("(%d,%d) %lf\t",j,i,diffC.real());
//			nrm += diffC.real();
//			diffC = goldRefC[j][i] * goldRefC[j][i];
//			phf += diffC.real();
//		}
//		printf("\n");
//	}
	COMPLEXD diffC1;
	float d;
	for(int j=0;j<200;j++){
		for(int i=0;i<52;i++){
			diffC1 = (lsOutC[j][i]-goldRefC[j][i]);
			d = norm(pow(diffC1,2));
			nrm += d;
			d = norm(pow(goldRefC[j][i],2));
			phf += d;

		}
	}


	nrm = sqrt(nrm)/200;
	phf = sqrt(phf)/200;
	errNorm = nrm/phf;

	printf("\nnormalized error: %E\n",errNorm);



	for(int i=0;i<52;i++){
			printf("\t %f + %fi", ls[0][i],ls[0][52+i]);
			printf("\n");
		}

	// result check

	if (errNorm > 0.0001){
		printf("\nFAILED!!!\n");
		return -1;
	}
	printf("\nSUCCESS!!!\n");
	return 0;

}
