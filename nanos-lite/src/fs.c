#include <fs.h>

#include <declaration.h>
//"文件记录表"其实是一个数组, 数组的每个元素都是一个结构体:
//在sfs中, 这三项信息都是固定不变的. 其中的文件名和我们平常使用的习惯不太一样: 由于sfs没有目录, 我们把目录分隔符/也认为是文件名的一部分, 例如/bin/hello是一个完整的文件名. 这种做法其实也隐含了目录的层次结构, 对于文件数量不多的情况, 这种做法既简单又奏效.
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;   //注意要在Finfo中添加一个open_offset字段来记录在某个文件当中的当前读写位置，并更新维护它，完事了之后在loader中使用即可。
} Finfo;

//为了方便用户程序进行标准输入输出, 操作系统准备了三个默认的文件描述符:
enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};
//它们分别对应标准输入stdin, 标准输出stdout和标准错误stderr. 我们经常使用的printf, 最终会调用write(FD_STDOUT, buf, len)进行输出; 而scanf将会通过调用read(FD_STDIN, buf, len)进行读入.



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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"  //nanos-lite/src/files.h包含进来表文件列表
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}


//在文件读写操作中，各个库函数最终会调用read/write/open/close函数进行文件操作。
// 而上述几个函数又会分别调用_read/_write/_open/_close函数，
// 后者中又调用了_syscall_函数来编译出ecall指令并设置好系统调用的寄存器参数。

//陷入内核态后，经由do_syscall处理分发，识别出调用类型，再分别调用sys_read/sys_write/sys_open/sys_close，
// 在这里面最终调用fs_read/fs_write/fs_open/fs_close，也就是我们要分别实现的这四个函数
int fs_open(const char *pathname, int flags, int mode)
{//遍历寻找文件返回文件标识符
  // printf("NR_FILES值为:%d\n",NR_FILES);
  for(int i=3;i<NR_FILES;i++)
  {if(strcmp(file_table[i].name, pathname)==0)
    {file_table[i].open_offset=0; return i;}//如果找到对应的文件名就返回文件标识符  并更新其offset
    }
  panic("路径为%s的文件没有找到\n",pathname);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len)
{
  //先验证文件描述符大于2（不是read、write或error） 
  if(fd<3){Log("忽略阅读文件%s ",file_table[fd].name); return 0;}
  size_t read_len=len;
  size_t size=file_table[fd].size;
  size_t open_offset=file_table[fd].open_offset;
  size_t disk_offset = file_table[fd].disk_offset;
  if(open_offset>size){Log("文件%s当前文件指针越界，此次不可读",file_table[fd].name);return 0;}//注意如果目前文件读写位置超出了文件的大小的话就选择不读写
  if(open_offset + len > size){read_len = size - open_offset;Log("由于本次读取长度超出文件范围，实际输出长度为:%d",read_len);}
  ramdisk_read(buf, disk_offset+open_offset, read_len);
  file_table[fd].open_offset+=read_len;
  return read_len;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
  if(fd==0){Log("忽略此次标准输入%s", file_table[fd].name);}
  if(fd==1||fd==2){for(size_t i=0;i<len;i++){putch(*( (char*)buf+i) );}return len;} //如果是标准输出或者标准erroe的话输出buf的缓冲区即可
  size_t write_len=len;//接下来的过程和fs_read几乎一模一样了
  size_t size=file_table[fd].size;
  size_t open_offset=file_table[fd].open_offset;
  size_t disk_offset = file_table[fd].disk_offset;
  if(open_offset>size){Log("文件%s当前文件指针越界，此次不可读",file_table[fd].name);return 0;}
  if(open_offset + len > size){write_len = size - open_offset; Log("由于本次读取长度超出文件范围，实际写入长度为:%d",write_len);}
  ramdisk_write(buf, disk_offset+open_offset, write_len);
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
  if(fd<3){Log("忽略本次lseek文件%s",file_table[fd].name);return 0;}
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
  if(new_open_offset<0||new_open_offset>file->size){Log("lseek指针寻址超出范围");return -1;}//检查新的指针位置是否在文件范围之内
  file->open_offset=new_open_offset;//更新文件读写指针偏移量
  return new_open_offset;
}
int fs_close(int fd)
{
  return 0;
}