# cmockery单元测试库
通过完成在源代码中给出的运行事例，逐步完善cmockery的仿写

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