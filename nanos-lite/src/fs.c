#include <fs.h>

#include <declaration.h>
//"文件记录表"其实是一个数组, 数组的每个元素都是一个结构体:
//在sfs中, 这三项信息都是固定不变的. 其中的文件名和我们平常使用的习惯不太一样: 由于sfs没有目录, 我们把目录分隔符/也认为是文件名的一部分, 例如/bin/hello是一个完整的文件名. 这种做法其实也隐含了目录的层次结构, 对于文件数量不多的情况, 这种做法既简单又奏效.
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);


//在sfs中, 这三项信息都是固定不变的. 其中的文件名和我们平常使用的习惯不太一样: 由于sfs没有目录, 我们把目录分隔符/也认为是文件名的一部分, 例如/bin/hello是一个完整的文件名. 这种做法其实也隐含了目录的层次结构, 对于文件数量不多的情况, 这种做法既简单又奏效
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;//另外, 我们也不希望每次读写操作都需要从头开始. 于是我们需要为每一个已经打开的文件引入偏移量属性open_offset, 来记录目前文件操作的位置. 每次对文件读写了多少个字节, 偏移量就前进多少.   
  //注意要在Finfo中添加一个open_offset字段来记录在某个文件当中的当前读写位置，并更新维护它，完事了之后在loader中使用即可。
} Finfo;//"文件记录表"其实是一个数组, 数组的每个元素都是一个结构体:
// 其中ReadFn和WriteFn分别是两种函数指针, 它们用于指向真正进行读写的函数, 并返回成功读写的字节数. 
// 有了这两个函数指针, 我们只需要在文件记录表中对不同的文件设置不同的读写函数, 就可以通过f->read()和f->write()的方式来调用具体的读写函数了.
// 不过在Nanos-lite中, 由于特殊文件的数量很少, 我们约定, 当上述的函数指针为NULL时, 表示相应文件是一个普通文件, 
// 通过ramdisk的API来进行文件的读写, 这样我们就不需要为大多数的普通文件显式指定ramdisk的读写函数了.


//为了方便用户程序进行标准输入输出, 操作系统准备了三个默认的文件描述符:
enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};
// 它们分别对应标准输入stdin, 标准输出stdout和标准错误stderr. 
// 我们经常使用的printf, 最终会调用write(FD_STDOUT, buf, len)进行输出; 
// 而scanf将会通过调用read(FD_STDIN, buf, len)进行读入.


// size_t am_ioe_read(void *buf, size_t offset, size_t len) {
//   printf("识别到am_ioe_read\n");
//   ioe_read(offset, buf);
//   return 0;
// }


// size_t am_ioe_write(const void *buf, size_t offset, size_t len) {
//   printf("识别到am_ioe_write\n");
//   ioe_write(offset, (void *)buf);
//   return 0;
// }



size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

//nanos-lite/src/fs.c中定义的file_table会包含nanos-lite/src/files.h,
//  其中前面还有3个特殊的文件: stdin, stdout和stderr的占位表项, 它们只是为了保证sfs和约定的标准输入输出的文件描述符保持一致, 
// 例如根据约定stdout的文件描述符是1, 而我们添加了三个占位表项之后, 文件记录表中的1号下标也就不会分配给其它的普通文件了.
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},//将FD_STDOUT和FD_STDERR设置为相应的write写入函数
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB]     = {"/dev/fb",0, 0, invalid_read, fb_write},// Nanos-lite和Navy约定, 把显存抽象成文件/dev/fb(fb为frame buffer之意), 它需要支持写操作和lseek, 以便于把像素更新到屏幕的指定位置上.
                //这里VGA一开始忘记要写系统调用了……
                {"/proc/dispinfo",0,0,dispinfo_read, invalid_write},
                {"/dev/events",0,0,events_read, invalid_write},//上述事件抽象成一个特殊文件/dev/events, 它需要支持读操作, 用户程序可以从中读出按键事件, 但它不必支持lseek, 因为它是一个字符设备.
                // {"/dev/am_ioe",128,0,am_ioe_read,am_ioe_write},//lut查找表正好128项  它们的本质都是读写文件，只要把ioe也当成文件就可以了。但ioe的设备寄存器有很多个，难道要为每个设备寄存器设置一个文件吗？其实有个方法，我们用文件指针来表示读取哪个寄存器。所有的ioe都抽象为同一个文件，用文件的open_offset区分具体指向哪一个ioe。用户程序打开这个文件后，将文件指针移动到reg位置上，再读写。操作系统处理读写时，就以open_offset判断读取哪个寄存器就可以了。
