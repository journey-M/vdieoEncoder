#include <librtmp/rtmp.h>
#include <sys/socket.h>
#include <stdio.h>


int push(){
	RTMP * handler = RTMP_Alloc();
	RTMP_Init(handler);
	RTMP_SetupURL(handler,"rtmp://149.28.87.78:8001/cctvf/123");
	handler->Link.timeout = 10;
	handler->Link.lFlags |= RTMP_LF_LIVE	;

	RTMP_SetBufferMS(handler, 3600 * 1000);
	if(!	RTMP_Connect(handler,NULL)){
		fprintf(stderr, "erro in rtmp connect \n");
		return -1;
	}
	if( !RTMP_ConnectStream(handler, NULL)){
		fprintf(stderr, "erro in rtmp connect stream\n");
		return -1;
	}



}
