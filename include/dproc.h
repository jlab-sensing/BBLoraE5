/*******************************************************************************
 * Created by: Brian Govers
 * Name: dproc.c
 * Date of Creation: 27 July 2022
 ******************************************************************************/

#include "csv.h"

#define BUFFER_SIZE 1024
#define BUFFER_SIZE 1024
#define NUM_SENSORS 2
#define NUM_SAMPLES 499

enum fields{TIMESTAMP = 0, I1LV, I2LV, I1H, I1L, V1, V2, I2H, I2L};