#include "files.h"  //nanos-lite/src/files.h包含进来表文件列表
};//在Nanos-lite中, 由于sfs的文件数目是固定的, 我们可以简单地把文件记录表的下标作为相应文件的文件描述符返回给用户程序. 在这以后, 所有文件操作都通过文件描述符来标识文件
//实际上, 操作系统中确实存在不少"没有名字"的文件. 为了统一管理它们, 我们希望通过一个编号来表示文件, 
// 这个编号就是文件描述符(file descriptor). 一个文件描述符对应一个正在打开的文件, 由操作系统来维护文件描述符到具体文件的映射



void init_fs() {
  // TODO: initialize the size of /dev/fb 初始化/dev/fb帧缓冲区大小
  AM_GPU_CONFIG_T gpu = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = gpu.width * gpu.height * sizeof(uint32_t);//每个像素用32位整数以`00RRGGBB`的方式描述颜色
  // printf("size:%u\n",file_table[FD_FB].size);
} 


//在文件读写操作中，各个库函数最终会调用read/write/open/close函数进行文件操作。
// 而上述几个函数又会分别调用_read/_write/_open/_close函数，
// 后者中又调用了_syscall_函数来编译出ecall指令并设置好系统调用的寄存器参数。

//陷入内核态后，经由do_syscall处理分发，识别出调用类型，再分别调用sys_read/sys_write/sys_open/sys_close，
// 在这里面最终调用fs_read/fs_write/fs_open/fs_close，也就是我们要分别实现的这四个函数
int fs_open(const char *pathname, int flags, int mode)
{//遍历寻找文件返回文件标识符
  // printf("NR_FILES值为:%d\n",NR_FILES);
  //除了写入stdout和stderr之外(用putch()输出到串口), 其余对于stdin, stdout和stderr这三个特殊文件的操作可以直接忽略(但是在这里如果要打开fd==0,1,2的话就直接panic报错了)
  for(int i=3;i<NR_FILES;i++)
  {if(strcmp(file_table[i].name, pathname)==0)
    {file_table[i].open_offset=0; return i;}//如果找到对应的文件名就返回文件标识符  并更新其offset
  }
  
  //由于sfs中每一个文件都是固定的, 不会产生新文件, 因此"fs_open()没有找到pathname所指示的文件"属于异常情况, 你需要使用assertion终止程序运行.
  // panic("路径为%s的文件没有找到\n",pathname);//这里我直接用panic了  因为我感觉panic和assert(0)没啥区别  emmm但是在busybox里面是不是会点问题？？？
  Log("路径为%s的文件没有找到\n",pathname);
  //注意如果这里panic没有注释掉的话，在execve实现时nterm里面如果遇到没有对应的路径的话会直接panic退出   这里暂时为了安全起见就暂时不注释吧
  return -1;
  //为了简化实现, 我们允许所有用户程序都可以对所有已存在的文件进行读写, 这样以后, 我们在实现fs_open()的时候就可以忽略flags和mode了.
}

