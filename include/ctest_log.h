//
// Created by oslab on 23-4-14.
//

#ifndef CMOCKERY_CTEST_LOG_H
#define CMOCKERY_CTEST_LOG_H

#include <cstdio>
#include <mutex>

/*测试系统使用多线程，同一时间可能有多个线程同时写日志，所以需要加锁
使用单例模式，保证只有一个日志文件,主要思路如下：
 (1)把无参构造函数和拷贝构造函数私有化；
（2）定义一个类内的静态成员指针；
（3）在类外初始化时，需要new一个对象；
（4）把指针的权限设置为私有，然后提供一个静态成员函数让外面能够获取这个指针。
*/
namespace ctest {
    enum LogLevel {  //日志级别
        LOG_LEVEL_DEBUG = 0,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_FATAL,
    };


    class LogFileInfo final {
    public:
        LogFileInfo(const char *szFile, int nLine);

        ~LogFileInfo();

        const char *GetFile() const;

        int GetLine() const;

    private:
        const char *_szFile = nullptr;
        int _nLine = 0;
    };

    class Log final {
    public:
        Log(const Log &) = delete;
        Log &operator=(const Log &) = delete;
        Log(const char *szPath = nullptr);
        // file
        int createFile();
        static std::shared_ptr<Log> getInstance();
        static LogFileInfo setLogFileInfo(const char *szFile, int nLine);
        //获得日志级别
        LogLevel GetLogLevel() const;
        void SetLogLevel(LogLevel nLogLevel);

        //写日志
        static int WriteLog(const char *szFile,
                            LogLevel nLogLevel,//日志级别
                            int nLine, //行号
                            const char *szType,
                            const char *szMsg, //格式化
                            ...);

        ~Log();

    private:
        bool __Write(const char *szTime, const char *szFile, const char *szLine, const char *szType, const char *szMsg);
        int     __Lock(bool isLock);
        static std::shared_ptr<Log>  log;
        friend class LogMgr;
        friend class LogFileInfo;
        //用于多线程环境下保证只有一个log类实例
        static std::mutex log_mutex;
        //日志级别
        LogLevel _nLogLevel = LOG_LEVEL_DEBUG;
        //handle
        FILE *_pFile = nullptr;
    };

    class LogMgr final {
    public:
        LogMgr();
        ~LogMgr();

        bool Init(const char *szPath);
        void UnInit();

        Log *GetLog();

    private:
        Log *_pLog = nullptr;
    };
}
#endif //CMOCKERY_CTEST_LOG_H
