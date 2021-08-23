#ifndef __PUSH_STREAM_MM__H_
#define __PUSH_STREAM_MM__H_

int initRtmp();

void send_video_sps_pps(char* sps,int sps_len,char* pps,int pps_len);

void send_rtmp_video(char *value,int len,long timestamp) ;

#endif
