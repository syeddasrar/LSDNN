/*
 * test.h
 *
 *  Created on: 18-Oct-2021
 *      Author: Asrar
 */

#ifndef SRC_TEST_H_
#define SRC_TEST_H_

#include <complex.h>
#include <stdio.h>

#include "xaxidma.h"
#include "xparameters.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xtime_l.h"

#include "xneuralnetworkhw.h"


typedef float complex COMPLEX;
typedef float dataType;
typedef union{
	u32 i;
	float f;
}conv;

#define N 104


#endif /* SRC_TEST_H_ */
