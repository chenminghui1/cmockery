# cmockery单元测试库
通过完成在源代码中给出的运行事例，逐步完善cmockery的仿写

@[toc]

## 运行测试（_run_tests _run_test)
### _run_tests 
在unit_tests中初始化为一个列表，通过遍历列表，完成每个单独的测试

### _run_test
实际运行每一个测试项目的函数，先通过使用c函数中的setjmp确保测试出错后可以
恢复程序的环境  
```c  
default_signal_functions[i] = signal(  
           exception_signals[i], exception_handler);
```
异常处理函数：
处理五种异常：
```cpp
static const int exception_signals[] = {
        SIGFPE,  //浮点异常，例如除以零或溢出。
        SIGILL,  //非法指令，例如执行了未定义或特权的指令。
        SIGSEGV, //段错误，例如访问了无效或受保护的内存地址。
        SIGBUS,  //总线错误，例如对齐错误或物理内存故障。
        SIGSYS,  //无效的系统调用，例如传递了错误的参数或调用了未实现的功能。
};
```
## 边界测试、

## 错误处理测试、

## 路径测试、

## 局部数据结构测试、
`assert_true`
`assert_false`




## 模块接口测试。

## 常见错误的判断
常见错误类型有，接口错误，I/0错误，数据结构错误，算法错误，比较及控制逻辑错误，错误处理错误，







# example文件
## run_tests.c
在第一个文件中，调用了comckery中的两个函数：
`         unit_test(null_test_success),
unit_test_with_prefix(someprefix_, null_test_success),`


### 改变
在_run_tests中使用new替代malloc


# 用到的外部知识

## 静态库和动态库
### 静态库和动态库的区别
静态库在程序的链接阶段被复制到了程序中，动态库在链接阶段没有被复制到程序中，而是在程序运行
时由系统动态的加载到内存中提供给程序调用。
* 静态库的好处：编译后不需要其他库函数的支持，可移植性好
* 缺点： 
  1. 如果静态库发生改变，必须重新编译
  2.  相同的库文件可能在内存中加载多份，消耗系统资源
* 动态库优点：
  1. 可实现不同进程间的资源共享
  2. 动态库升级简单，只需要替换库文件，无需重新编译应用程序
  3. 可以控制何时加载动态库，不调用库函数动态库不会被加载
* 动态库缺点：
  1. 加载速度比静态库慢
  2. 发布程序需要提供依赖的动态库


### 创建
#### 静态库制作
1. gcc获得.o文件
2. 将.o文件打包，使用ar工具(archive)  
    `ar rcs libxxx.a xxx.o xxx.o  `  
    r：将文件插入到备存文件中  
    c:建立备存文件  
    s:索引  
   ![img.png](doc_source/img.png "静态库制作")
#### 动态库制作
1. gcc获得.o文件，得到和位置无关的代码  
`gcc -c -fpic/-fPIC a.c b.c`
2. gcc 得到动态库  
`gcc -shared a.o b.o -o libxxx.so`
   ![img_1.png](doc_source/img_1.png "动态库创建流程")
### 使用

## 类型转换
#### c++四种强制类型转换
* static_cast<type>(value)
> static_cast<> 用于非多态类型的转换，如基本数据类型之间的转换，
> 将指针转换为 void*，或将 void* 转换为指针。主要完成的和隐式类型转化累死的工作

* dynamic_cast<>()
> 主要用于将基类指针转化为派生类指针，向下转换的成功与否还与将要转换的类型有关，
> 即要转换的指针指向的对象的实际类型与转换以后的对象类型一定要相同，否则转换失败。  
> （1）其他三种都是编译时完成的，dynamic_cast是运行时处理的，运行时要进行类型检查。  
> （2）不能用于内置的基本数据类型的强制转换。  
> （3）dynamic_cast转换如果成功的话返回的是指向类的指针或引用，转换失败的话则会返回NULL。  
> （4）使用dynamic_cast进行转换的，基类中一定要有虚函数，否则编译不通过。
* reinterpret_cast<>()
> 是c++一切决定于程序员的思想的体现，可以进行一些有破坏性的转化，但是一切后果自己承担
> reinterpret_cast主要有三种强制转换用途：改变指针或引用的类型、
> 1. 改变指针或引用的类型、 
> 2. 将指针或引用转换为一个足够长度的整形、
> 3. 将整型转换为指针或引用类型。
> 
#### 特殊情况
* 将void*转化为unsigned int  
 `static_cast<int>(reinterpret_cast<intptr_t>(void_p));`  
  intptr_t 是 C++ 中的一个整数类型，它是一个有符号的整数类型，可以存储指针或句柄的值。它的大小与`void *`相同，通常为 32 位或 64 位.
  作为中间变量，足够大可以包含`void*`的值，又可以在转化为整形是直接截断不被编译器报错

## 顶层函数
### 函数指针
作为实参传递给另一个函数的函数称为回调函数
函数指针作为一个合理的类型，可以作为函数参数。在函数中，可以使用其函数指针参数
调用实参指向的函数。可以指定函数名称作为函数指针类型的参数。
### 函数对象与lambda表达式
函数对象就是生成一个类，实现类似函数调用的使用方式，最关键的就是重载operator()。
lambda表达式也是一个函数对象，编译遇到一个lambda表达式就会新产生一个类

### 标准库<function.h>

## 异常处理
### try{...} catch(){...}
1. try里面定义的变量是局部变量，无法在代码块外面使用

