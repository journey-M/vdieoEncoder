#ifndef __YUV_SAVER__
#define __YUV_SAVER__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "yuv_saver.h"

FILE *file_fd;


void yuv_saver(void *yuv_frame, size_t length){

    if (!file_fd)
    {
        file_fd = fopen("abc.yuv", "wb");
    }
    
    fwrite(yuv_frame, sizeof(char), length, file_fd);

}


#endif