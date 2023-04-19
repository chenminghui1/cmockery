//
// Created by oslab on 23-4-14.
//
#include <memory>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <fcntl.h>
#include <iomanip>
#include "../include/ctest_log.h"


namespace ctest {

    void Log::init(ctest::LogLevel nLogLevel) {
        SetLogLevel(nLogLevel);

        createFile();
    }

    Log::Log(const char *szPath) {
        SetLogLevel(LOG_LEVEL_DEBUG);
    }
    //创建日志文件，并获得文件句柄
    int ctest::Log::createFile() {
        //todo:让log文件与测试用例文件同名
        _pFile.open("testlog.txt", std::ios::out | std::ios::app);
        if(!_pFile.is_open()){
            std::cout<<"open file failed"<<std::endl;
            return 0;
        }
        return 1;
    }
    LogLevel Log::GetLogLevel() const {
        return this->_nLogLevel;
    }
    void ctest::Log::SetLogLevel(ctest::LogLevel nLogLevel) {
        this->_nLogLevel = nLogLevel;
    }


    void Log::outputLog(const char *szTime, const char *szInfo, const char *szMsg) {
        if(Log::getInstance()->_pFile.is_open()){
            printf("%s %s %s", szTime, szInfo, szMsg);
        }
    }


    //在unix系统下，同一个进程内针对同一个FILE*的操作是线程安全的
    int Log::WriteLog(LogLevel nLogLevel,//日志级别
                        int nLine, //行号
                        const char *szType,
                        const char *szMsg, //格式化
                        ...) {
        int ret = 0;
        //获取日期和时间
        time_t t = time(nullptr);
        tm *p = localtime(&t);
        getInstance()->_pFile<<p->tm_year + 1900 << "-" << p->tm_mon + 1 << "-" << p->tm_mday << " " << p->tm_hour << ":" << p->tm_min << ":" << p->tm_sec;
        //log级别
        char * logLevel = nullptr;
        switch (nLogLevel) {
            case LOG_LEVEL_DEBUG:
                logLevel = "DEBUG";
                break;
            case LOG_LEVEL_INFO:
                logLevel = "INFO";
                break;
            case LOG_LEVEL_WARN:
                logLevel = "WARN";
                break;
            case LOG_LEVEL_ERROR:
                logLevel = "ERROR";
                break;
            case LOG_LEVEL_FATAL:
                logLevel = "FATAL";
                break;
            default:
                logLevel = "UNKNOWN";
                break;
        }
        // [进程号][线程号][Log级别][文件名][函数名:行号]
        char lcoInfo[1024] = {0};
        char* format = "[PID:%d][TID:%d][%s][%-s][%s:%d]";

        ret = sprintf(lcoInfo, format, getpid(), gettid(), logLevel, szType,  nLine);
        if (ret < 0) {
            return -1;
        }
        //日志正文
        char logMsg[1024] = {0};
        va_list args;
        va_start(args, szMsg);
        ret = vsprintf(logMsg, szMsg, args);
        if (ret < 0) {
            return -1;
        }
        va_end(args);

    }

    Log::~Log() {
        if(_pFile.is_open()){
            _pFile.close();
        }
    }



//c++11之后，可以使用静态局部变量的方式解决单例的线程安全问题，由编译器保证线程安全
////直到调用getInstance()时才会创建对象
//std::shared_ptr<Log> Log::getInstance() {
//    //双重检查锁
//    if(log == nullptr){
//        log_mutex.lock();
//        if(log == nullptr){
//            //需要使用智能指针解决释放问题
//            log = std::make_shared<Log>();
//        }
//        log_mutex.unlock();
//    }
//    return log;
//}

}