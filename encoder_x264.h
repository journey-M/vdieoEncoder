#ifndef __ENCODER__H__
#define __ENCODER__H__

#include <stdint.h>
#include <stdlib.h>



void init_endcode();

void endcode_frame(void *yuvframe, size_t yunlength);




#endif