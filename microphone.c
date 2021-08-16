#include "microphone.h"

#include <alsa/asoundlib.h>


int openMicrophone(){

    snd_pcm_t *pcm_handle;

    /*设置音频流的方向为播放 */
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    snd_pcm_hw_params_t *hwparams;

    char *pcm_name;//定义设备名称
    pcm_name = strdup("default");


    //分配硬件结构体
    snd_pcm_hw_params_alloca(&hwparams);

    //打开设备录制
    if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0)
    {
        fprintf(stderr, " Eroor open PCM device  \n");
        return (-1);
    }

    int rate = 44100; /* Sample rate */
    int exact_rate;   /* Sample rate returned by */
                      /* snd_pcm_hw_params_set_rate_near */ 
    int dir;          /* exact_rate == rate --> dir = 0 */
                      /* exact_rate < rate  --> dir = -1 */
                      /* exact_rate > rate  --> dir = 1 */
    int periods = 2;       /* Number of periods */
    snd_pcm_uframes_t periodsize = 8192; /* Periodsize (bytes) */

    //设置参数

     if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
      fprintf(stderr, "Error setting access.\n");
      return(-1);
    }
  
    /* Set sample format */
    if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE) < 0) {
      fprintf(stderr, "Error setting format.\n");
      return(-1);
    }

    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */ 
    exact_rate = rate;
    if (snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_rate, 0) < 0) {
      fprintf(stderr, "Error setting rate.\n");
      return(-1);
    }
    if (rate != exact_rate) {
      fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n", rate, exact_rate);
    }

    /* Set number of channels */
    if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2) < 0) {
      fprintf(stderr, "Error setting channels.\n");
      return(-1);
    }

    /* Set number of periods. Periods used to be called fragments. */ 
    if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0) {
      fprintf(stderr, "Error setting periods.\n");
      return(-1);
    }
    

    // 接着我们要设置buffer的大小,buffer的大小是由函数的设置决定,有时参数是字节数,有时需要指定的是frames的数量,一帧的数据大小包括的是所有通道的数据大小,比如,立体声16-Bit 数据,它的一帧就是4Bytes. 
    /* Set buffer size (in frames). The resulting latency is given by */
        /* latency = periodsize * periods / (rate * bytes_per_frame)     */
    if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (periodsize * periods)>>2) < 0) {
        fprintf(stderr, "Error setting buffersize.\n");
        return(-1);
    }


    //当所有参数都配置完成后我们要让这些参数作用于PCM设备:
    /* Apply HW parameter settings to */
    /* PCM device and prepare device  */
    if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
      fprintf(stderr, "Error setting HW params.\n");
      return(-1);
    }


    //PCM设备配置之后,我们就可以播放数据了,但是这里还要注意的 是 对于interleaved write access,我们调用: 
    snd_pcm_sframes_t snd_pcm_writei(pcm_handle, data, num_frames);


}