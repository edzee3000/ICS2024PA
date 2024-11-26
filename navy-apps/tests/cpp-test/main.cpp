#include <stdio.h>

class Test {
public:
  Test()  { printf("%s,%d: Hello, Project-N!\n", __func__, __LINE__); }
  ~Test() { printf("%s,%d: Goodbye, Project-N!\n", __func__, __LINE__); }
};

Test test;

int main() {
  printf("%s,%d: Hello world!\n", __func__, __LINE__);
  return 0;
}

// Navy准备了一个测试cpp-test. 这个测试程序做的事情非常简单: 代码中定义了一个类, 在构造函数和析构函数中进行输出, 
// 并通过这个类定义了一个全局对象. 在Navy的native上直接运行它, 你可以看到程序按照构造函数->main()->析构函数的顺序来运行, 
// 这是因为Navy的native会链接Linux的glibc, 它提供的运行时环境已经支持全局对象的构造和销毁. 
// 但如果你通过Nanos-lite来运行它, 你会发现程序并没有调用构造函数和析构函数, 
// 这样就会使得全局对象中的成员处于未初始化的状态, 程序访问这个全局对象就会造成非预期的结果.


// 实际上, C++的标准规定, "全局对象的构造函数调用是否位于main()函数执行之前" 是和编译器的实现相关的(implementation-defined behavior), 
// g++会把全局对象构造函数的初始化包装成一个类型为void (*)(void)的辅助函数, 
// 然后把这个辅助函数的地址填写到一个名为.init_array的节(section)中. 
// 这个特殊的节可以看做是一个void (*)(void)类型的函数指针数组, 专门用于收集那些需要在main()函数执行之前执行的函数. 
// 这样以后, CRT就可以遍历这个数组, 逐个调用这些函数了.




