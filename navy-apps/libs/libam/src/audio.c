#include <am.h>
#include <nemu.h>

#include "klib.h"
// #include "assert.h"
#include <riscv.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
 

static int location=0;

void __am_audio_init() {
  
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  // cfg->present = false;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  cfg->present = 1;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  // 将音频的参数传给硬件
  outl(AUDIO_FREQ_ADDR, ctrl->freq);  //将freq频率传给((0xa0000000 + 0x0000200) + 0x00)
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);//将channels声道数传给((0xa0000000 + 0x0000200) + 0x04)
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);//将samples采样率传给((0xa0000000 + 0x0000200) + 0x08)  刚好都差4字节
  outl(AUDIO_INIT_ADDR, 1);
  location = 0;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

//由NEMU完成SDL音频初始化, 然后软件一直向缓冲区写数据, SDL则会定时调用audio_callback读数据.
//软件向缓冲区写数据的话会导致count流缓冲区已经使用的大小逐渐增大
void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  uint32_t data_len = ctl->buf.end - ctl->buf.start;
  uint32_t sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  assert(data_len < sbuf_size);//要求ctrl的数据长度一定要小于流缓冲区sbuf的长度0x10000    否则就assert报错

  while (data_len > sbuf_size - inl(AUDIO_COUNT_ADDR)) {/*printf("数据长度大于所剩余流缓冲区的大小\n");*/}
  uint8_t *sbuf = (uint8_t *) AUDIO_SBUF_ADDR;//将AUDIO_SBUF_ADDR视为地址
  if (data_len + location < sbuf_size) {
    memcpy(sbuf + location, ctl->buf.start, data_len);
    location += data_len;
  } else {
    uint32_t temp_len = sbuf_size - location;
    memcpy(sbuf + location, ctl->buf.start, temp_len);
    uint32_t remain_len=  data_len-temp_len;
    memcpy(sbuf, ctl->buf.start + temp_len, remain_len);
    location = remain_len;
  }
  uint32_t count = inl(AUDIO_COUNT_ADDR);//读取count的值并且进行修改
  outl(AUDIO_COUNT_ADDR, count+data_len);//更新/增加缓冲区已经使用的大小
}



/*
来自于/usr/include/SDL/SDL_audio.h中的定义

typedef struct SDL_AudioSpec {
    int freq;//freq: 采样率，即每秒采样的次数，通常为22050Hz、44100Hz等。
    Uint16 format;//format: 采样格式，可以是8位无符号（AUDIO_U8）、16位有符号（AUDIO_S16）等。
    Uint8  channels;//channels: 声道数，可以是单声道（1）或立体声（2）。
    Uint8  silence;//silence: 静音值，用于表示静音的样本值。
    Uint16 samples;//samples: 音频缓冲区中每个片段的样本数量。
    Uint16 padding;//padding: 保留字节，用于结构体对齐。
    Uint32 size;//size: 音频缓冲区的大小（以字节为单位）。
    void (SDLCALL *callback)(void *userdata, Uint8 *stream, int len);//callback: 回调函数，当需要更多音频数据时由SDL调用。
    void  *userdata;//userdata: 用户可以传递给回调函数的任意数据。
} SDL_AudioSpec;

#define AUDIO_U8 0x0008     //AUDIO_U8, AUDIO_S8, AUDIO_U16LSB, AUDIO_S16LSB, AUDIO_U16MSB, AUDIO_S16MSB: 定义了不同的音频采样格式。
#define AUDIO_S8 0x8008       //AUDIO_U16SYS, AUDIO_S16SYS: 根据系统字节序定义16位音频格式。
#define AUDIO_U16LSB 0x0010
#define AUDIO_S16LSB 0x8010
#define AUDIO_U16MSB 0x1010
#define AUDIO_S16MSB 0x9010
#define AUDIO_U16 AUDIO_U16LSB
#define AUDIO_S16 AUDIO_S16LSB
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS AUDIO_U16LSB
#define AUDIO_S16SYS AUDIO_S16LSB
#else


typedef struct SDL_AudioCVT
{
    int needed;  //needed: 是否需要转换。
    Uint16 src_format;  //src_format: 源音频格式。
    Uint16 dst_format;  //dst_format: 目标音频格式。
    double rate_incr;   //rate_incr: 采样率转换系数。
    Uint8 *buf;   //buf: 转换缓冲区。
    int len;  //len: 源音频数据长度。
    int len_cvt;  //len_cvt: 转换后的音频数据长度。
    int len_mult;  //len_mult: 长度乘数。
    double len_ratio;  //len_ratio: 长度比例。
    void (SDLCALL *filters[10])(struct SDL_AudioCVT *cvt, Uint16 format);  //filters: 转换过滤器数组。
    int filter_index;  //filter_index: 当前使用的过滤器索引
} SDL_AudioCVT;

typedef enum SDL_audiostatus
{
    SDL_AUDIO_STOPPED,  //SDL_AUDIO_STOPPED: 音频已停止。
    SDL_AUDIO_PLAYING,  // SDL_AUDIO_PLAYING: 音频正在播放
    SDL_AUDIO_PAUSED   //SDL_AUDIO_PAUSED: 音频已暂停
} SDL_audiostatus;


#define SDL_MIX_MAXVOLUME 128

extern DECLSPEC int SDLCALL SDL_AudioInit(const char *driver_name);   初始化音频子系统。
extern DECLSPEC void SDLCALL SDL_AudioQuit(void);                    清理音频子系统。
extern DECLSPEC char * SDLCALL SDL_AudioDriverName(char *namebuf, int maxlen);获取当前音频驱动名称。
extern DECLSPEC int SDLCALL SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);打开音频设备。
extern DECLSPEC void SDLCALL SDL_PauseAudio(int pause_on);暂停或恢复音频播放。
extern DECLSPEC void SDLCALL SDL_CloseAudio(void);   关闭音频设备。
extern DECLSPEC SDL_audiostatus SDLCALL SDL_GetAudioStatus(void);  获取音频播放状态。
extern DECLSPEC void SDLCALL SDL_LockAudio(void);  锁定音频设备
extern DECLSPEC void SDLCALL SDL_UnlockAudio(void); 解锁音频设备。
extern DECLSPEC SDL_AudioSpec * SDLCALL SDL_LoadWAV_RW(SDL_RWops *src, int freesrc, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len); 从RWops加载WAV文件
extern DECLSPEC void SDLCALL SDL_FreeWAV(Uint8 *audio_buf);   释放WAV文件。
extern DECLSPEC int SDLCALL SDL_BuildAudioCVT(SDL_AudioCVT *cvt, Uint16 src_format, Uint8 src_channels, int src_rate, Uint16 dst_format, Uint8 dst_channels, int dst_rate);构建音频转换格式
extern DECLSPEC int SDLCALL SDL_ConvertAudio(SDL_AudioCVT *cvt);   执行音频转换。
extern DECLSPEC void SDLCALL SDL_MixAudio(Uint8 *dst, const Uint8 *src, Uint32 len, int volume);  混合音频

#define SDL_LoadWAV(file, spec, audio_buf, audio_len) SDL_LoadWAV_RW(SDL_RWFromFile(file, "rb"),1, spec,audio_buf,audio_len)


*/