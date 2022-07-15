#include "mprpcapplication.h"
#include<iostream>
#include <unistd.h>

MprpcConfig MprpcApplication::m_config;

// 单例
MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}


void ShowArgsHelp()
{
    std::cout<< "format: command -i <configfile>" <<std::endl;
}

void MprpcApplication::Init(int argc, char **argv)
{
    // 判断输入参数是否合法
    if(argc<2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    
    int c = 0;
    std::string config_file;
    while((c = getopt(argc,argv,"i:"))!=-1)
    {
        switch(c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':' :
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    //开始加载配置文件
    m_config.LoadConfigFile(config_file.c_str());
    
    // 调试信息
    // std::cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<std::endl;
    // std::cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<std::endl;
    // std::cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<std::endl;
    // std::cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<std::endl;
}


 MprpcConfig& MprpcApplication::GetMprpcConfig()
 {
    return m_config;
 }