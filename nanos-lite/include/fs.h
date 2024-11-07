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
//这组扩展语义之后的API有一个酷炫的名字, 叫VFS(虚拟文件系统). 既然有虚拟文件系统, 那相应地也应该有"真实文件系统", 这里所谓的真实文件系统, 其实是指具体如何操作某一类文件. 比如在Nanos-lite上, 普通文件通过ramdisk的API进行操作; 在真实的操作系统上, 真实文件系统的种类更是数不胜数: 比如熟悉Windows的你应该知道管理普通文件的NTFS, 目前在GNU/Linux上比较流行的则是EXT4; 至于特殊文件的种类就更多了, 于是相应地有procfs, tmpfs, devfs, sysfs, initramfs... 这些不同的真实文件系统, 它们都分别实现了这些文件的具体操作方式.
//所以, VFS其实是对不同种类的真实文件系统的抽象, 它用一组API来描述了这些真实文件系统的抽象行为, 屏蔽了真实文件系统之间的差异, 上层模块(比如系统调用处理函数)不必关心当前操作的文件具体是什么类型, 只要调用这一组API即可完成相应的文件操作. 有了VFS的概念, 要添加一个真实文件系统就非常容易了: 只要把真实文件系统的访问方式包装成VFS的API, 上层模块无需修改任何代码, 就能支持一个新的真实文件系统了.

#endif