size_t fs_read(int fd, void *buf, size_t len)
{
  //先验证文件描述符大于2（不是read、write或error） 
  // if(fd<3){Log("忽略本次读入标准输入输出文件%s ",file_table[fd].name); return 0;} //除了写入stdout和stderr之外(用putch()输出到串口), 其余对于stdin, stdout和stderr这三个特殊文件的操作可以直接忽略
  ReadFn readfun = file_table[fd].read;
  //##########################################################################################################
  if(readfun!=NULL)
  {if(readfun==events_read) return events_read(buf,0,len);
    else if(readfun==dispinfo_read) return dispinfo_read(buf,0,len);
    else return invalid_read(buf,0,len);}//我们约定, 当上述的函数指针为NULL时, 表示相应文件是一个普通文件}
  //##########################################################################################################
  //另外Nanos-lite也不打算支持stdin的读入, 因此在文件记录表中设置相应的报错函数即可.  也就是上面定义好的invalid_read和invalid_write
  size_t read_len=len;
  size_t size=file_table[fd].size;
  size_t open_offset=file_table[fd].open_offset;
  size_t disk_offset = file_table[fd].disk_offset;
  //由于文件的大小是固定的, 在实现fs_read(), fs_write()和fs_lseek()的时候, 注意偏移量不要越过文件的边界.
  if(open_offset>size){Log("文件%s当前文件指针越界，此次不可读",file_table[fd].name);return 0;}//注意如果目前文件读写位置超出了文件的大小的话就选择不读写
  if(open_offset + len > size){read_len = size - open_offset;/*Log("由于本次读取长度超出文件范围，实际输出长度为:%d",read_len)*/;}
  ramdisk_read(buf, disk_offset+open_offset, read_len);//使用ramdisk_read()和ramdisk_write()来进行文件的真正读写.
  file_table[fd].open_offset+=read_len;
  return read_len;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
  // if(fd==FD_STDIN){Log("忽略此次写入标准输入文件%s", file_table[fd].name);}
  //除了写入stdout和stderr之外(用putch()输出到串口), 其余对于stdin, stdout和stderr这三个特殊文件的操作可以直接忽略
  // if(fd==FD_STDOUT||fd==FD_STDERR){for(size_t i=0;i<len;i++){putch(*( (char*)buf+i) );}return len;} //如果是标准输出或者标准erroe的话输出buf的缓冲区即可
  WriteFn writefun=file_table[fd].write;
  //#############################################################################################################
  // if(writefun!=NULL)return writefun(buf,0,len);
  if(writefun!=NULL){ if(writefun==serial_write) return serial_write(buf,0,len);
    else if (writefun==fb_write) return fb_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
    //啊啊啊啊啊这个地方一开始卡了好久  因为默认非NULL的时候offset为0……无语死了……
    else return invalid_write(buf,0,len);}
  //################################################################################################################
  size_t write_len=len;//接下来的过程和fs_read几乎一模一样了
  size_t size=file_table[fd].size;
  size_t open_offset=file_table[fd].open_offset;
  size_t disk_offset = file_table[fd].disk_offset;
  if(open_offset>size){Log("文件%s当前文件指针越界，此次不可读",file_table[fd].name);return 0;}
  if(open_offset + len > size){write_len = size - open_offset; Log("由于本次读取长度超出文件范围，实际写入长度为:%d",write_len);}
  ramdisk_write(buf, disk_offset + open_offset, write_len);
  file_table[fd].open_offset+=write_len;
  return write_len;
}
size_t fs_lseek(int fd, size_t offset, int whence)
{
  //根据传入的文件号 fd 从文件记录表 file_table 中找到相应的文件记录项，即 Finfo 结构
  // "whence"参数通常在与文件操作相关的编程语言中使用，特别是在打开文件时。它是 os.open 或类似函数的一个参数，用于指定文件指针的起始位置。
  // 这个参数的全称是 “where to start from”，它指示从哪个位置开始读取或写入文件。
  // 获取当前文件的读写指针位置 file->open_offset 和当前文件的大小 file->size。读写指针记录了文件中正在读写的位置，文件大小表示文件的总字节数
  // 根据 whence 参数的不同含义，计算出新的读写指针位置 new_offset。whence 参数表示定位方式，可能的值有：
  // SEEK_SET：将读写指针设置为 offset，即距离文件开头 offset 字节处。
  // SEEK_CUR：将读写指针设置为当前位置加上 offset 字节，offset 可以为正数或负数，分别表示向文件尾或文件头移动。
  // SEEK_END：将读写指针设置为文件末尾再加上 offset 字节，同样 offset 可以为正数或负数。
  // 在计算出新的读写指针位置 new_offset 后，需要确保它在文件范围内，不能小于 0（文件开头）或大于当前文件的大小（文件末尾）。这是为了避免越界访问文件内容，保证读写操作的合法性。
  // 如果新的读写指针位置 new_offset 超出文件范围，需要将它设置为合法的边界位置。例如，如果 new_offset 小于 0，它将被设置为 0（文件开头），如果 new_offset 大于文件大小，它将被设置为文件末尾。
  // 最后，将新的读写指针位置 new_offset 存入文件记录表 file_table 中当前文件对应的项的读写指针字段 file->open_offset 中。
  // 返回值是一个表示新的读写指针位置的无符号整数。根据 man 2 lseek，返回值通常是新的文件读写指针位置（距离文件开头的偏移量）。这是 fs_lseek 函数的返回值，供后续文件读写操作使用。
  if(fd<3){Log("忽略本次lseek标准输入输出文件%s",file_table[fd].name);return 0;} //除了写入stdout和stderr之外(用putch()输出到串口), 其余对于stdin, stdout和stderr这三个特殊文件的操作可以直接忽略
  Finfo *file=&file_table[fd];
  size_t new_open_offset;
  //接下来根据whence参数计算新的open_offset偏移量并重新赋值和返回
  switch (whence)
  {
  case SEEK_SET: new_open_offset=offset;break;
  case SEEK_CUR: new_open_offset=file->open_offset+offset;break;
  case SEEK_END: new_open_offset=file->size+offset;break; //file->size也就是文件末尾
  default: Log("whence参数不合法,参数值为:%d",whence); return -1; break;
  }
  if(new_open_offset<0||new_open_offset > file->size){Log("lseek指针寻址超出范围");return -1;}//检查新的指针位置是否在文件范围之内
  file->open_offset=new_open_offset;//更新文件读写指针偏移量
  // if(fd==FD_FB)printf("在FD_FB当中new_open_offset:%u\n",new_open_offset);//这里是debug调试的内容
  return new_open_offset;
}
int fs_close(int fd)
{
  if(fd<3){Log("忽略本次关闭文件%s",file_table[fd].name);} //除了写入stdout和stderr之外(用putch()输出到串口), 其余对于stdin, stdout和stderr这三个特殊文件的操作可以直接忽略
  return 0;
}













