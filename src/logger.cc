#include "logger.h"
#include <time.h>
#include <iostream>

//获取日志单例
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    //启动专门的写日志线程
    std::thread writeLogTask([&]()
    {
        for(;;)
        {
            //获取当前的日期，然后获取日志信息，写入相应的日志文件中,a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon +1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if(nullptr == pf)
            {
                //std::cout <<"logger file:" << file_name << " open error!" <<std::endl;
                LOG_ERR("logger file:%s open error!", file_name);
                exit(EXIT_FAILURE);
            }

            std::string msg = m_logQue.Pop();

            //添加时分秒信息
            char time_buf[128];
            sprintf(time_buf,"%d:%d:%d=>[%s]",
                        nowtm->tm_hour, 
                        nowtm->tm_min, 
                        nowtm->tm_sec,
                        (m_logLevel == INFO?"info":"error"));

            msg.insert(0,time_buf);
            msg.append("\n");
            
            // 写入磁盘
            fputs(msg.c_str(),pf);
            fclose(pf);
        }
    });
    //设置线程分离
    writeLogTask.detach();
}

//设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
}

//写日志,将日志信息写入logqueue缓冲区中
void Logger::Log(std::string msg)
{
    m_logQue.Push(msg);
}