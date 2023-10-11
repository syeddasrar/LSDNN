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

int lsDnn_sw(float lsSw_in[208], float lsSw_out[104], u32 modNum){
	int i,j;
	COMPLEX sigIn[N],pilIn[N];
	COMPLEX LSOut[N];
	float inVal[104];
	float t_lsSw_out[104];
static int layer1Size = 52;
static int layer2Size = 104;
static int layer1InputSize = 104;

	float neuron_output;
	float layer1Output[layer1Size];
//	float layer2Output[layer2Size];

//	int modNum = 0;

	dataType l1weights[104][52];
	dataType l1bias[52];
	dataType l2weights[52][104];
	dataType l2bias[104];
	dataType mean[104];
	dataType stdDev[104];
	dataType mean_o[104];
	dataType stdDev_o[104];

	input_loop: for(int i=0;i<52;i++){
		sigIn[i] = lsSw_in[i]+lsSw_in[52+i]*I;
		pilIn[i] = lsSw_in[104+i]+lsSw_in[104+52+i]*I;
		}
	loop_init1: for (i=0;i<104;i++){
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

///////////////// LS estimation /////////////////////////////
		loop_ls: for (i = 0; i <52; i++){
			LSOut[i] =  sigIn[i]/pilIn[i];
			inVal[i]=(dataType)((dataType)crealf(LSOut[i])-mean[i])/(dataType)stdDev[i];
			inVal[52+i]=(dataType)((dataType)cimagf(LSOut[i])-mean[52+i])/(dataType)stdDev[52+i];
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
		neuron_output = 0;
		layer2Neurons:for(j=0;j<layer1Size;j++){
			neuron_output = neuron_output + l2weights[j][i]*layer1Output[j];
		}
		t_lsSw_out[i] = neuron_output + l2bias[i];
	}
	layerOut:for(i=0;i<layer2Size;i++){
			lsSw_out[i] = (d0_stdDev_o[i]*t_lsSw_out[i]) + d0_mean_o[i] ;
		}
	return 1;
}
