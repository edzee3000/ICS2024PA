#include <proc.h>
#include <elf.h>

#include <declaration.h>

#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

// 你很有可能会因为疏忽, 从而让native的Nanos-lite来加载运行一个x86/mips32/riscv32的dummy. 
// 从ISA规范的角度来说, 这种行为显然属于UB, 具体而言通常会发生一些难以理解的错误. 
// 为了避免这种情况, 可以在loader中检测ELF文件的ISA类型.
// 在abstract-machine/am/src/platform/nemu/include/nemu.h当中查看AM中定义的宏
#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
# define EXPECT_TYPE EM_386
// see /usr/include/elf.h to get the right type
#elif defined(__ISA_MIPS32__) 
# define EXPECT_TYPE EM_MIPS_X   
//#define EM_MIPS_RS3_LE  10      /* MIPS R3000 little-endian */
//#define EM_MIPS_X       51      /* Stanford MIPS-X */
#elif defined(__ISA_LOONGARCH32R__)
# define EXPECT_TYPE 
//竟然找不到…… 
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  // TODO();
  // 你需要在Nanos-lite中实现loader的功能, 来把用户程序加载到正确的内存位置, 然后执行用户程序.
  // loader()函数在nanos-lite/src/loader.c中定义, 其中的pcb参数目前暂不使用, 可以忽略, 
  // 而因为ramdisk中目前只有一个文件, filename参数也可以忽略. 
  // 在下一个阶段实现文件系统之后, filename就派上用场了.
  
  // 你需要先实现fs_open(), fs_read()和fs_close(), 这样就可以在loader中使用文件名来指定加载的程序了, 例如"/bin/hello".
  // 实现之后, 以后更换用户程序只需要修改传入naive_uload()函数的文件名即可.
  int fd=fs_open(filename,0,0);//不考虑各种mode等等
  if(fd<0){panic("should not reach here");}


  //Ehdr 是 ELF Header 的缩写，它代表 ELF 文件的头部信息。ELF Header 包含了描述 ELF 文件如何被分析的信息，例如文件类型、机器类型、版本、入口点地址、程序头表和节头表的偏移量等重要信息。
  Elf_Ehdr elf_header;
  Elf_Ehdr* elf=&elf_header;
  // ramdisk_read(elf, 0, sizeof(Elf_Ehdr));//读取elf的header
  int read_elf_len=fs_read(fd,elf,sizeof(elf_header));
  assert(read_elf_len==sizeof(elf_header));
  // assert(*(uint32_t *)elf->e_ident ==  0x7f454c46);//Linux/GNU 上的 ELF 文件的魔数是 0x7F 'E' 'L' 'F'，即十六进制的 7f 45 4c 46。  话说0x7f454c46是怎么摆放的？？？
  assert(*(uint32_t *)elf->e_ident ==  0x464c457f); //好吧我承认原来是因为小端模式  e_ident里面有16个字节的内容  读数据的时候是从小到大摆放  强制类型转换为uint32_t *之后一次读取字节为4个字节但是解引用之后高位确实为0x46 低位确实为0x7f
  // 检测ELF文件的ISA类型 避免让native的Nanos-lite来加载运行一个dummy
  if(elf->e_machine!=EXPECT_TYPE) panic("您可能因为疏忽, 让native的Nanos-lite来加载运行一个x86/mips32/riscv32的dummy.");

  // Elf_Phdr ProgramHeaders[elf->e_phnum];
  Elf_Phdr programheader;//
  // ramdisk_read(ProgramHeaders, elf->e_phoff, sizeof(Elf_Phdr)*elf->e_phnum);//读取程序头表（即段头表）一共要读取Elf_Phdr大小乘以程序头表个数
  for (int i = 0; i < elf->e_phnum; i++) {//遍历每一个程序头
    uint32_t base = elf->e_phoff + i * elf->e_phentsize;  //计算出程序头的起始位置（基址）
    fs_lseek(fd, base, 0);  //根据base基址去更新文件指针的位置
    assert(fs_read(fd, &programheader, elf->e_phentsize) == elf->e_phentsize);
    // if (ProgramHeaders[i].p_type == PT_LOAD) {//如果第i个程序头的种类是Load的话
    //   ramdisk_read((void*)ProgramHeaders[i].p_vaddr, ProgramHeaders[i].p_offset, ProgramHeaders[i].p_memsz);//读取内存大小为p_memsz的程序头内容给对应地址为p_vaddr的段里面
    //   memset((void*)(ProgramHeaders[i].p_vaddr+ProgramHeaders[i].p_filesz), 0, ProgramHeaders[i].p_memsz - ProgramHeaders[i].p_filesz);      // set .bss with zeros将.bss的section初始化为0
    // }
     // 需要装载的段
    if (programheader.p_type == PT_LOAD) {
      char * buf_malloc = (char *)malloc(programheader.p_filesz);
      fs_lseek(fd, programheader.p_offset, 0);
      assert(fs_read(fd, buf_malloc, programheader.p_filesz) == programheader.p_filesz);
      memcpy((void*)programheader.p_vaddr, buf_malloc, programheader.p_filesz);
      memset((void*)programheader.p_vaddr + programheader.p_filesz, 0, programheader.p_memsz - programheader.p_filesz);
      free(buf_malloc);
    }
    //我们现在关心的是如何加载程序, 因此我们重点关注segment的视角. ELF中采用program header table来管理segment,
    //  program header table的一个表项描述了一个segment的所有属性, 包括类型, 虚拟地址, 标志, 对齐方式, 以及文件内偏移量和segment大小. 
    // 根据这些信息, 我们就可以知道需要加载可执行文件的哪些字节了, 同时我们也可以看到, 加载一个可执行文件并不是加载它所包含的所有内容, 
    // 只要加载那些与运行时刻相关的内容就可以了, 例如调试信息和符号表就不必加载. 
    // 我们可以通过判断segment的Type属性是否为PT_LOAD来判断一个segment是否需要加载.
  }
  printf("ok\n");
  assert(fs_close(fd) == 0);//关闭文件
  return elf->e_entry;//返回程序的入口地址
  // 在 ELF 文件中，e_entry 是一个非常重要的字段，它表示程序的入口地址。
  // 具体来说，e_entry 是一个虚拟地址，指向程序开始执行时的第一条指令的位置。
  // 对于可执行文件来说，这个地址通常指向程序的启动函数，
  // 比如 _start；对于动态链接库（共享对象文件），它可能指向一个初始化函数或者为空。
  // 如果 e_entry 的值为0，这通常意味着该 ELF 文件没有关联的入口点
}

void naive_uload(PCB *pcb, const char *filename) {
  // 实现后, 在init_proc()中调用naive_uload(NULL, NULL), 它会调用你实现的loader来加载第一个用户程序, 
  // 然后跳转到用户程序中执行. 如果你的实现正确, 你会看到执行dummy程序时在Nanos-lite中触发了一个未处理的4号事件. 
  // 这说明loader已经成功加载dummy, 并且成功地跳转到dummy中执行了
  printf("filename:%s\n",filename);
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();//跳转到程序的入口地址  开始执行程序
}

