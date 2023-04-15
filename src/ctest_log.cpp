//
// Created by oslab on 23-4-14.
//
#include <memory>
#include "../include/ctest_log.h"
namespace ctest {

Log::Log(const char *szPath) {
    SetLogLevel(LOG_LEVEL_DEBUG);
    setLogFileInfo("test", 1);
    createFile();
}

int ctest::Log::createFile() {
    return 0;
}

void ctest::Log::SetLogLevel(ctest::LogLevel nLogLevel) {

}

ctest::LogFileInfo ctest::Log::setLogFileInfo(const char *szFile, int nLine) {
    return ctest::LogFileInfo(nullptr, 0);
}
//直到调用getInstance()时才会创建对象
std::shared_ptr<Log> Log::getInstance() {
    //双重检查锁
    if(log == nullptr){
        log_mutex.lock();
        if(log == nullptr){
            //需要使用智能指针解决释放问题
            log = std::make_shared<Log>();
        }
        log_mutex.unlock();
    }
    return log;
}


}