#include "nn_hw.h"
#include "models.h"

dataType relu(dataType reluIn){
	dataType reluOut = (reluIn<=(dataType)0)? (dataType)0:reluIn;
	return reluOut;
}

void neuralNetworkHw(hls::stream<intSdCh>&in_stream, hls::stream<intSdCh>&out_stream, int modNum){
	#pragma HLS INTERFACE s_axilite port=modNum bundle=CTRL_BUS
	#pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS
	#pragma HLS INTERFACE axis 		port=in_stream
	#pragma HLS INTERFACE axis 		port=out_stream


/////////////////////////// LS VARIABLES ////////////////////////////////
	dataType t_l1op[52];
#pragma HLS ARRAY_PARTITION variable=t_l1op complete dim=1
	dataType t_l2op[104];
#pragma HLS ARRAY_PARTITION variable=t_l2op complete dim=1

//to store received signal(Yin), pilot symbols (Xin), and LS estimated values LSOut
	COMPLEX Yin[dim],Xin[dim];
#pragma HLS ARRAY_PARTITION variable=Yin cyclic factor=8 dim=1
#pragma HLS ARRAY_PARTITION variable=Xin cyclic factor=8 dim=1
	COMPLEX LSOut[dim];
#pragma HLS ARRAY_PARTITION variable=LSOut cyclic factor=8 dim=1
/// Neural network variables
	dataType inVal[layer1InputSize];
#pragma HLS ARRAY_PARTITION variable=inVal cyclic factor=8 dim=1
	dataType neuron_output;
	dataType layer1Output[layer1Size];
#pragma HLS array_partition variable=layer1Output cyclic factor=8
	dataType layer2Output[layer2Size];
#pragma HLS array_partition variable=layer2Output cyclic factor=8
	// to store streaming data
//	intSdCh valIn;
//	intSdCh valOut;
	// for type conversion between AXI stream default datatype (unsigned integer) and the design specified datatype (float)
//	conv y;
//	int i,j;
///////////////// Neural network model /////////////////////
		dataType l1weights[104][52];
//#pragma HLS array_partition variable=l1weights cyclic factor=16 dim=1
		dataType l1bias[52];
//#pragma HLS ARRAY_PARTITION variable=l1bias cyclic factor=2 dim=1
		dataType l2weights[52][104];
//#pragma HLS array_partition variable=l2weights cyclic factor=16 dim=1
		dataType l2bias[104];
//#pragma HLS ARRAY_PARTITION variable=l2bias cyclic factor=2 dim=1
		dataType mean[104];
//#pragma HLS ARRAY_PARTITION variable=mean cyclic factor=8 dim=1
		dataType stdDev[104];
//#pragma HLS ARRAY_PARTITION variable=stdDev cyclic factor=8 dim=1
		dataType mean_o[104];
//#pragma HLS ARRAY_PARTITION variable=mean_o cyclic factor=2 dim=1
		dataType stdDev_o[104];
//#pragma HLS ARRAY_PARTITION variable=stdDev_o cyclic factor=2 dim=1
		// to store streaming data
		intSdCh valIn;
		intSdCh valOut;
		// for type conversion between AXI stream default datatype (unsigned integer) and the design specified datatype (float)
		conv y;
		int i,j;

#pragma HLS ARRAY_PARTITION variable=l1weights complete dim=2
#pragma HLS ARRAY_PARTITION variable=l2weights complete dim=2
#pragma HLS ARRAY_PARTITION variable=l1bias complete dim=1
#pragma HLS ARRAY_PARTITION variable=l2bias complete dim=1
#pragma HLS array_partition variable=mean cyclic factor=8
#pragma HLS array_partition variable=stdDev cyclic factor=8
#pragma HLS array_partition variable=mean_o cyclic factor=8
#pragma HLS array_partition variable=stdDev_o cyclic factor=8

///////////////////////////// Select Model /////////////////////////////////////

		loop_init1: for (i=0;i<104;i++){
#pragma HLS UNROLL factor=2
#pragma HLS PIPELINE
			if(modNum == 0){
				if(i<52)
					l1bias[i] = d0_l1bias[i];
				l2bias[i] = d0_l2bias[i];
				mean[i] = d0_mean_i[i];
				stdDev[i] = d0_stdDev_i[i];
				mean_o[i] = d0_mean_o[i];
				stdDev_o[i] = d0_stdDev_o[i];
			}
			else if(modNum == 1){
				if(i<52)
					l1bias[i] = d1_l1bias[i];
				l2bias[i] = d1_l2bias[i];
				mean[i] = d1_mean_i[i];
				stdDev[i] = d1_stdDev_i[i];
				mean_o[i] = d1_mean_o[i];
				stdDev_o[i] = d1_stdDev_o[i];
			}
			else if(modNum == 2){
				if(i<52)
					l1bias[i] = d2_l1bias[i];
				l2bias[i] = d2_l2bias[i];
				mean[i] = d2_mean_i[i];
				stdDev[i] = d2_stdDev_i[i];
				mean_o[i] = d2_mean_o[i];
				stdDev_o[i] = d2_stdDev_o[i];
			}
			else if(modNum == 3){
				if(i<52)
					l1bias[i] = d3_l1bias[i];
				l2bias[i] = d3_l2bias[i];
				mean[i] = d3_mean_i[i];
				stdDev[i] = d3_stdDev_i[i];
				mean_o[i] = d3_mean_o[i];
				stdDev_o[i] = d3_stdDev_o[i];
			}
		}

		loop_init2_1: for (i=0;i<104;i++){
			loop_init2_2:for(j=0;j<52;j++){
#pragma HLS UNROLL factor=2
#pragma HLS PIPELINE

				if(modNum == 0){
					l1weights[i][j] = d0_l1weights[i][j];
					l2weights[j][i] = d0_l2weights[j][i];
				}
				else if(modNum == 1){
					l1weights[i][j] = d1_l1weights[i][j];
					l2weights[j][i] = d1_l2weights[j][i];
				}
				else if(modNum == 2){
					l1weights[i][j] = d2_l1weights[i][j];
					l2weights[j][i] = d2_l2weights[j][i];
				}
				else if(modNum == 3){
					l1weights[i][j] = d3_l1weights[i][j];
					l2weights[j][i] = d3_l2weights[j][i];
				}
			}
		}

///////// register the streamed data////////////////////
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
		#pragma HLS UNROLL //factor=13
		//#pragma HLS PIPELINE
				if (i<52)
					t_l1op[i] = 0;
				t_l2op[i] = 0;
			}

		layer1:for (i =0;i<104;i++){
		#pragma HLS PIPELINE
			l1_1:for (j=0;j<52;j++){
		//#pragma HLS UNROLL factor=13
		//#pragma HLS PIPELINE
				t_l1op[j] = t_l1op[j] + inVal[i] * l1weights[i][j];
			}
		}

		relu_loop: for (i =0;i<52;i++){
		#pragma HLS UNROLL factor=13
		#pragma HLS PIPELINE
					layer1Output[i] = relu(t_l1op[i]+l1bias[i]);
		}

		layer2:for (i =0;i<52;i++){
		#pragma HLS PIPELINE
			l2_1:for (j=0;j<104;j++){
		//#pragma HLS UNROLL factor=13
		//#pragma HLS PIPELINE
				t_l2op[j] = t_l2op[j] + layer1Output[i] * l2weights[i][j];
			}
		}
		op_loop: for (i =0;i<104;i++){
		#pragma HLS UNROLL factor=8
		#pragma HLS PIPELINE
			layer2Output[i] = ((t_l2op[i]+l2bias[i])*stdDev_o[i]) + mean_o[i];
		}

	loop_out:for(i=0;i<layer2Size;i++){
#pragma HLS pipeline
		y.f = (float)layer2Output[i];
		//y.f = (float)((layer2Output[i]*stdDev_o[i]) + mean_o[i]);
		//	printf("\t%f\t",y.f);
		valOut.data = y.i;
		// generating the last signal and strobe signal
		valOut.last = (i==layer2Size-1)?1:0;
		valOut.strb = -1;
		valOut.keep = -1;
		out_stream.write(valOut);
	}

}















