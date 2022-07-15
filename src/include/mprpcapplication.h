#pragma onece
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"


//mprpc框架的基础类，负责框架的初始化操作
class MprpcApplication
{
public:
    // 框架初始化
    static void Init(int argc,char ** argv);
    // 获取框架对象,单例模式
    static MprpcApplication& GetInstance();
    // 获取框架配置信息对象
    static MprpcConfig& GetMprpcConfig();
private:
    // 配置信息对象
    static MprpcConfig m_config;
    
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication& )=delete;
    MprpcApplication(MprpcApplication &&  ) = delete;
};