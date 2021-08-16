#ifndef __ENCODER__H__
#define __ENCODER__H__

#include <stdint.h>
#include <stdlib.h>



void init_endcode();

void endcode_frame(int pts, void *yuvframe,int size, int width, int height);

void yuyv_to_i420(const unsigned char *in, unsigned char *out, unsigned int width ,unsigned int height);




#endif
