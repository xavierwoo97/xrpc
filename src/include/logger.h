#pragma once
#include "logqueue.h"
#include <string>


enum LogLevel
{
    INFO, // 普通信息
    ERROR, // 错误信息
};

//Mprpc 框架提供的日志系统
class Logger 
{
public:
    //获取日志单例
    static Logger& GetInstance();
    //设置日志级别
    void SetLogLevel(LogLevel level);
    //将日志写入日志队列缓冲区
    void Log(std::string msg);

private:
    int m_logLevel; //记录日志级别
    LogQue<std::string> m_logQue; // 日志缓冲队列
    Logger(); //构造对象，并将日志写入磁盘
    Logger(const Logger&) = delete;
    Logger(Logger&&)= delete;
};

//定义日志宏,可变参, LOG_INFO("xxx %d %d",20,"xxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(INFO);\
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c);\
    }while(0)

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(ERROR);\
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c);\
    }while(0)
