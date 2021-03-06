#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <sys/mman.h>
#include <SDL2/SDL.h>
#include <pthread.h>

#include "camera.h"


#define LOAD_BGRA    0  
#define LOAD_RGB24   0  
#define LOAD_BGR24   0  
#define LOAD_YUV422P 1  

int screen_w=640,screen_h=480;  
const int pixel_w=640,pixel_h=480;  

int quit = 0;
SDL_Texture* sdlTexture;
SDL_Rect sdlRect;     
SDL_Renderer* sdlRenderer;

/**
 * 获取到图片的buffer
 */
void onGetPictureBuffer(struct picbuffer *pBuffer){
    if (!pBuffer)
    {
        return;
    }
    
    #if LOAD_BGRA  
        //We don't need to change Endian  
        //Because input BGRA pixel data(B|G|R|A) is same as ARGB8888 in Little Endian (B|G|R|A)  
        SDL_UpdateTexture( sdlTexture, NULL, buffer, pixel_w*4);    
    #elif LOAD_RGB24|LOAD_BGR24  
        //change 24bit to 32 bit  
        //and in Windows we need to change Endian  
        CONVERT_24to32(buffer,buffer_convert,pixel_w,pixel_h);  
        SDL_UpdateTexture( sdlTexture, NULL, buffer_convert, pixel_w*4);    
    #elif LOAD_YUV422P  
        SDL_UpdateTexture( sdlTexture, &sdlRect, pBuffer->start, pixel_w*2);    

    #endif  

        //FIX: If window is resize  
        sdlRect.x = 0;    
        sdlRect.y = 0;    
        sdlRect.w = screen_w;    
        sdlRect.h = screen_h;    
        SDL_RenderClear( sdlRenderer );     
        SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);    
        SDL_RenderPresent( sdlRenderer );    
        //Delay 40ms  
        SDL_Delay(40); 
        
}


/**
 * 渲染线程，负责从camera的缓存中读取视频，然后在sdl中显示
 */ 
void renderThread(){
    while (!quit)
    {
        getPictures();
        usleep(100);
    }
}


int main(int argc ,char* argv[] ){

    if(SDL_Init(SDL_INIT_EVERYTHING)) {    
        printf( "Could not initialize SDL - %s\n", SDL_GetError());   
        return -1;  
    }  

    SDL_Window *screen;   
    //SDL 2.0 Support for multiple windows  
    screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  
        screen_w, screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);  
    memset(&sdlRect, 0, sizeof(SDL_Rect));
    if(!screen) {    
        printf("SDL: could not create window - exiting:%s\n",SDL_GetError());    
        return -1;  
    }  
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);    
    Uint32 pixformat=0;  
    #if LOAD_BGRA  
    //Note: ARGB8888 in "Little Endian" system stores as B|G|R|A  
    pixformat= SDL_PIXELFORMAT_ARGB8888;    
    #elif LOAD_RGB24  
        pixformat= SDL_PIXELFORMAT_RGB888;    
    #elif LOAD_BGR24  
        pixformat= SDL_PIXELFORMAT_BGR888;    
    #elif LOAD_YUV422P  
        //IYUV: Y + U + V  (3 planes)  
        //YV12: Y + V + U  (3 planes)  
        pixformat= SDL_PIXELFORMAT_YUY2; // SDL_PIXELFORMAT_IYUV;    
    #endif  
        sdlTexture = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);  
        sdlRect.w = pixel_w;
        sdlRect.h = pixel_h;
        sdlRect.x = 0;
        sdlRect.y = 0;

    openCamera(onGetPictureBuffer, V4L2_PIX_FMT_YUYV);


    pthread_t tid; 
    int ret = pthread_create(&tid, NULL, renderThread, NULL);


    //sdl事件线程

    SDL_Event event;
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
        }
    }

    return 0;
}