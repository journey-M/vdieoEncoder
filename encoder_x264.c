
#include <malloc.h>
#include <sys/types.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>

#include "x264.h"
#include "yuv_saver.h"
#include "encoder_x264.h"


FILE *fd_x264;

x264_t *handler;

int width = 640;
int height = 480;

u_int8_t * x264_buffer;
x264_param_t *params;
x264_picture_t *picture;

static void writ_file(void * data, ssize_t len){

    fwrite(data,len , 1,fd_x264);

}

void init_endcode(){

    picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));


    params = (x264_param_t *) malloc(sizeof(x264_param_t));
    x264_param_default(params);

    params->i_threads = X264_SYNC_LOOKAHEAD_AUTO;
    params->i_width = width;
    params->i_height = height;
    params->i_frame_total = 0;
    
    params->i_keyint_max = 10;
    params->rc.i_lookahead = 0;
    params->i_bframe = 5;

    params->b_open_gop = 0;
	params->i_bframe_pyramid = 0;
	params->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

	params->rc.i_bitrate = 1024 * 10;//rate 10 kbps
	params->i_fps_num = 25; 
	params->i_fps_den = 1;
	x264_param_apply_profile(params, x264_profile_names[0]); 


    if ((handler = x264_encoder_open(params)) == 0)
    {
        fprintf(stderr, "encoder init false  \n");
        return;
    }
    
    /**
     * create a new pic
     */
    x264_picture_alloc(picture, X264_CSP_I420, params->i_width, params->i_height);

    picture->img.i_csp = X264_CSP_I420;
    picture->img.i_plane = 3;

    x264_buffer = malloc(sizeof(u_int8_t) *width * height * 1.5);
    //初始化264保存文件
    fd_x264 = fopen("abc.264","wb");
}

void endcode_frame(void *yuvframe, size_t yunlength){
    
    x264_picture_t picture_out;
    x264_nal_t *nal;
    int nNal = -1;
    int result = 0;
    int i = 0;


    char* y = picture->img.plane[0];
    char* u = picture->img.plane[1];
    char* v = picture->img.plane[2];

    //yuv420_length = 1.5 * en->param->i_width * en->param->i_height
	//int length =  en->param->i_width * en->param->i_height;
	//y = 640*480 ; U = 640*480*0.25, V = 640*480*0.25;
    memcpy(y, yuvframe, 307200 );
    memcpy(u, yuvframe + 307200, 76800);
    memcpy(v, yuvframe + 384000, 76800);

    picture->i_type = X264_TYPE_IDR;

    int x264_len = x264_encoder_encode(handler, &nal, &nNal, picture, &picture_out);
    if (x264_len < 0)
    {
        fprintf(stderr, "encoder encode  error \n");
        return ;
    }
    

    void * p_out = x264_buffer;
    for (size_t i = 0; i < nNal; i++)
    {
        memcpy(p_out,nal[i].p_payload, nal[i].i_payload);
        p_out += nal[i].i_payload;
        result += nal[i].i_payload;
    }
    
    writ_file(x264_buffer, result);

}
