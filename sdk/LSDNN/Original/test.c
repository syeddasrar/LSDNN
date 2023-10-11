/*
 * test.c
 *
 *  Created on: 18-Oct-2021
 *      Author: Asrar
 *      This model is to be tested on ZC706.
 *      Least square an DNN is in a single file with top module neuralNetworkHw
 *      We have to check the functionality and time comparison between HW and SW
 */

#include "test.h"
#include "data.h"

int result_check(float HwOut[200][104],float SwOut[200][104]);
int lsDnn_sw(float lsSw_in[208], float lsSw_out[104]);

int main(){
	int status;
	conv a[208];
	conv ls[104];
	XTime start,stop;
	float TIME_HW[200];
	float TIME_SW[200];
	float TIME_HW_AVG;
	float TIME_SW_AVG;
//	uint64_t accFactor;

	float ls_hw[200][104];
	float ls_sw[200][104];
	float swOut[104];

//==============================	configuration		==============================//
	XAxiDma_Config *myDmaConfig;
	XAxiDma myDma;
	XNeuralnetworkhw_Config *neuralConfig;
	XNeuralnetworkhw myNeural;
//==============================	NN Initialization		==============================//
	neuralConfig = XNeuralnetworkhw_LookupConfig(XPAR_NEURALNETWORKHW_0_DEVICE_ID);
	status = XNeuralnetworkhw_CfgInitialize(&myNeural, neuralConfig);
	if(status != XST_SUCCESS){
		print("Neural Network initialization failed\n");
		return -1;
	}
	print("\n==============Start================\n");
////////////////////////////////
	TIME_HW_AVG = 0;
	for (int j=0;j<200;j++){
		for(int i = 0; i<104;i++){
			a[i].f = Yin[j][i];
			a[104+i].f = Xin[i];
		}
//==============================	DMA Initialization	==============================//
		myDmaConfig = XAxiDma_LookupConfigBaseAddr(XPAR_AXI_DMA_0_BASEADDR);
		status = XAxiDma_CfgInitialize(&myDma, myDmaConfig);
		if(status != XST_SUCCESS){
			print("DMA initialization failed\n");
			return -1;
		}
		//print("DMA initialization Success\n");

//==============================	disable the interrupts	==============================//
		XAxiDma_IntrDisable(&myDma, XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);
		XAxiDma_IntrDisable(&myDma, XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);

//==============================	start Neural network 	==============================//
		XNeuralnetworkhw_Start(&myNeural);
//==============================		DMA Transfer		==============================//
		Xil_DCacheFlush();
		XTime_GetTime(&start);
		status = XAxiDma_SimpleTransfer(&myDma, (u32)ls, 104*sizeof(u32),XAXIDMA_DEVICE_TO_DMA);
		status = XAxiDma_SimpleTransfer(&myDma, (u32)a, 208*sizeof(u32),XAXIDMA_DMA_TO_DEVICE);
//	    while(XAxiDma_Busy(&myDma,XAXIDMA_DMA_TO_DEVICE));
		while (!XNeuralnetworkhw_IsDone(&myNeural));
//		while(XAxiDma_Busy(&myDma,XAXIDMA_DEVICE_TO_DMA));
		if(status != XST_SUCCESS){
				print("data sent failure\n");
				return -1;
			}
		XTime_GetTime(&stop);
//==============================		Save the results		==============================//
		Xil_DCacheFlush();
		for (int i=0;i<104;i++){
			//ls_hw[j][i] = (float) (stdDev[i]*ls[i].f) + (float)mean[i] ;
			ls_hw[j][i] = ls[i].f;
			//printf("%f ",ls_hw[j][i]);
		}
		//print(";\n");
		TIME_HW[j] = ((stop-start)*1000000.0)/COUNTS_PER_SECOND;
		TIME_HW_AVG = TIME_HW_AVG + TIME_HW[j];
	}

	printf("\ntime taken by hardware : %f\n", TIME_HW_AVG/200);

//==============================	Software Implementation		==============================//
	TIME_SW_AVG = 0;
	for (int j=0;j<200;j++){
		float swIn[208];
		for(int i = 0; i<104;i++){
			swIn[i] = Yin[j][i];
			swIn[104+i] = Xin[i];
		}
		XTime_GetTime(&start);
		lsDnn_sw(swIn,swOut);
		XTime_GetTime(&stop);
		TIME_SW[j] = ((stop-start)*1000000.0)/COUNTS_PER_SECOND;
		TIME_SW_AVG = TIME_SW_AVG + TIME_SW[j];

		for (int i=0;i<104;i++){
		//	temp_hw[i][j] =ls[i].f;
			ls_sw[j][i] = (float) (stdDev[i]*swOut[i]) + (float)mean[i] ;
		}
	}
	printf("\n time taken by software : %f\n", TIME_SW_AVG/200);

	printf("\n HW Acceleration Factor = %f",TIME_SW_AVG/TIME_HW_AVG);

//============================== 	RESULT CHECK	==============================//
		status = result_check(ls_hw,ls_sw);
		if (status == -1)
		{
			print("\nFAIL!!!!!");
			return 0;
		}
		print("\nPASS!!!!!!\n");
		return 1;
}


int result_check(float HwOut[200][104],float SwOut[200][104]){
	for (int j=0;j<200;j++){
		for (int i=0;i<104;i++){
			if (((HwOut[j][i] - SwOut[j][i])>=0.00001)|((SwOut[j][i] - HwOut[j][i])>=0.00001))
			{
				print("RESULT CHECK FAILED\n");
				printf("position = (%d,%d) || software result = %f || hardware result = %f ", j,i,SwOut[j][i],HwOut[j][i]);
				printf("\nerror %f",HwOut[j][i] - SwOut[j][i]);
				return -1;
			}
		}
	}
	return 1;
}
