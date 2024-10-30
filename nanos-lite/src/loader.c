#include <proc.h>
#include <elf.h>

#include <declaration.h>


#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  // TODO();
  // 你需要在Nanos-lite中实现loader的功能, 来把用户程序加载到正确的内存位置, 然后执行用户程序.
  // loader()函数在nanos-lite/src/loader.c中定义, 其中的pcb参数目前暂不使用, 可以忽略, 
  // 而因为ramdisk中目前只有一个文件, filename参数也可以忽略. 
  // 在下一个阶段实现文件系统之后, filename就派上用场了.

  //Ehdr 是 ELF Header 的缩写，它代表 ELF 文件的头部信息。ELF Header 包含了描述 ELF 文件如何被分析的信息，例如文件类型、机器类型、版本、入口点地址、程序头表和节头表的偏移量等重要信息。
  Elf_Ehdr elf_header;
  Elf_Ehdr* elf=&elf_header;
  ramdisk_read(elf, 0, sizeof(Elf_Ehdr));//读取elf的header
  assert(*(uint32_t *)elf->e_ident ==  0x7f454c46);//Linux/GNU 上的 ELF 文件的魔数是 0x7F 'E' 'L' 'F'，即十六进制的 7f 45 4c 46。  话说0x7f454c46是怎么摆放的？？？

  Elf_Phdr ProgramHeaders[elf->e_phnum];
  ramdisk_read(ProgramHeaders, elf->e_phoff, sizeof(Elf_Phdr)*elf->e_phnum);//读取程序头表（即段头表）一共要读取Elf_Phdr大小乘以程序头表个数
  for (int i = 0; i < elf->e_phnum; i++) {//遍历每一个程序头
    if (ProgramHeaders[i].p_type == PT_LOAD) {//如果第i个程序头的种类是Load的话
      ramdisk_read((void*)ProgramHeaders[i].p_vaddr, ProgramHeaders[i].p_offset, ProgramHeaders[i].p_memsz);//读取内存大小为p_memsz的程序头内容给对应地址为p_vaddr的段里面
      memset((void*)(ProgramHeaders[i].p_vaddr+ProgramHeaders[i].p_filesz), 0, ProgramHeaders[i].p_memsz - ProgramHeaders[i].p_filesz);      // set .bss with zeros将.bss的section初始化为0
    }
  }

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
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

