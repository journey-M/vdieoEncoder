#include <sys/socket.h>
#include <stdio.h>
#include <librtmp/rtmp.h>
#include <memory.h>
#include <x264.h>
#include "push_stream.h"

#define RTMP_HEAD_SIZE (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

static char* URL = "rtmp://45.32.29.118:1935/stream/123";
static RTMP * handler = NULL;
int StartTime;

int initRtmp(){

  handler = RTMP_Alloc();
  RTMP_Init(handler);
  RTMP_SetupURL(handler,URL);
  handler->Link.timeout = 10;
  handler->Link.lFlags |= RTMP_LF_LIVE	;

  RTMP_SetBufferMS(handler, 3600 * 1000);
  RTMP_EnableWrite(handler);
  if(!	RTMP_Connect(handler,NULL)){
    fprintf(stderr, "erro in rtmp connect \n");
    return -1;
  }
  if( !RTMP_ConnectStream(handler, NULL)){
    fprintf(stderr, "erro in rtmp connect stream\n");
    return -1;
  }


  return 0;
}

void send_video_sps_pps(char* sps,int sps_len,char* pps,int pps_len){
  RTMPPacket * packet;
	unsigned char * body;
	int i;
	if (handler != NULL) {
		packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + 1024);
		memset(packet, 0, RTMP_HEAD_SIZE);

		packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
		body = (unsigned char *) packet->m_body;
		i = 0;
		body[i++] = 0x17;
		body[i++] = 0x00;

		body[i++] = 0x00;
		body[i++] = 0x00;
		body[i++] = 0x00;

		/*AVCDecoderConfigurationRecord*/
		body[i++] = 0x01;
		body[i++] = sps[1];
		body[i++] = sps[2];
		body[i++] = sps[3];
		body[i++] = 0xff;

		/*sps*/
		body[i++] = 0xe1;
		body[i++] = (sps_len >> 8) & 0xff;
		body[i++] = sps_len & 0xff;
		memcpy(&body[i], sps, sps_len);

		i += sps_len;

		/*pps*/
		body[i++] = 0x01;
		body[i++] = (pps_len >> 8) & 0xff;
		body[i++] = (pps_len) & 0xff;
		memcpy(&body[i], pps, pps_len);
		i += pps_len;

		packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
		packet->m_nBodySize = i;
		packet->m_nChannel = 0x04;

		packet->m_nTimeStamp = 0;
		packet->m_hasAbsTimestamp = 0;
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
		packet->m_nInfoField2 = handler->m_stream_id;

		if (RTMP_IsConnected(handler)) {
			//调用发送接口
			int success = RTMP_SendPacket(handler, packet, TRUE);
			if (success != 1) {
				fprintf(stderr ,"send_video_sps_pps fail");
			}
		} else {
			fprintf(stderr ,"send_video_sps_pps RTMP is not ready");
		}
		free(packet);
	}

}

void send_rtmp_video(char *data,int data_len,long timestamp){
  int type;
	RTMPPacket * packet;
	unsigned char * body;
	unsigned char* buffer = data;
	uint32_t length = data_len;

	if (handler != NULL) {
		timestamp = timestamp - StartTime;
		/*去掉帧界定符(这里可能2种,但是sps or  pps只能为 00 00 00 01)*/
		if (buffer[2] == 0x00) { /*00 00 00 01*/
			buffer += 4;
			length -= 4;
		} else if (buffer[2] == 0x01) { /*00 00 01*/
			buffer += 3;
			length -= 3;
		}
		type = buffer[0] & 0x1f;

		packet = (RTMPPacket *) malloc(RTMP_HEAD_SIZE + length + 9);
		memset(packet, 0, RTMP_HEAD_SIZE);

		packet->m_body = (char *) packet + RTMP_HEAD_SIZE;
		packet->m_nBodySize = length + 9;

		/*send video packet*/
		body = (unsigned char *) packet->m_body;
		memset(body, 0, length + 9);

		/*key frame*/
		body[0] = 0x27;
		if (type == NAL_SLICE_IDR) //此为关键帧
				{
			body[0] = 0x17;
		}

		body[1] = 0x01; /*nal unit*/
		body[2] = 0x00;
		body[3] = 0x00;
		body[4] = 0x00;

		body[5] = (length >> 24) & 0xff;
		body[6] = (length >> 16) & 0xff;
		body[7] = (length >> 8) & 0xff;
		body[8] = (length) & 0xff;

		/*copy data*/
		memcpy(&body[9], buffer, length);

		packet->m_nTimeStamp = timestamp;
		packet->m_hasAbsTimestamp = 0;
		packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
		packet->m_nInfoField2 = handler->m_stream_id;
		packet->m_nChannel = 0x04;
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

		if (RTMP_IsConnected(handler)) {
			// 调用发送接口

			int success = RTMP_SendPacket(handler, packet, TRUE);
			if (success != 1) {
				fprintf(stderr, "send_rtmp_video fail");
			} else {
//			LOGE("發送成功");
			}
		} else {
//		rtmp_open_for_write();
			fprintf(stderr,"send_rtmp_video RTMP is not ready");
		}
		free(packet);
	}

}



