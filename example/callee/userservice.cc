#include <iostream>
#include "user.pb.h"
#include <string>
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

/*
UserService原本是一个本地服务，提供两个进程内方法,Login,GetFriendLists
*/
class UserService : public usermodel::UserServiceRpc //使用在rpc服务发布端(rpc服务提供者)
{
public:
    // 本地方法
    bool Login(std::string name, std::string pwd)
    {
        //模拟业务
        LOG_INFO("doing local service: Login");
        LOG_INFO("name %s pwd %s", name.c_str(),pwd.c_str());
        return false;
    }

    //本地业务
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        //模拟业务
        LOG_INFO("doing local service: Register");
        LOG_INFO("id %d name %s pwd %s", id, name.c_str(),pwd.c_str());
        return true;
    }

    /*重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1.caller调用Login(LoginRequest)方法,通过muduo网络库发送到callee
    2.callee接受Login(LoginRequest)调用请求,交给下面重写的Login方法
    */
    void Login(::google::protobuf::RpcController *controller,
               const ::usermodel::LoginRequest *request,
               ::usermodel::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        //框架给业务上报请求参数LoginRequest,应用获取相应的数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //做本地业务
        bool login_result = Login(name, pwd);

        //将响应写入LoginResponse
        usermodel::ResultCode *rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");
        response->set_success(login_result);

        //执行回调操作，即响应对象数据的序列化和网络发送(由框架完成)
        done->Run();
    }

    // 重写基类方法
    void Register(::google::protobuf::RpcController *controller,
                  const ::usermodel::RegisterRequest *request,
                  ::usermodel::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(id, name, pwd);

        usermodel::ResultCode *rc = response->mutable_result();
        rc ->set_errcode(0);
        rc ->set_errmsg("");
        response->set_success(register_result);
        done->Run();
    }
    
};

int main(int argc, char **argv)
{
    //日志测试
    /*
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d",__FILE__, __FUNCTION__, __LINE__);
    */
   
    //调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象,把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点,Run以后,进程进入阻塞状态,等待远程的rpc调用请求
    provider.Run();
    return 0;
}