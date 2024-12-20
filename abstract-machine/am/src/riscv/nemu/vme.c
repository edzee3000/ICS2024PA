#include <am.h>
#include <nemu.h>
#include <klib.h>
// #include <riscv.h>

static AddrSpace kas = {};//内核虚拟地址空间(kas)的定义
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings  内核存储映射  物理内存
  NEMU_PADDR_SPACE      //定义在 abstract-machine/am/src/platform/nemu/include/nemu.h 当中
  // 0x80000000 ~ 0x88000000：内核区和用户区，在无 VME 下，用户程序被链接到内存位置 0x83000000 附近
  // 0xa1000000 ~ 0xa1200000：显存
  // 0xa0000000 ~ 0xa0001000：其余设备
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)   // 虚拟内存  范围为 0x40000000 ~ 0x80000000 

static inline void set_satp(void *pdir) {
  //riscv_xlen是32，把1左移31位赋值给mode，得到MODE位，此时MODE 的值为 1，表示使用两级页表结构，pdir右移12位得到的是pdir的高20位，即一级页表的基地址，它作为PPN位，与mode按位或操作，得到32位数据赋值给satp寄存器
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));//诶但是为什么我没有印象我有写过有关于satp寄存器的相关操作啊？？？？？
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}


//VME的主要功能已经通过上述三个API抽象出来了. 最后还有另外两个统一的API:
bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  //由于页表位于内存中, 但计算机启动的时候, 内存中并没有有效的数据, 因此我们不可能让计算机启动的时候就开启分页机制. 
  // 操作系统为了启动分页机制, 首先需要准备一些内核页表. 框架代码已经为我们实现好这一功能了
  pgalloc_usr = pgalloc_f;   //页分配给usr的函数方法  在map里面会用到
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);////调用用户提供的页面分配函数 pgalloc_f 来为 kas 分配一个页4kb大小的内存，把物理页首地址即页目录地址赋值给ptr

  // 但是在vme_init()中，包含如下部分代码，它从segments中获取真实物理地址赋值给va，然后调用map
  // 且map的"va","pa"参数均传的是真实物理地址，这样在分页模式下, Nanos-lite可以把内存的物理地址直接当做虚拟地址来访问，因为它们都是相同的
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
      //map()是VME中的核心API, 它需要在虚拟地址空间as的页目录和页表中填写正确的内容, 使得将来在分页模式下访问一个虚拟页(参数va)时, 
      // 硬件进行page table walk后得到的物理页, 正是之前在调用map()时给出的目标物理页(参数pa). 这再次体现了分页是一个软硬协同才能工作的机制: 
      // 如果map()没有正确地填写这些内容, 将来硬件进行page table walk的时候就无法取得正确的物理页.
    }
  }
  // Sv32 采用两级页表结构，页目录基地址和分页使能位都是位于satp寄存器中
  // satp 寄存器的位域包括 MODE、ASID（Address Space ID）、PPN（Page Table Base Physical Address）等字段
  // MODE：satp 寄存器的最高位是 MODE，用于指定页表的模式。在 Sv32 模式下，MODE 的值为 1，表示使用两级页表结构。其他模式的定义取决于 RISC-V 的扩展。
  // ASID：Address Space ID 是用来区分不同的地址空间的标识符。ASID 的存在使得处理器能够支持多任务，因为不同的任务可以有不同的 ASID。ASID 的作用是在 TLB（Translation Lookaside Buffer，地址翻译缓冲区）中标识不同的地址空间。在同一个 ASID 下，TLB 可以缓存之前的地址翻译结果，提高效率。
  // PPN：Page Table Base Physical Address，即页表的基地址。PPN 指定了页表的物理地址。在 Sv32 模式下，页表是一个两级结构，PPN 指向第一级页表的物理地址。通过多级的页表结构，可以有效地管理和翻译大范围的虚拟地址空间。
  
  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}//用于进行VME相关的初始化操作. 其中它还接受两个来自操作系统的页面分配回调函数的指针, 让AM在必要的时候通过这两个回调函数来申请/释放一页物理页.




// 虚存管理的具体实现自然是架构相关的, 比如在x86中用于存放页目录基地址的CR3, 在riscv32上并不叫这个名字, 
// 访问这个寄存器的指令自然也各不相同. 再者, 不同架构中的页面大小可能会有差异, 页表项的结构也不尽相同, 
// 更不用说有的架构还可能有多于两级的页表结构了. 于是, 我们可以将虚存管理的功能划入到AM的一类新的API中, 
// 名字叫VME(Virtual Memory Extension).

// 老规矩, 我们来考虑如何将虚存管理的功能抽象成统一的API. 换句话说, 虚存机制的本质究竟是什么呢? 
// 我们在上文已经讨论过这个问题了: 虚存机制, 说白了就是个映射(或函数). 也就是说, 
// 本质上虚存管理要做的事情, 就是在维护这个映射. 但这个映射应该是每个进程都各自维护一份, 因此我们需要如下的两个API:

