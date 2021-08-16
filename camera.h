#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <stdlib.h>


struct picbuffer
{
    void* start;
    int width ;
    int height;
    size_t length;
};


void openCamera(void (*fn)(struct picbuffer *n),  __uint32_t pixfmt);

int getPictures();


#endif
