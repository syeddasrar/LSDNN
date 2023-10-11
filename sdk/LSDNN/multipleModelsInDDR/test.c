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
#include "model_sw.h"

int result_check(float HwOut[200][104],float SwOut[200][104]);
int lsDnn_sw(float lsSw_in[208], float lsSw_out[104], u32 modNum);
void init_inputBuffer(int k, u32 modNum, conv a[inputBufferSize]);
static void dmaReceiveISR(void *CallBackRef);
XScuGic IntcInstance;
int done = 0;

int main(){

	 init_platform();
	int status;
	conv a[inputBufferSize];
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
	u32 modNum = 2;

//==============================	configuration		==============================//
	XAxiDma_Config *myDmaConfig;
	XAxiDma myDma;
//Interrupt Controller Configuration
	XScuGic_Config *IntcConfig;
	IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	status =  XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);

	if(status != XST_SUCCESS){
		xil_printf("Interrupt controller initialization failed..");
		return -1;
	}

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&IntcInstance);
	Xil_ExceptionEnable();

	print("\n==============Start================\n");
////////////////////////////////
	TIME_HW_AVG = 0;
	for (int j=0;j<200;j++){
	//	for(int i = 0; i<104;i++){
		//	a[i].f = d0_Yin[j][i];
			//a[104+i].f = d0_Xin[i];
		//}
		init_inputBuffer(j,modNum,a);
//==============================	DMA Initialization	==============================//
		myDmaConfig = XAxiDma_LookupConfigBaseAddr(XPAR_AXI_DMA_0_BASEADDR);
		status = XAxiDma_CfgInitialize(&myDma, myDmaConfig);
		if(status != XST_SUCCESS){
			print("DMA initialization failed\n");
			return -1;
		}
		//print("DMA initialization Success\n");

//==============================	enable the interrupts	==============================//
	XAxiDma_IntrEnable(&myDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XScuGic_SetPriorityTriggerType(&IntcInstance,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,0xA1,3);
	status = XScuGic_Connect(&IntcInstance,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,(Xil_InterruptHandler)dmaReceiveISR,(void *)&myDma);
	if(status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
	XScuGic_Enable(&IntcInstance,XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR);
//==============================		DMA Transfer		==============================//
		Xil_DCacheFlushRange((INTPTR)a, (u32)inputBufferSize*sizeof(u32));
		Xil_DCacheFlushRange((INTPTR)ls, (u32)104*sizeof(u32));
		XTime_GetTime(&start);
		status = XAxiDma_SimpleTransfer(&myDma, (u32)ls, 104*sizeof(u32),XAXIDMA_DEVICE_TO_DMA);
		status = XAxiDma_SimpleTransfer(&myDma, (u32)a, inputBufferSize*sizeof(u32),XAXIDMA_DMA_TO_DEVICE);
//========================	Wait for transfer to complete 		=========================//
		status = (XAxiDma_ReadReg(XPAR_AXI_DMA_0_BASEADDR, 0x4)) & XAXIDMA_IDLE_MASK;//XAXIDMA_IDLE_MASK;
			//printf("\n\r status =  %d ",status);
		while(status != 2)
			{
			status = (XAxiDma_ReadReg(XPAR_AXI_DMA_0_BASEADDR, 0x4)) & XAXIDMA_IDLE_MASK;//XAXIDMA_IDLE_MASK;
			}
		//printf("\n\r status =  %d ",status);
		status = (XAxiDma_ReadReg(XPAR_AXI_DMA_0_BASEADDR, 0x34)) & XAXIDMA_IDLE_MASK;//XAXIDMA_IDLE_MASK;
		while(status != 2)
			{
			status = (XAxiDma_ReadReg(XPAR_AXI_DMA_0_BASEADDR, 0x34)) & XAXIDMA_IDLE_MASK;//XAXIDMA_IDLE_MASK;
			}

//		while(!done){}
//		while(XAxiDma_Busy(&myDma,XAXIDMA_DEVICE_TO_DMA));
		XTime_GetTime(&stop);
//==============================		Save the results		==============================//
		Xil_DCacheInvalidateRange((INTPTR)ls, (u32)104*sizeof(u32));;
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
			swIn[i] = d0_Yin[j][i];
			swIn[104+i] = d0_Xin[i];
		}
		XTime_GetTime(&start);

		lsDnn_sw(swIn,swOut,modNum);

		XTime_GetTime(&stop);
		TIME_SW[j] = ((stop-start)*1000000.0)/COUNTS_PER_SECOND;
		TIME_SW_AVG = TIME_SW_AVG + TIME_SW[j];

		for (int i=0;i<104;i++){
			ls_sw[j][i] =swOut[i];
			//ls_sw[j][i] = (float) (d0_stdDev[i]*swOut[i]) + (float)d0_mean[i] ;
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
		 cleanup_platform();
}

static void dmaReceiveISR(void *CallBackRef){
	XAxiDma_IntrDisable((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	done = 1;
	XAxiDma_IntrEnable((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
}

int result_check(float HwOut[200][104],float SwOut[200][104]){
	for (int j=0;j<200;j++){
		for (int i=0;i<104;i++){
			if (((HwOut[j][i] - SwOut[j][i])>=0.001)|((SwOut[j][i] - HwOut[j][i])>=0.001))
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


void init_inputBuffer(int k,u32 modNum, conv a[inputBufferSize]){
		int t =0;
		int i, j;
		int dim = 52;

		inWeightLoopR1: for(j=0;j<104;j++){
			inWeightLoopC1: for (i=0;i<52;i++){
				if (modNum==0){
					a[t].f = (float)d0_l1weights[j][i];
				}
				else if (modNum==1){
					a[t].f = (float)d1_l1weights[j][i];
				}
				else if (modNum==2){
					a[t].f = (float)d2_l1weights[j][i];
				}
				else if (modNum==3){
					a[t].f = (float)d3_l1weights[j][i];
				}
				t = t+1;
			}
		}
		inBias1:for (i=0;i<52;i++){
			if (modNum==0){
				a[t].f = (float)d0_l1bias[i];
			}
			else if (modNum==1){
				a[t].f = (float)d1_l1bias[i];
			}
			else if (modNum==2){
				a[t].f = (float)d2_l1bias[i];
			}
			else if (modNum==3){
				a[t].f = (float)d3_l1bias[i];
			}
			t = t + 1;
		}
		inWeightLoopR2: for(j=0;j<52;j++){
			inWeightLoopC2: for (i=0;i<104;i++){
				if (modNum==0){
					a[t].f = (float)d0_l2weights[j][i];
				}
				else if (modNum==1){
					a[t].f = (float)d1_l2weights[j][i];
				}
				else if (modNum==2){
					a[t].f = (float)d2_l2weights[j][i];
				}
				else if (modNum==3){
					a[t].f = (float)d3_l2weights[j][i];
				}
			t = t+1;
			}
		}
		inBias2:for (i=0;i<104;i++){
			if (modNum==0){
				a[t].f = (float)d0_l2bias[i];
			}
			else if (modNum==1){
				a[t].f = (float)d1_l2bias[i];
			}
			else if (modNum==2){
				a[t].f = (float)d2_l2bias[i];
			}
			else if (modNum==3){
				a[t].f = (float)d3_l2bias[i];
			}
			t = t+1;
		}
		inMean:for (i=0;i<104;i++){
			if (modNum==0){
				a[t].f = (float)d0_mean_i[i];
			}
			else if (modNum==1){
				a[t].f = (float)d1_mean_i[i];
			}
			else if (modNum==2){
				a[t].f = (float)d2_mean_i[i];
			}
			else if (modNum==3){
				a[t].f = (float)d3_mean_i[i];
			}
			t =t+1;
		}
		inStdDev:for (i=0;i<104;i++){
			if (modNum==0){
				a[t].f = (float)d0_stdDev_i[i];
			}
			else if (modNum==1){
				a[t].f = (float)d1_stdDev_i[i];
			}
			else if (modNum==2){
				a[t].f = (float)d2_stdDev_i[i];
			}
			else if (modNum==3){
				a[t].f = (float)d3_stdDev_i[i];
			}
			t = t+1;
		}
		inMean_o:for (i=0;i<104;i++){
			if (modNum==0){
				a[t].f = (float)d0_mean_o[i];
			}
			else if (modNum==1){
				a[t].f = (float)d1_mean_o[i];
			}
			else if (modNum==2){
				a[t].f = (float)d2_mean_o[i];
			}
			else if (modNum==3){
				a[t].f = (float)d3_mean_o[i];
			}
			t = t+1;
		}
		inStdDev_o:for (i=0;i<104;i++){
			if (modNum==0){
				a[t].f = (float)d0_stdDev_o[i];
			}
			else if (modNum==1){
				a[t].f = (float)d1_stdDev_o[i];
			}
			else if (modNum==2){
				a[t].f = (float)d2_stdDev_o[i];
			}
			else if (modNum==3){
				a[t].f = (float)d3_stdDev_o[i];
			}
			t = t+1;
		}
		for(int i = 0; i<2*dim;i++){
			if (modNum==0){
				a[t].f = d0_Yin[k][i];
			}
			else if (modNum==1){
				a[t].f = d1_Yin[k][i];
			}
			else if (modNum==2){
				a[t].f = d2_Yin[k][i];
			}
			else if (modNum==3){
				a[t].f = d3_Yin[k][i];
			}
			//printf("\nYin: %f\t",a[t].f);
			t =t+1;
		}
		for(int i = 0; i<2*dim;i++){
			if (modNum==0){
				a[t].f = d0_Xin[i];
			}
			else if (modNum==1){
				a[t].f = d1_Xin[i];
			}
			else if (modNum==2){
				a[t].f = d2_Xin[i];
			}
			else if (modNum==3){
				a[t].f = d3_Xin[i];
			}
			t =t+1;
		}
		//printf("\n t = %d\n",t);
		//print("\n\n\n\n\n");

}
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
			lsSw_out[i] = (stdDev_o[i]*t_lsSw_out[i]) + mean_o[i] ;
		}
	return 1;
}
