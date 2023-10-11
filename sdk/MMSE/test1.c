#include "test.h"
#include "mData.h"
typedef union{
	u64 i;
	double f;
}conv;

typedef double T;
typedef u64 U;


#define dim 52

int main(){
	xil_printf("start\n");
	int dataTransfer = 104+104+1;
	conv a[dataTransfer];
	conv ls[2*dim];
	XTime start,stop;
	float TIME_HW = 0;

	int snr_index = 21;   /////////////////////////////   1       ////////////////////////
	T snr= snr_val[snr_index-1];


	printf("\nsnr = %d: %lf\n",snr_index,snr);

    u32 status;

 	XAxiDma_Config *myDmaConfig;
	XAxiDma myDma;

	T temp_hw[104][200];
	//float temp_hwR[52][52];
	//float temp_hwI[52][52];
	for(int n=0;n<5;n++){
	int t = 0;
	for(int i = 0; i<dim;i++){
				a[t++].f = Y21[i][n];   /////////////////////////  2         ///////////////////////
			}
			for(int i = 0; i<dim;i++){
				a[t++].f = Y21[52+i][n];  /////////////////////          3         ////////////////////////
			}
			for(int i = 0; i<dim;i++){
				a[t++].f = realXin[i];
			}
			for(int i = 0; i<dim;i++){
				a[t++].f = imagXin[i];
			}
			a[t].f = snr;

	myDmaConfig = XAxiDma_LookupConfigBaseAddr(XPAR_AXI_DMA_0_BASEADDR);
	print("\nDMA config done\n");

	status = XAxiDma_CfgInitialize(&myDma, myDmaConfig);
	if(status != XST_SUCCESS){
		print("DMA initialization failed\n");
		return -1;
	}
	print("DMA initialization success..\n");
// disable the interrupts
	XAxiDma_IntrDisable(&myDma, XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrDisable(&myDma, XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);

	Xil_DCacheFlush();
// initialize DMA transfer
	XTime_GetTime(&start);
	status = XAxiDma_SimpleTransfer(&myDma, (u32)ls, 52*2*sizeof(U),XAXIDMA_DEVICE_TO_DMA);
	if(status != XST_SUCCESS){
		print("DMA transfer failed\n");
		return -1;
	}
	//print("DMA transfer success\n");
	status = XAxiDma_SimpleTransfer(&myDma, (u32)a, dataTransfer*sizeof(U),XAXIDMA_DMA_TO_DEVICE);
	if(status != XST_SUCCESS){
			print("data sent failure\n");
			return -1;
		}
	//print("DMA sent success\n");
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
	Xil_DCacheInvalidateRange(ls, 52*2*sizeof(U));
	XTime_GetTime(&stop);

	TIME_HW = TIME_HW + ((stop-start)*1000000.0)/COUNTS_PER_SECOND;

	// display the output
	    Xil_DCacheFlush();
    for (int i=0;i<2*52;i++){
    		temp_hw[i][n] = ls[i].f;
    		printf("%lf \t", ls[i].f);
    }
    printf("\n");
	}


    TIME_HW = TIME_HW/5;

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////


//	print("DMA transfer success..\n");
//	float TIME_HW = ((stop-start)*1000000.0)/COUNTS_PER_SECOND;
		    printf(" TIME_HW = %f",TIME_HW);
		    xil_printf("DONE");
			return 0;
	//////////////////////////////////////////////////////////////////////////////
}
