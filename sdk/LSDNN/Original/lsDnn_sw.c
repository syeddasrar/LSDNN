/*
 * lsDnn_sw.c
 *
 *  Created on: 18-Oct-2021
 *      Author: Asrar
 */

#include "test.h"
#include "model_sw.h"

float relu(float reluIn){
	float reluOut = (reluIn<=0)? (float)0:reluIn;
	return reluOut;
}

int lsDnn_sw(float lsSw_in[208], float lsSw_out[104]){
	int i,j;
	COMPLEX sigIn[N],pilIn[N];
	COMPLEX LSOut[N];
	float inVal[104];

static int layer1Size = 52;
static int layer2Size = 104;
static int layer1InputSize = 104;

	float neuron_output;
	float layer1Output[layer1Size];
//	float layer2Output[layer2Size];

	input_loop: for(int i=0;i<52;i++){
		sigIn[i] = lsSw_in[i]+lsSw_in[52+i]*I;
		pilIn[i] = lsSw_in[104+i]+lsSw_in[104+52+i]*I;
		}

///////////////// LS estimation /////////////////////////////
		loop_ls: for (i = 0; i <52; i++){
			LSOut[i] =  sigIn[i]/pilIn[i];
			inVal[i]=(dataType)((dataType)crealf(LSOut[i])-mean_i[i])/(dataType)stdDev_i[i];
			inVal[52+i]=(dataType)((dataType)cimagf(LSOut[i])-mean_i[52+i])/(dataType)stdDev_i[52+i];
		}
////////////////////////////////////////////////////////////////////////////////////////////
	layer1:for(i=0;i<layer1Size;i++){
		neuron_output = 0;
		layer1Neurons:for(j=0;j<layer1InputSize;j++){
			neuron_output = neuron_output + l1weights[j][i]*inVal[j];
		}
		layer1Output[i] = relu(neuron_output + l1bias[i]);
	}

	layer2:for(i=0;i<layer2Size;i++){
	#pragma HLS pipeline
		neuron_output = 0;
		layer2Neurons:for(j=0;j<layer1Size;j++){
			neuron_output = neuron_output + l2weights[j][i]*layer1Output[j];
		}
		lsSw_out[i] = neuron_output + l2bias[i];
	}
	return 1;
}
