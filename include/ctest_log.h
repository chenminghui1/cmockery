//
// Created by oslab on 23-4-14.
//

#ifndef CMOCKERY_CTEST_LOG_H
#define CMOCKERY_CTEST_LOG_H

#include <cstdio>
#include <mutex>
#include <fstream>
/*测试系统使用多线程，同一时间可能有多个线程同时写日志，所以需要加锁
使用单例模式，保证只有一个日志文件,主要思路如下：
 (1)把无参构造函数和拷贝构造函数私有化；
（2）定义一个类内的静态成员指针；
（3）在类外初始化时，需要new一个对象；
（4）把指针的权限设置为私有，然后提供一个静态成员函数让外面能够获取这个指针。
*/

namespace ctest {
    //定义宏，方便调用
    #define LOG_DEBUG(format, ...) \
        ctest::Log::getInstance().WriteLog(ctest::LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) \
        ctest::Log::getInstance().WriteLog(ctest::LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) \
        ctest::Log::getInstance().WriteLog(ctest::LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) \
        ctest::Log::getInstance()->WriteLog(ctest::LOG_LEVEL_ERROR, __LINE__, __FILE__ ,format, ##__VA_ARGS__)


    enum LogLevel {  //日志级别
        LOG_LEVEL_DEBUG = 0,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_FATAL,
    };




    class Log final {
    public:
        Log(const Log &) = delete;
        Log &operator=(const Log &) = delete;
        Log(const char *szPath = nullptr);
        void init(LogLevel nLogLevel = LOG_LEVEL_DEBUG);
        //单例模式的局部静态对象
        static std::unique_ptr<Log> getInstance() {
            static std::unique_ptr<Log> log = std::make_unique<Log>();
            return std::move(log);
        }
        // file
        int createFile();
        static void outputLog(const char *szTime,  \
                                const char *szInfo,  \
                              const char *szMsg);
        //获得日志级别
        LogLevel GetLogLevel() const;
        void SetLogLevel(LogLevel nLogLevel);

        //写日志
        static int WriteLog(
                            LogLevel nLogLevel,//日志级别
                            int nLine, //行号
                            const char *szType, //文件名
                            const char *szMsg, //格式化
                            ...);

        ~Log();
    private:
        static std::unique_ptr<Log> log;
        //用于多线程环境下保证只有一个log类实例
        static std::mutex log_mutex;
        //日志级别
        LogLevel _nLogLevel = LOG_LEVEL_DEBUG;
        //日志文件信息
        std::ofstream  _pFile ;
    };


}
#endif //CMOCKERY_CTEST_LOG_H
