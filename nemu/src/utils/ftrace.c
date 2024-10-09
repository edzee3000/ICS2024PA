#include <common.h>
#include <isa.h>
#include <elf.h>
#include <ftrace.h>
#include <memory/paddr.h>
#define ftrace_write log_write

// SymEntry *symbol_tbl = NULL; // dynamic allocated
// uint32_t symbol_tbl_size = 0;
// uint32_t call_depth = 0;

// //call（调用）和ret（返回）都需要记录指令所在的地址，用参数pc表示  call的第二个参数target表示被调用函数的首地址
// //记录函数调用
// void trace_func_call(paddr_t pc, paddr_t target) {
//   if (symbol_tbl == NULL) return;
//   call_depth++;
//   if (call_depth <= 2) return; // 忽略 _trm_init 和 main 函数的调用
//   int i = find_symbol_func(target, true);//这里有一个寻找函数符号位置的函数find_symbol_func还没有写
//   ftrace_write(FMT_PADDR ": %*scall [%s@" FMT_PADDR "]\n",
// 	pc,
// 	(call_depth-3)*2, "",
// 	i>=0?symbol_tbl[i].name:"???",
// 	target
//   );
// }
// //记录函数返回
// void trace_func_ret(paddr_t pc) {
//   if (symbol_tbl == NULL) return;
//   if (call_depth <= 2) return; // ignore _trm_init & main
//   int i = find_symbol_func(pc, false);
//   ftrace_write(FMT_PADDR ": %*sret [%s]\n",
//   	pc,
//  	(call_depth-3)*2, "",
// 	i>=0?symbol_tbl[i].name:"???"
//   );
//   call_depth--;
// }

FunctionInfo functions[64];//假设最多只有64个函数
uint32_t num_functions = 0;//记录一共有多少个函数
Elf32_Sym sym;
char *string_table = NULL;

//解析elf文件函数（默认这里是elf32位的）已经自己查过使用 riscv64-linux-gnu-readelf -h build/add-riscv32-nemu.elf 命令，结果的Magic魔数第五个数01表示是32位的
void parse_elf(const char *elf_file) {
  if (elf_file == NULL) return;
  Log("ELF File is: %s", elf_file);
  //int fd = open(elf_file, O_RDONLY|O_SYNC);
  //Assert(fd >= 0, "Error %d: unable to open %s\n", fd, elf_file);
  
  FILE *file=fopen(elf_file,"rb");if(!file){perror("打开文件出错\n");assert(0);}// 打开 ELF 文件
  Elf32_Ehdr ehdr; if(fread(&ehdr, sizeof(Elf32_Ehdr), 1, file) != 1) {perror("读取文件头出错\n");fclose(file);assert(0);};  // 读取 ELF 文件头，读取一个即可
  if(memcmp(ehdr.e_ident,ELFMAG,SELFMAG)==0){}//如果比较为0的话则表示确实是ELF文件




  //定位到节头表
  fseek(file, ehdr.e_shoff, SEEK_SET);
  //读取节头表项
  Elf32_Shdr *shdr=(Elf32_Shdr *)malloc(ehdr.e_shentsize*ehdr.e_shnum);
  for (int i = 0; i < ehdr.e_shnum; i++) {if (fread(&shdr[i], sizeof(Elf32_Shdr), 1, file) != 1) {perror("读取节头表项出错\n");free(shdr);fclose(file);exit(EXIT_FAILURE);}}
   // 遍历节头表，找到符号表节
  for (int i = 0; i < ehdr.e_shnum; i++) {
    if (shdr[i].sh_type == SHT_SYMTAB) {
        printf("在索引i为%d处找到符号表节\n", i);
        // 接下来处理符号表节
        size_t num_symbols = shdr[i].sh_size / shdr[i].sh_entsize;// 计算符号表的条目数量
        fseek(file, shdr[i].sh_offset, SEEK_SET);// 读取符号表条目

        // 读取字符串表
        fseek(file, shdr[i].sh_offset, SEEK_SET);
        string_table = (char *)malloc(shdr[i].sh_size);
        if (fread(string_table, shdr[i].sh_size, 1, file) != 1) {
            perror("读取字符串表发生错误");
            free(string_table);
            fclose(file);
            exit(EXIT_FAILURE);
        }


        for (size_t j = 0; j < num_symbols; j++) {//循环遍历符号表寻找STT_FUNC
          if (fread(&sym, sizeof(Elf32_Sym), 1, file) != 1) {perror("读取符号表条目某一条出错");fclose(file);exit(EXIT_FAILURE);}
          // 检查符号类型如果是函数类型的话
          if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC) {
            //char *name = (char*)malloc(shdr[i].sh_size);//防止'\0'不在name里面
            //fseek(file, sym.st_name, SEEK_SET);
            //if(fread(name, sizeof(char), 1, file)!=1){perror("读取函数名称出错\n");free(shdr);fclose(file);exit(EXIT_FAILURE);}//注意fread函数是有返回值的为1的时候才表示读取成功
            char *name=&string_table[sym.st_name];
            printf("函数符号名称为: %s\n", name);
            strcpy(functions[num_functions].name, name);
            functions[num_functions].addr = sym.st_value;
            functions[num_functions].size = sym.st_size;
            num_functions++;//func函数个数加一
            free(name);
          }       
        }
    break;
    }
  }
  free(shdr);
  fclose(file);
}

//打印函数名称的函数
void print_func_name(const char *elf_file)
{
  parse_elf(elf_file);
  for (uint32_t i = 0; i < num_functions; i++) {
      printf("Function: %s, Address: %#x, Size: %d\n",
              functions[i].name,
              (paddr_t)functions[i].addr,
              functions[i].size);
}
}

