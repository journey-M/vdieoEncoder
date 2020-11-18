#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <sys/mman.h>

#include "camera.h"


#define devName "/dev/video0"

void (*picCallback)(struct picbuffer *) = NULL;
int fd = -1;
struct picbuffer * mbuffer = NULL;

void openCamera(void (*fn)(struct picbuffer *), __uint32_t pixfmt){
    picCallback = fn;

    //1.first open file
    fd = open(devName, O_RDWR);
    struct v4l2_capability capability;

    //2. 读取视频能力参数
    int ret = ioctl(fd ,VIDIOC_QUERYCAP, &capability );
    if (ret < 0)
    {
        fprintf(stderr, "this is an error in  %d video capability\n",ret);
        return ;
    }

    //3.向设备设置视频的参数
    struct v4l2_format format;
    memset(&format, 0, sizeof(struct v4l2_format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 640;
    format.fmt.pix.height = 480;
    format.fmt.pix.pixelformat = pixfmt;
    format.fmt.pix.field = V4L2_FIELD_ANY;
    //保存图片格式的时候使用
	// fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;//V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_YVU420;//V4L2_PIX_FMT_YUYV;
	// fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    ret = ioctl(fd, VIDIOC_S_FMT, &format);

    //4.向驱动申请缓冲, 不超过5个
    struct v4l2_requestbuffers buffers;
    memset(&buffers, 0, sizeof(struct v4l2_requestbuffers));
    buffers.count = 5;
    buffers.memory = V4L2_MEMORY_MMAP;
    buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_REQBUFS, &buffers);
    if(ret < 0){
        fprintf(stderr, "request buffer error \n");
    }


    //5.映射内核中申请到的缓空间到用户的内存空间
    mbuffer = calloc(buffers.count, sizeof(*mbuffer));
    
    struct v4l2_buffer vbuf;
    for(int i=0; i< buffers.count;i++){
        memset(&vbuf, 0, sizeof(vbuf));
        vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &vbuf) == -1)
        {
            fprintf(stderr, "vidioc  -- querry  = -1 \n");
            return;
        }

        mbuffer[i].length = vbuf.length;
        //转换为相对地址
        mbuffer[i].start = mmap(NULL, vbuf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,vbuf.m.offset);

        if (mbuffer[i].start == MAP_FAILED)
        {
            fprintf(stderr, "map  error   \n");
            return;
        }
        
    }


    //5 申请到的缓冲进入队列
	for (int i = 0; i < buffers.count; ++i) 
	{
		struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(struct v4l2_buffer));

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))//申请到的缓冲进入列队
			printf ("VIDIOC_QBUF failed\n");
	}


    //6.开始采集
    int buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMON, &buf_type);    

    //7.取出FIFO队列中已经采样的帧缓存
    // for (size_t i = 0; i < buffers.count; i++)
    // {
	// 	fd_set fds;
	// 	struct timeval tv;
	// 	int r;

	// 	FD_ZERO (&fds);//将指定的文件描述符集清空
	// 	FD_SET (fd, &fds);//在文件描述符集合中增加一个新的文件描述符

	//    /* Timeout. */
	// 	tv.tv_sec = 2;
	// 	tv.tv_usec = 0;

	// 	r = select (fd + 1, &fds, NULL, NULL, &tv);//判断是否可读（即摄像头是否准备好），tv是定时

	// 	if (-1 == r) 
	// 	{
	// 		printf ("select err\n");
	// 	}
	// 	if (0 == r) V4L2_PIX_FMT_JPEG
	// 	{
	// 		fprintf (stderr, "select timeout\n");
	// 		exit (-1);
	// 	}
    // }


}



int getPictures(){

    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
    buf.memory = V4L2_MEMORY_MMAP;
    int ff = ioctl(fd, VIDIOC_DQBUF, &buf);
    if ( ff < 0)
    {
        /* code */
        fprintf(stderr, "error to  dequeue  video  \n");
        return -1;
    }

    //通过回调把视频信息传回去
    if (picCallback)
    {
        picCallback( &(mbuffer[buf.index]));
    }
    
    // FILE* file_fd = fopen("test.jpg", "wb"); 
    // //将图片写入 文件
    // fwrite(mbuffer[buf.index].start, mbuffer[buf.index].length, 1, file_fd); //将其写入文件中


    //重新进入队列
    ff = ioctl (fd, VIDIOC_QBUF, &buf); //再将其入列

    return 0;
}