// 创建一个默认的地址空间
void protect(AddrSpace *as) {// 话说as是啥来着？？？？？全称是啥   噢噢噢地址空间啊Address Space
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));//调用pgalloc_usr()返回分配的物理页的首地址
  as->ptr = updir;//物理页首地址也就是页目录地址赋值给ptr
  as->area = USER_SPACE;//虚拟内存0x40000000 ~ 0x80000000,虚拟地址空间中用户态的范围，每个进程都会访问这一范围虚拟地址
  as->pgsize = PGSIZE;//页面的大小4kb
  // map kernel space  映射内核空间
  memcpy(updir, kas.ptr, PGSIZE);//将内核地址空间的内容复制到新分配的用户页目录
  
}
// 销毁指定的地址空间
void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  // 在__am_irq_handle()的开头调用__am_get_cur_as() (在abstract-machine/am/src/$ISA/nemu/vme.c中定义), 
  // 来将当前的地址空间描述符指针保存到上下文中
  //但是为什么这里可能要加上一个条件判断是否为NULL
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  //因此我们只要在kcontext()中将上下文的地址空间描述符指针设置为NULL, 来进行特殊的标记, 等到将来在__am_irq_handle()中调用__am_switch()时, 如果发现地址空间描述符指针为NULL, 就不进行虚拟地址空间的切换.
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}




#define VA_OFFSET(addr) (addr & 0x00000FFF) //提取虚拟地址的低 12 位，即在页面内的偏移。
#define VA_VPN_0(addr)  ((addr >> 12) & 0x000003FF) //提取虚拟地址的中间 10 位，即一级页号
#define VA_VPN_1(addr)  ((addr >> 22) & 0x000003FF) //提取虚拟地址的高 10 位，即二级页号
#define PA_OFFSET(addr) (addr & 0x00000FFF)//提取物理地址的低 12 位，即在页面内的偏移
#define PA_PPN(addr)    ((addr >> 12) & 0x000FFFFF)//提取物理地址的高 20 位，即物理页号
#define PTE_PPN 0xFFFFF000 // 31 ~ 12
/*
VA:
31         22 21      12 11          0
+------------------------------------+
|   VPN[1]   |  VPN[0]  |   offset   |
+------------------------------------+
PA:
33                       12 11            0
+-----------------------------------------+
|          PPN             |   offset     |
+-----------------------------------------+
页表项结构为:
31           20 19          10 9 8 7 6 5 4 3 2 1 0
+-------------------------------------------------+
|    PPN[1]    |    PPN[0]    |RSW|D|A|G|U|X|W|R|V|      
+-------------------------------------------------+
*/
//有了地址空间, 我们还需要有相应的API来维护它们. 于是很自然就有了如下的API:
void map(AddrSpace *as, void *va, void *pa, int prot) {
  uintptr_t va_trans = (uintptr_t) va;
  uintptr_t pa_trans = (uintptr_t) pa;//需要进行强制类型转换
  // assert(PA_OFFSET(pa_trans) == 0);//判断是否刚好页对齐   因此注意传进来的pa和va都是需要进行页对齐的
  // assert(VA_OFFSET(va_trans) == 0);
  //提取虚拟地址的二级页号和一级页号，以及物理地址的物理页号
  uint32_t ppn = PA_PPN(pa_trans);
  uint32_t vpn_1 = VA_VPN_1(va_trans);
  uint32_t vpn_0 = VA_VPN_0(va_trans);
  //获取地址空间的页表基址和一级页表的目标位置
  PTE * page_dir_base = (PTE *) as->ptr;//通过satp寄存器寻找一级页表（页目录）基址
  PTE * page_dir_location = page_dir_base + vpn_1;//确定一级页表（页目录）目标的位置
  //如果一级页表中的页表项的地址(二级页表的基地址)为空，创建并填写页表项
  if (*page_dir_location == 0) { 
    PTE * page_table_base = (PTE *) pgalloc_usr(PGSIZE);//通过 pgalloc_usr 分配一页物理内存，作为二级页表的基地址
    *page_dir_location = ((PTE) page_table_base) | PTE_V;//将这个基地址填写到一级页表的页表项中，同时设置 PTE_V 表示这个页表项是有效的。
    PTE * page_table_target = page_table_base + vpn_0; //计算在二级页表中的页表项的地址
    *page_table_target = (ppn << 12) | PTE_V | PTE_R | PTE_W | PTE_X;//将物理页号 ppn 左移 12 位，即去掉低 12 位的偏移，与权限标志 PTE_V | PTE_R | PTE_W | PTE_X 组合，填写到二级页表的页表项中。
  } else {
    PTE * page_table_base = (PTE *) ((*page_dir_location) & PTE_PPN);//取得一级页表项的内容，然后 & PTE_PPN 通过按位与操作提取出页表的基地址，提取高20位，低 12 位为零  页表对齐  因为VPN[0]是10位的再乘以每个表项4B一共12位
    PTE * page_table_target = page_table_base + vpn_0;//通过加上 vpn_0 计算得到在二级页表中的目标项的地址
    *page_table_target = (ppn << 12) | PTE_V | PTE_R | PTE_W | PTE_X;//将物理页号 ppn 左移 12 位，即去掉低 12 位的偏移，与权限标志 PTE_V | PTE_R | PTE_W | PTE_X 组合，填写到二级页表的目标项中。
    //这个过程其实很明确，唯一让人疑惑的是PPN是一个22位的数据，最后又拼接了一个12位的地址上去，这样以来最后获得的物理地址数据岂不是34位的吗，
    // 事实上确实如此，但是二级页表里的数据其实是我们填的，所以我们只要不在二级页表中存放22位的地址就可以了。
    // 因此我们可以很清楚地得出一个结论就是每个二级页表的每一项的最高两位理论上来说都应该是0（？？？？？？？？？？？？等等这个解释总感觉有一点点问题……算了之后再说吧）
  }


}//它用于将地址空间as中虚拟地址va所在的虚拟页, 以prot的权限映射到pa所在的物理页. 当prot中的present位为0时, 表示让va的映射无效. 由于我们不打算实现保护机制, 因此权限prot暂不使用.


