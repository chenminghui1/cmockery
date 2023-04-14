//
// Created by oslab on 23-4-14.
//

#ifndef CMOCKERY_CTEST_LOG_H
#define CMOCKERY_CTEST_LOG_H

#include <cstdio>

namespace ctest {
    class Log final {
    public:
             Log(const Log&) = delete;
             Log& operator=(const Log&) = delete;

Log(const char* szPath = nullptr);
          ~Log();

         private:
             bool    __Open(const char* szPath);
             int     __Close();
           bool    __Write(const char* szTime, const char* szFile, const char* szLine, const char* szType, const char* szMsg);
             //int     __Lock(bool isLock);

         private:
             friend class LogMgr;
            friend class LogFileInfo;
             FILE* _pFile = nullptr;
    };
class LogMgr final {
    public:
        LogMgr();
        ~LogMgr();

        bool Init(const char* szPath);
        void UnInit();

        Log* GetLog();

    private:
        Log* _pLog = nullptr;
    };
class LogFileInfo final {
    public:
        LogFileInfo(const char* szFile, int nLine);
        ~LogFileInfo();

        const char* GetFile() const;
        int GetLine() const;

    private:
        const char* _szFile = nullptr;
        int _nLine = 0;
    };
}
#endif //CMOCKERY_CTEST_LOG_H
