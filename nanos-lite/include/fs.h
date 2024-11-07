#ifndef __FS_H__
#define __FS_H__

#include <common.h>

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

// #define NR_FILES 23  //定义一共有23个文件  file.h当中有20个  in、out、err有3个
#define NR_FILES sizeof(file_table)/sizeof(Finfo)
//根据以上信息, 我们就可以在文件系统中实现以下的文件操作了:
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);


#endif
