/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>  //注意这里已经提示我们因为引入了SDL库 因此在硬件层面就可以实现了

#include <stdio.h>
#include <string.h>

enum {
  reg_freq,  //freq, channels和samples这三个寄存器可写入相应的初始化参数
  reg_channels,
  reg_samples,
  reg_sbuf_size,//sbuf_size寄存器可读出流缓冲区的大小
  reg_init,//init寄存器用于初始化, 写入后将根据设置好的freq, channels和samples来对SDL的音频子系统进行初始化
  reg_count,//count寄存器可以读出当前流缓冲区已经使用的大小
  nr_reg
};

//NEMU的简单声卡在初始化时会分别注册0x200处长度为24个字节的端口, 以及0xa0000200处长度为24字节的MMIO空间, 
// 它们都会映射到上述寄存器; 此外还注册了从0xa1200000开始, 长度为64KB的MMIO空间作为流缓冲区.

static uint8_t *sbuf = NULL;  //流缓冲区STREAM_BUF是一段MMIO空间, 用于存放来自程序的音频数据, 这些音频数据会在将来写入到SDL库中
static uint32_t *audio_base = NULL;  //audio_base作为声卡MMIO内存空间的起始地址

static void nemu_audio_callback(void *userdata, Uint8 *stream, int len);
static void nemu_audio_init();

//声卡的回调函数
static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  static int have_audio_init=false;//如果还没有初始化的话先初始化
  if (audio_base[reg_init] && !have_audio_init && is_write) {have_audio_init = true; nemu_audio_init();}
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;//此外还注册了从0xa1200000开始, 长度为64KB的MMIO空间作为流缓冲区.
  audio_base = (uint32_t *)new_space(space_size); //获得空间之后初始化声卡的起始地址
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}


//一个真实的声卡设备非常复杂, 在NEMU中, 我们根据SDL库的API来设计一个简单的声卡设备. 使用SDL库来播放音频的过程非常简单:
// 通过SDL_OpenAudio()来初始化音频子系统, 需要提供频率, 格式等参数, 还需要注册一个用于将来填充音频数据的回调函数 
// 更多的信息请阅读man SDL_OpenAudio(需要安装libsdl2-doc)或者这个页面.
// SDL库会定期调用初始化时注册的回调函数, 并提供一个缓冲区, 请求回调函数往缓冲区中写入音频数据
// 回调函数返回后, SDL库就会按照初始化时提供的参数来播放缓冲区中的音频数据
// 声卡不能独立播放音频, 它需要接受来自客户程序的设置和音频数据. 程序要和设备交互, 自然是要通过I/O方式了, 
// 因此我们需要定义一些寄存器和 MMIO空间来让程序访问(见nemu/src/device/audio.c).



static void nemu_audio_init()
{
  /*
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
  */
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用16位有符号数来表示  
  s.userdata = NULL;        // 不使用
  //从abstract-machine/am/src/platform/nemu/ioe/audio.c中的__am_audio_ctrl可以得知刚好每个寄存器之间就差4个字节  而audio_base刚好又是uint32_t的
  s.freq = audio_base[reg_freq]; // 频率
  s.channels = audio_base[reg_channels]; // 单声道/双声道
  s.samples = audio_base[reg_samples]; // 采样率
  s.callback = nemu_audio_callback ; // 播放回调函数    从/usr/include/SDL/SDL_audio.h中的定义可得void (SDLCALL *callback)(void *userdata, Uint8 *stream, int len);
  //这里有一个名为callback的参数, 即SDL用于定期读取音频数据的函数, 由我们定义. 其函数原型为void audio_callback(void *userdata, uint8_t *stream, int len). 若存在新的音频数据, 就将其写入stream内.
  audio_base[reg_count] = 0;//初始化的时候已经用的count为0，因为流缓冲区sbuf还没有被使用
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_OpenAudio(&s, NULL);
  SDL_PauseAudio(0);
}


//由NEMU完成SDL音频初始化, 然后软件一直向缓冲区写数据, SDL则会定时调用audio_callback读数据.
//SDL播放音乐调用audio_callback读数据  这样的话count相当于被释放出了一部分  就减少了
static int location=0;
static void nemu_audio_callback(void *userdata, Uint8 *stream, int len)
{
  // 维护流缓冲区. 我们可以把流缓冲区可以看成是一个队列, 程序通过AM_AUDIO_PLAY的抽象往流缓冲区里面写入音频数据, 
  // 而SDL库的回调函数则从流缓冲区里面读出音频数据. 所以维护流缓冲区其实是一个数据结构的作业, 不过这个作业的特殊之处在于, 
  // 队列的读写双方分别位于两个不同的项目(硬件和软件), 它们之间只能通过I/O操作来进行交互. 
  // 此外, 如果回调函数需要的数据量大于当前流缓冲区中的数据量, 你还需要把SDL提供的缓冲区剩余的部分清零, 
  // 以避免把一些垃圾数据当做音频, 从而产生噪音.
  uint32_t stream_len = audio_base[reg_count];
  uint32_t real_len = stream_len < len ? stream_len : len;  //取有效长度
  SDL_LockAudio();//这个是干嘛的？？？？？？？？？？？？？？？？？？？  难道这一段为了锁住不让其他的按键产生影响？？？
  //考虑是否超过sbuf的最大范围CONFIG_SB_SIZE
  if(real_len + location < CONFIG_SB_SIZE) { //如果没有超过最大的范围那就正常copy
    memcpy(stream, sbuf + location, real_len);
    location += real_len;//更新已经到达的新的位置
  } else{//如果超出范围的话那就使用iringbuf的方式copy
    uint32_t temp_len1=CONFIG_SB_SIZE - location;
    memcpy(stream, sbuf + location, temp_len1 );
    uint32_t remain_len= real_len - temp_len1;
    memcpy(stream + temp_len1, sbuf, remain_len);
    location = remain_len;//更新在streambuffer当中location的位置
  }
  if (real_len < len) { memset(stream + real_len, 0, len - real_len); }// 长度不足, 清除后续数据, 避免杂音  注意这里清零的是stream而不是sbuf
  SDL_UnlockAudio();//解锁
  audio_base[reg_count]-=real_len;// 更新有效数据长度
}