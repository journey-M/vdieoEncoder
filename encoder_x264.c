
#include <malloc.h>
#include <sys/types.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <time.h>

#include <x264.h>
#include "yuv_saver.h"
#include "encoder_x264.h"


FILE *fd_x264;

x264_t *handler;
int sps_len;
int pps_len;
unsigned char sps[30];
unsigned char pps[10];
int first = 0;

int width = 640;
int height = 480;

u_int8_t * x264_buffer;
int lastBufferSize = 0;
x264_param_t *params;
x264_picture_t *picture;

static void writ_x264_encoded(void * data, ssize_t len){

  fwrite(data,len , 1,fd_x264);

}

int get_encoded_data(unsigned char **destPtr, int *size){
  *size = lastBufferSize;
  *destPtr = x264_buffer;
  return 1;
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
  fd_x264 = fopen("abc.h264","wb");
}

void endcode_frame(int pts, void *yuyvframe,int size,  int width , int height){
  unsigned char* yuvframe = malloc (width * height * 1.5);

  yuyv_to_i420(yuyvframe ,yuvframe, width, height);

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
  picture->i_pts = pts;

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

    if (nal[i].i_type == NAL_SPS) { //this NAL type is SPS
      sps_len = nal[i].i_payload - 4;
      memcpy(sps, nal[i].p_payload+ 4, sps_len);
      fprintf(stderr, "NAL_SPS  %s \n", sps); 
    }else if(nal[i].i_type == NAL_PPS){
      pps_len = nal[i].i_payload - 4;
      memcpy(pps, nal[i].p_payload + 4, pps_len);
      if (first == 0) {
        send_video_sps_pps(sps, sps_len, pps, pps_len);
        first = 1;
      } 
    }else{
      struct timeval tv;
      gettimeofday(&tv, NULL);
      long timestamp = tv.tv_sec * 1000000 + tv.tv_usec;
      send_rtmp_video(nal[i].p_payload,x264_len - result,timestamp) ;

    }
    p_out += nal[i].i_payload;
    result += nal[i].i_payload;

  }

  lastBufferSize = result;
  writ_x264_encoded(x264_buffer, result);

  free(yuvframe);
}

void yuyv_to_i420(const unsigned char *in, unsigned char *out, unsigned int width ,unsigned int height){
  unsigned char *y = out;
  unsigned char *u = out + width*height;
  unsigned char *v = out + width*height + width*height/4;

  unsigned int i,j;
  unsigned int base_h;
  unsigned int is_y = 1, is_u = 1;
  unsigned int y_index = 0, u_index = 0, v_index = 0;

  unsigned long yuv422_length = 2 * width * height;

  //序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
  //丢弃偶数行 u v
  for(i=0; i<yuv422_length; i+=2)
  {
    *(y+y_index) = *(in+i);
    y_index++;
  }

  for(i=0; i<height; i+=2)
  {
    base_h = i*width*2;
    for(j=base_h+1; j<base_h+width*2; j+=2)
    {
      if(is_u)
      {
        *(u+u_index) = *(in+j);
        u_index++;
        is_u = 0;
      }
      else
      {
        *(v+v_index) = *(in+j);
        v_index++;
        is_u = 1;
      }
    }
  }
}
