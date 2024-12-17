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


Context *ucontext(AddrSpace *as, Area kstack, void *entry);
void draw_ustack(uintptr_t* us_top, uintptr_t* us_end, int argc, int envc ,char *const argv[], char *const envp[]);


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
  Elf_Phdr* programheader=(Elf_Phdr*)malloc(sizeof(Elf_Phdr));//
  // ramdisk_read(ProgramHeaders, elf->e_phoff, sizeof(Elf_Phdr)*elf->e_phnum);//读取程序头表（即段头表）一共要读取Elf_Phdr大小乘以程序头表个数
  for (int i = 0; i < elf->e_phnum; i++) {//遍历每一个程序头
    uint32_t base = elf->e_phoff + i * elf->e_phentsize;  //计算出程序头的起始位置（基址）
    fs_lseek(fd, base, 0);  //根据base基址去更新文件指针的位置
    assert(fs_read(fd, programheader, elf->e_phentsize) == elf->e_phentsize);
    // if (ProgramHeaders[i].p_type == PT_LOAD) {//如果第i个程序头的种类是Load的话
    //   ramdisk_read((void*)ProgramHeaders[i].p_vaddr, ProgramHeaders[i].p_offset, ProgramHeaders[i].p_memsz);//读取内存大小为p_memsz的程序头内容给对应地址为p_vaddr的段里面
    //   memset((void*)(ProgramHeaders[i].p_vaddr+ProgramHeaders[i].p_filesz), 0, ProgramHeaders[i].p_memsz - ProgramHeaders[i].p_filesz);      // set .bss with zeros将.bss的section初始化为0
    // }
     // 需要装载的段
    if (programheader->p_type == PT_LOAD) {
      char * buf_malloc = (char *)malloc(programheader->p_filesz);
      fs_lseek(fd, programheader->p_offset, 0);
      assert(fs_read(fd, buf_malloc, programheader->p_filesz) == programheader->p_filesz);
      memcpy((void*)programheader->p_vaddr, buf_malloc, programheader->p_filesz);
      memset((void*)programheader->p_vaddr + programheader->p_filesz, 0, programheader->p_memsz - programheader->p_filesz);
      free(buf_malloc);
    }
    //我们现在关心的是如何加载程序, 因此我们重点关注segment的视角. ELF中采用program header table来管理segment,
    //  program header table的一个表项描述了一个segment的所有属性, 包括类型, 虚拟地址, 标志, 对齐方式, 以及文件内偏移量和segment大小. 
    // 根据这些信息, 我们就可以知道需要加载可执行文件的哪些字节了, 同时我们也可以看到, 加载一个可执行文件并不是加载它所包含的所有内容, 
    // 只要加载那些与运行时刻相关的内容就可以了, 例如调试信息和符号表就不必加载. 
    // 我们可以通过判断segment的Type属性是否为PT_LOAD来判断一个segment是否需要加载.
  }
  // printf("filename:%s\tfd:%d\n",filename,fd);
  // printf("elf->e_entry:%x\n",elf->e_entry);


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
  // printf("filename:%s\n",filename);
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();//跳转到程序的入口地址  开始执行程序
}