## 信号
C++的信号处理函数在`<csignal>`里面。  
![img.png](doc_source/signal.png "常见信号描述")
### signal()函数
```c++
typedef void (*sig_t)(int);
sig_t signal(int signum, sig_t handler);
```
### sigaction()函数
sigaction()允许单独获取信号的处理函数而不是设置，并且还可以设置各种属性对调用信号处理函数时
的行为施以更加精准的控制，其函数原型如下所示:
```c++
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```
<strong>函数参数和返回值含义如下：</strong>  
<strong>signum</strong>：需要设置的信号，除了 SIGKILL 信号和 SIGSTOP 信号之外的任何信号。  
<strong>act</strong>：act 参数是一个 `struct sigaction` 类型指针，指向一个 struct sigaction 数据结构，该数据结构描述了信
号的处理方式，稍后介绍该数据结构；如果参数 act 不为 NULL，则表示需要为信号设置新的处理方式；如
果参数 act 为 NULL，则表示无需改变信号当前的处理方式。  
<strong>oldact</strong>：oldact 参数也是一个 struct sigaction 类型指针，指向一个 struct sigaction 数据结构。如果参数
oldact 不为 NULL，则会将信号之前的处理方式等信息通过参数 oldact 返回出来；如果无意获取此类信息，
那么可将该参数设置为 NULL。  
<strong>返回值</strong>：成功返回 0；失败将返回-1，并设置 errno。
```c++
struct sigaction {
 void (*sa_handler)(int);
 void (*sa_sigaction)(int, siginfo_t *, void *);
 sigset_t sa_mask;
 int sa_flags;
 void (*sa_restorer)(void);
};
```
### 信号掩码
* 当应用程序调用 signal()或 sigaction()函数为某一个信号设置处理方式时，进程会自动将该信号添加
到信号掩码中，这样保证了在处理一个给定的信号时，如果此信号再次发生，那么它将会被阻塞；
* 可以使用 sigprocmask()系统调用，随时可以显式地向信号掩码中添加/
移除信号。

## 工厂模式
在工厂模式中，我们在创建对象时不会对客户端暴露创建逻辑，并且是通过使用一个共同的接口来指向新创建的对象。  
<strong>意图</strong>：定义一个创建对象的接口，让其子类自己决定实例化哪一个工厂类，工厂模式使其创建过程延迟到子类进行。  
优点： 
1. 一个调用者想创建一个对象，只要知道其名称就可以了。 
2. 扩展性高，如果想增加一个产品，只要扩展一个工厂类就可以。
3. 屏蔽产品的具体实现，调用者只关心产品的接口。

缺点：每次增加一个产品时，都需要增加一个具体类和对象实现工厂，使得系统中类的个数成倍增加，在一定程度上增加了系统的复杂度，同时也增加了系统具体类的依赖。这并不是什么好事。


## git使用
### 使用场景一：下班后使用另一台电脑继续开发


# 困难
### 问题：宏定义
1. 宏定义的生存周期：从宏的定义位置，直到文件结束或遇到undef
2. 不同文件中的宏定义的符号的在编译过程中： ？？？
### 不同文件中符号的导入顺序
1. 头文件的导入： 如果头文件中有依赖关系，需要按照依赖的顺序导入
### 运行结果的输出上
1. 在运行最终完成，结果的输出上，从stderr和stdout输出的信息总是分开输出，而在gdb调试过程中
输出是按照预期顺序输出的  
answer： stderr和stdout在默认情况下，stdout是行缓冲的，它的输出会放在一个buffer里面，只有到换行的时候，才会输出到屏幕。而stderr是无缓冲的，
会直接输出1。所以，stderr和stdout并不共用一个缓冲区。如果想刷新缓冲区，可以使用操作符`flush`  
例：`cout << "Hello, World! " << "Flush the screen now!!!" << flush`
2. 

### 内联函数
1. 声明和实现分开的话，内联函数需要在实现处也加上inline关键词吗
answer：需要。在C++中，如果将内联函数的实现和声明分开，需要在实现处也加上inline关键词。
因为内联函数要在调用点展开，所以编译器必须随处可见内联函数的定义，要不然就成了非内联函数的调用了。所以，每个调用了内联函数的文件都出现了
该内联函数的定义。


### new
1. operator new 操作符:  
   new operator操作符可以被重载，不调用构造函数，使用示例：  
   `FUNC *pFunc2 = (FUNC*) ::operator new(sizeof(FUNC));`   
    `::operator delete pFunc2`  
   ::表示全局，（type*)因为operator new返回void*指针，需要类型转换为对应类型。  
2. new  operator 操作符：
    operator new操作符不可以被重载，调用构造函数，调用的时候先申请内存，再调用构造函数，使用示例：
    ```c++
    type * ptr = new type(initi_list);   
    type * ptr = new []type(initi_list);
   ```
3. placement new操作符：  
    仅仅返回已经申请好内存的指针，它通常应用在对效率要求高的场景下，提前申请好内存，能够节省申请内存过程中耗费的时间。
    ![img.png](doc_source/placement_new.png)

### 调用另一个文件namespace里面定义的内容
在文件`allocate_module.cpp`中，在cmockery没有定义namespace ctest时，原本使用报错无法找到。
```c++
extern void* _test_malloc(const size_t size, const char* file, const int line);
extern void* _test_calloc(const size_t number_of_elements, const size_t size,
                          const char* file, const int line);
extern void _test_free(void* const ptr, const char* file, const int line);
```
此时需要
```c++
namespace ctest{
extern void* _test_malloc(const size_t size, const char* file, const int line);
extern void* _test_calloc(const size_t number_of_elements, const size_t size,
                          const char* file, const int line);
extern void _test_free(void* const ptr, const char* file, const int line);
}

//此时就可以使用namespace：：funcion调用了；
void leak_memory() {
    int * const temporary = (int*)ctest::malloc(sizeof(int));
    *temporary = 0;
}

```
























