#include "nn_hw.h"

dataType relu(dataType reluIn){
	dataType reluOut = (reluIn<=0)? (dataType)0:reluIn;
	return reluOut;
}

void neuralNetworkHw(hls::stream<intSdCh>&in_stream, hls::stream<intSdCh>&out_stream){

	#pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS
	#pragma HLS INTERFACE axis 		port=in_stream
	#pragma HLS INTERFACE axis 		port=out_stream
/////////////////////////// LS VARIABLES ////////////////////////////////
	//to store received signal(Yin), pilot symbols (Xin), and LS estimated values LSOut
	COMPLEX Yin[dim],Xin[dim];
#pragma HLS ARRAY_PARTITION variable=Yin cyclic factor=16 dim=1
#pragma HLS ARRAY_PARTITION variable=Xin cyclic factor=16 dim=1
	COMPLEX LSOut[dim];
#pragma HLS ARRAY_PARTITION variable=LSOut cyclic factor=16 dim=1
	/// Neural network variables
	dataType inVal[layer1InputSize];
#pragma HLS array_partition variable=inVal cyclic factor=16
	dataType neuron_output;
	dataType layer1Output[layer1Size];
#pragma HLS array_partition variable=layer1Output cyclic factor=16
	dataType layer2Output[layer2Size];
#pragma HLS array_partition variable=layer2Output cyclic factor=16

///////////////// Neural network model /////////////////////
		dataType l1weights[104][52];
#pragma HLS array_partition variable=l1weights cyclic factor=16 dim=1
		dataType l1bias[52];
		dataType l2weights[52][104];
#pragma HLS array_partition variable=l2weights cyclic factor=16 dim=1
		dataType l2bias[104];
		dataType mean[104];
#pragma HLS ARRAY_PARTITION variable=mean cyclic factor=16 dim=1
		dataType stdDev[104];
#pragma HLS ARRAY_PARTITION variable=stdDev cyclic factor=16 dim=1
		dataType mean_o[104];
//#pragma HLS ARRAY_PARTITION variable=mean cyclic factor=16 dim=1
		dataType stdDev_o[104];
//#pragma HLS ARRAY_PARTITION variable=stdDev cyclic factor=16 dim=1
		// to store streaming data
		intSdCh valIn;
		intSdCh valOut;
		// for type conversion between AXI stream default datatype (unsigned integer) and the design specified datatype (float)
		conv y;
		int i,j;

	///////// register the streamed data////////////////////

			inWeightLoopR1: for(j=0;j<104;j++){
				inWeightLoopC1: for (i=0;i<52;i++){
#pragma HLS PIPELINE
					valIn = in_stream.read();
					y.i = (unsigned int)valIn.data;
					l1weights[j][i]  = (dataType)y.f;
				}
			}
//			printf("l1weight done");

			inBias1:for (i=0;i<52;i++){
#pragma HLS PIPELINE
				valIn = in_stream.read();
				y.i = (unsigned int)valIn.data;
				l1bias[i]  = (dataType)y.f;
			}
//			printf("l1bias done");
			inWeightLoopR2: for(j=0;j<52;j++){
				inWeightLoopC2: for (i=0;i<104;i++){
#pragma HLS PIPELINE
					valIn = in_stream.read();
					y.i = (unsigned int)valIn.data;
					l2weights[j][i]  = (dataType)y.f;
				}
			}
//			printf("l2weight done");
			inBias2:for (i=0;i<104;i++){
#pragma HLS PIPELINE
				valIn = in_stream.read();
				y.i = (unsigned int)valIn.data;
				l2bias[i]  = (dataType)y.f;
			}
//			printf("l2 bias done");

			inMean:for (i=0;i<104;i++){
#pragma HLS PIPELINE
				valIn = in_stream.read();
				y.i = (unsigned int)valIn.data;
				mean[i]  = (dataType)y.f;
			}
//			printf("mean done");

			inStdDev:for (i=0;i<104;i++){
#pragma HLS PIPELINE
				valIn = in_stream.read();
				y.i = (unsigned int)valIn.data;
				stdDev[i]  = (dataType)y.f;
			}
			inMean_o:for (i=0;i<104;i++){
			#pragma HLS PIPELINE
				valIn = in_stream.read();
				y.i = (unsigned int)valIn.data;
				mean_o[i]  = (dataType)y.f;
			}
//			printf("mean done");

			inStdDev_o:for (i=0;i<104;i++){
#pragma HLS PIPELINE
				valIn = in_stream.read();
				y.i = (unsigned int)valIn.data;
				stdDev_o[i]  = (dataType)y.f;
			}

	loop_in:for(i=0;i<4*dim;i++){
	#pragma HLS PIPELINE
		valIn = in_stream.read();
		y.i = valIn.data;
		if (i<dim)
			Yin[i].real((dataType)y.f);
		else if ((i>=dim)&&(i<2*dim))
			Yin[i-dim].imag((dataType)y.f);
		else if ((i>=2*dim)&&(i<3*dim))
			Xin[i-2*dim].real((dataType)y.f);
		else
			Xin[i-3*dim].imag((dataType)y.f);
	}
	///////////////// LS estimation /////////////////////////////
		loop_ls: for (i = 0; i <dim; i++){
#pragma HLS UNROLL factor=8
#pragma HLS PIPELINE
			LSOut[i] =  Yin[i]/Xin[i];
			inVal[i]=(dataType)((dataType)LSOut[i].real()-mean[i])/(dataType)stdDev[i];
			inVal[dim+i]=(dataType)((dataType)LSOut[i].imag()-mean[dim+i])/(dataType)stdDev[dim+i];
		}

		l1:for (i=0;i<104;i++){
	#pragma HLS UNROLL
			if (i<52)
				t_l1op[i] = 0;
			t_l2op[i] = 0;
		}

	layer1:for (i =0;i<104;i++){
	#pragma HLS PIPELINE
		l1_1:for (j=0;j<52;j++){
			t_l1op[j] = t_l1op[j] + inVal[i] * (dataType)l1weights[i][j];
		}
	}

	relu_loop: for (i =0;i<52;i++){
	#pragma HLS UNROLL factor=13
	#pragma HLS PIPELINE
				layer1Output[i] = relu(t_l1op[i]+(dataType)l1bias[i]);
	}

	layer2:for (i =0;i<52;i++){
	#pragma HLS PIPELINE
		l2_1:for (j=0;j<104;j++){
			t_l2op[j] = t_l2op[j] + layer1Output[i] * (dataType)l2weights[i][j];
		}
	}
	op_loop: for (i =0;i<104;i++){
	#pragma HLS UNROLL factor=8
	#pragma HLS PIPELINE
		layer2Output[i] = ((t_l2op[i]+(dataType)l2bias[i])*(dataType)stdDev_o[i]) + (dataType)mean_o[i];
	}
	loop_out:for(i=0;i<layer2Size;i++){
#pragma HLS pipeline
		y.f = (float)((layer2Output[i]*stdDev_o[i]) + mean_o[i]);
	//	printf("\t%f\t",y.f);
		valOut.data = y.i;
		// generating the last signal and strobe signal
		valOut.last = (i==layer2Size-1)?1:0;
		valOut.strb = -1;
		valOut.keep = -1;
		out_stream.write(valOut);
	}

}