/*
不过为了给用户进程传递参数, 你还需要修改context_uload()的原型:
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
这样你就可以在init_proc()中直接给出用户进程的参数来测试了: 在创建仙剑奇侠传用户进程的时候给出--skip参数, 
你需要观察到仙剑奇侠传确实跳过了商标动画. 目前我们的测试程序中不会用到环境变量, 所以不必传递真实的环境变量字符串. 
至于实参应该写什么, 这又是一个指针相关的问题, 就交给你来解决吧.
*/
// void context_uload(PCB *pcb, const char *filename)
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[])
{
  uintptr_t entry = loader(pcb, filename);
  //用户进程的上下文(mepc指针等)存储在PCB栈，而函数参数之类的数据存储在用户栈，PCB栈和用户栈是完全分开的，
  // 进程加载后只会把上下文放进PCB中，数据还是在自己的用户栈。这里要求要传参数给函数，
  // 就把这些数据放用户栈(heap)，然后在call_main中从用户栈中拿这些信息，之后调用main
  // 定义用户栈的区域
  // Area stack;stack.start = pcb->stack;stack.end = pcb->stack + STACK_SIZE;
  //计算对应的argc与argv的值
  int argc = 0; if(argv!=NULL){while (argv[argc] != NULL) argc++;}
  int envc = 0; if(envp!=NULL){while (envp[envc] != NULL) envc++;}
  // printf("envc的值为:%d\n",envc);
  // 分配用户栈空间，用于存储 argv 和 envp 指针
  // printf("heap.end-1值为:%x\n",(uintptr_t*)heap.end-1);
  // uintptr_t* user_stack = (uintptr_t*)heap.end;//注意这里的user_stack是在不断变化的向低地址处增长使得栈顶的位置不断增长
  // char* user_stack = (char*)heap.end;//注意这里是因为要存储string area因此是char*类型!!!!
  char* new_user_stack=(char*)new_page(8);  //把之前的heap改成调用new_page()  因为这里需要创建一个新的用户栈而不能影响原来的用户栈
  char* user_stack = new_user_stack;
  printf("user_stack位置为: %x\n",user_stack);
  // 将 argv 字符串逆序拷贝到用户栈  逆向压栈
  for (int i = 0; i < argc; i++) {size_t len = strlen(argv[i]) + 1;  // 包括 null 终止符也要copy进来   但是这里是不是有问题？？？？？？？？没问题 因为传进去的是指针
    user_stack -= len; strncpy((char*)user_stack, argv[i], len);}
  // 对齐到 uintptr_t 边界   ？？？？？？这行代码是什么意思？？？？                 会不会出现问题？？？？？？？？、
  // // user_stack = (uintptr_t*)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));
  // user_stack = (char*)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));
  // 将 envp 字符串逆序拷贝到用户栈
  for (int i = 0; i <envc; i++) {size_t len = strlen(envp[i]) + 1;  // 包括 null 终止符
    user_stack -= len; strncpy((char*)user_stack, envp[i], len);}
  // 对齐到 uintptr_t 边界   应该这个时候再对齐到uintptr_t边界  上面那个应该不用
  // user_stack = (uintptr_t*)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));
  user_stack = (char*)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));
  // 将 argv 和 envp 指针拷贝到用户栈
  // uintptr_t* us2 = (uintptr_t *)user_stack;
  uintptr_t* us1 = (uintptr_t *)user_stack;
  // user_stack -= (argc + envc + 4);  // +4 为 NULL 结尾和 argc/envc 的值
  us1 -= (argc + envc + 3);//此时user_stack的位置是在string area以及envp的NULL之间的!!!  +3是因为有2个NULL以及一个argc放置需要处理
  // uintptr_t* user_argv = user_stack;
  // 设置 argc 的值
  // user_stack[0] = argc;
  uintptr_t* us2=us1;
  *us1 = argc; us1++;
  printf("argc对应位置的值为:%d\targc位置为:%x\n",*(us1-1), us1-1);
  // 设置 argv 指针
  // user_stack = (char*)heap.end;//结果这里忘记改了  然后报了一点点小错误  但是问题不大改回来了
  user_stack=new_user_stack;
  for (int i = 0; i < argc; i++) {
    // user_stack[i + 1] = (uintptr_t)heap.end - (argc - i - 1) * sizeof(uintptr_t);
    user_stack-= (strlen(argv[i]) + 1);  *((char **)us1) =user_stack;  us1++;
    printf("argv[%d]内容为:%s\targv[%d]指针值为:%x\targv[%d]指针存放的位置为:%x\n",i,user_stack, i,user_stack, i, (us1-1));
  }
  // 设置 argv 的 NULL 终止符
  // us2[argc + 1] = 0; 
  *((char **)us1)=0; us1++;
  // // 设置 envc 的值
  // user_stack[argc + 2] = envc;
  // 设置 envp 指针
  for (int i = 0; i < envc; i++) {
    // user_stack[argc + 3 + i] = (uintptr_t)heap.end - (argc + 3 + envc - i - 1) * sizeof(uintptr_t);
    user_stack-= (strlen(envp[i]) + 1);  *((char **)us1) =user_stack;  us1++; 
    printf("envp[%d]内容为:%s\tenvp[%d]指针值为:%x\tenvp[%d]指针存放的位置为:%x\n",i,user_stack, i,user_stack, i, (us1-1));  
  }
  // 设置 envp 的 NULL 终止符
  // user_stack[argc + 3 + envc] = 0;
  *((char **)us1)=0;
  // 调用 ucontext 函数创建用户上下文，传入入口地址和用户栈
  // pcb->cp = ucontext(&pcb->as, stack, (void*)entry);
  pcb->cp=ucontext(&pcb->as,  (Area){pcb->stack, pcb->stack+STACK_SIZE}, (void*)entry );//参数as用于限制用户进程可以访问的内存, 我们在下一阶段才会使用, 目前可以忽略它
  // 将用户栈的顶部地址赋给 GPRx 寄存器
  pcb->cp->GPRx = (uintptr_t)us2;
  // pcb->cp->GPRx = (uintptr_t) heap.end; //目前我们让Nanos-lite把heap.end作为用户进程的栈顶, 然后把这个栈顶赋给用户进程的栈指针寄存器就可以了.
  // 将栈顶位置存到 GPRx 后，恢复上下文时就可以保证 GPRx 中就是栈顶位置  
  //这里用heap，表示用户栈   在abstract-machine/am/src/platform/nemu/trm.c文件当中定义 Area heap = RANGE(&_heap_start, PMEM_END); //Area heap结构用于指示堆区的起始和末尾
  // draw_ustack((uintptr_t*)us2, (uintptr_t*)heap.end, argc, envc, argv,envp);  //这里暂时先不画了
  draw_ustack((uintptr_t*)us2, (uintptr_t*)new_user_stack, argc, envc, argv,envp); 
}
//事实上, 用户栈的分配是ISA无关的, 所以用户栈相关的部分就交给Nanos-lite来进行, ucontext()无需处理. 
// 目前我们让Nanos-lite把heap.end作为用户进程的栈顶, 然后把这个栈顶赋给用户进程的栈指针寄存器就可以了.