// Virtual Memory Extension 虚拟内存管理/拓展

//创建用户进程的上下文则需要一些额外的考量. 在PA3的批处理系统中, 我们在naive_uload()中直接通过函数调用转移到用户进程的代码, 
// 那时候使用的还是内核区的栈, 万一发生了栈溢出, 确实会损坏操作系统的数据, 不过当时也只有一个用户进程在运行, 我们也就不追究了.
//  但在多道程序操作系统中, 系统中运行的进程就不止一个了, 如果让用户进程继续使用内核区的栈, 万一发生了栈溢出, 就会影响到其它进程的运行, 这是我们不希望看到的.
//因此, 和内核线程不同, 用户进程的代码, 数据和堆栈都应该位于用户区, 而且需要保证用户进程能且只能访问自己的代码, 数据和堆栈. 
// 为了区别开来, 我们把PCB中的栈称为内核栈, 位于用户区的栈称为用户栈. 于是我们需要一个有别于kcontext()的方式来创建用户进程的上下文, 
// 为此AM额外准备了一个API ucontext()(在abstract-machine/am/src/nemu/isa/$ISA/vme.c中定义),
Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  //参数as用于限制用户进程可以访问的内存, 我们在下一阶段才会使用, 目前可以忽略它; 
  // kstack是内核栈, 用于分配上下文结构, entry则是用户进程的入口. 由于目前我们忽略了as参数, 
  // 所以ucontext()的实现和kcontext()几乎一样, 甚至比kcontext()更简单: 连参数都不需要传递.
  //用户栈的分配是ISA无关的, 所以用户栈相关的部分就交给Nanos-lite来进行, ucontext()无需处理.
  //  目前我们让Nanos-lite把heap.end作为用户进程的栈顶, 然后把这个栈顶赋给用户进程的栈指针寄存器就可以了.
  Context *cp = (Context *)kstack.end - 1;
  cp->mepc = (uintptr_t)entry; 
  assert(cp->mepc >= 0x40000000 && cp->mepc <= 0x88000000);//cp->mepc是有一定的范围限制的
  
  cp->mstatus = 0x1800 | MSTATUS_MPIE;//这一步好像是为了迎合difftest？？？？不管了   
  // 现在来填坑了  这一步是在PA4.3部分在构造上下文的时候, 设置正确中断状态, 使得将来恢复上下文之后CPU处于开中断状态.
  // 只需要设置 MPIE 为 1 即可，在切换到内核线程或用户进程时便会将 mstatus.MPIE 还原到 mstatus.MIE 中
  // 在riscv32中, 如果mstatus中的MIE位为0, 则CPU处于关中断状态
 
  // cp->gpr[0] = 0; //等等这一步是在干嘛？？？？  为什么要把gpr[0]设置为0？？？？？？？

  cp->pdir = as->ptr;   //修改ucontext()的实现, 在创建的用户进程上下文中设置地址空间描述符指针
  // return NULL;
  return cp;
  // 栈指针寄存器可是ISA相关的, 在Nanos-lite里面不方便处理. 别着急, 还记得用户进程的那个_start吗? 
  // 在那里可以进行一些ISA相关的操作. 于是Nanos-lite和Navy作了一项约定: Nanos-lite把栈顶位置设置到GPRx中,
  //  然后由Navy里面的_start来把栈顶位置真正设置到栈指针寄存器中.
}//用于创建用户进程上下文. 我们之前已经介绍过这个API, 但加入虚存管理之后, 我们需要对这个API的实现进行一些改动, 具体改动会在下文介绍.