// 栈指针寄存器可是ISA相关的, 在Nanos-lite里面不方便处理. 别着急, 还记得用户进程的那个_start吗? 
// 在那里可以进行一些ISA相关的操作. 于是Nanos-lite和Navy作了一项约定: Nanos-lite把栈顶位置设置到GPRx中, 
// 然后由Navy里面的_start来把栈顶位置真正设置到栈指针寄存器中.


/*
          |               |     //高地址处  用户栈底？？？？  后来自己新创建了用户栈了
          +---------------+ <---- ustack.end     //但是这里有个问题为什么是ustack.end而不是heap.end？？？？？？而且栈的生长方向不是向下吗？？？
          |  Unspecified  |
          +---------------+
          |               | <----------+
          |    string     | <--------+ |
          |     area      | <------+ | |
          |               | <----+ | | |
          |               | <--+ | | | |
          +---------------+    | | | | |
          |  Unspecified  |    | | | | |
          +---------------+    | | | | |
          |     NULL      |    | | | | |
          +---------------+    | | | | |
          |    ......     |    | | | | |
          +---------------+    | | | | |
          |    envp[1]    | ---+ | | | |
          +---------------+      | | | |
          |    envp[0]    | -----+ | | |
          +---------------+        | | |
          |     NULL      |        | | |
          +---------------+        | | |
          | argv[argc-1]  | -------+ | |
          +---------------+          | |
          |    ......     |          | |
          +---------------+          | |
          |    argv[1]    | ---------+ |
          +---------------+            |
          |    argv[0]    | -----------+
          +---------------+
          |      argc     |
          +---------------+ <---- cp->GPRx
          |               |

*/

//尝试自己画一个类似于上面的栈图
void draw_ustack(uintptr_t* us_top, uintptr_t* us_end, int argc, int envc ,char *const argv[], char *const envp[])
{
  
  int num = us_end - us_top;
  printf("+-----------------------------------------+\n| %x: \t\t\t<---- ustack.end\n",us_end);
  us_end--;
  for(int i=0;i<num;i++)
  {
    if(i==0 && i != num-argc-envc-3)
    {printf("| %x:| %x | \t<---- String Area\n",(us_end-i),*(us_end-i));}
    else if(i<num-1 && i>num-argc-2)
    {printf("| %x:| %x | \t<---- argv[%d]\n",(us_end-i),*(us_end-i), num-i-2 );}
    else if(i == num-argc-2)
    {printf("| %x:| %x | \t\t<---- NULL\n",(us_end-i),*(us_end-i));}
    else if (i == num-1)
    {printf("| %x:| %x | \t\t<---- argc & cp->GPRx\n",(us_end-i),*(us_end-i));}
    else if(i<num-argc-2 && i>num-argc-envc-3)
    {printf("| %x:| %x | \t<---- argv[%d]\n",(us_end-i),*(us_end-i), num-i-argc-3 );}
    else if(i == num-argc-envc-3) {printf("| %x:| %x | \t\t<---- NULL\n",(us_end-i),*(us_end-i));}
    else
    {
      printf("| %x:| %x |\n",(us_end-i),*(us_end-i));
    }
  }
  printf("+-----------------------------------------+\n");
}