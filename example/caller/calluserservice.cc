#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "logger.h"

int main(int argc, char **argv)
{
    //整个程序启动以后，先调用框架的初始化函数,只初始化一次
    MprpcApplication::Init(argc, argv);

    //演示调用远程发布的rpc方法Login
    usermodel::UserServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    usermodel::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc方法的响应参数
    usermodel::LoginResponse response;

    //发起rpc方法的调用 同步的rpc调用过程 MprpcChannel::callmethod
    MprpcController controller;
    stub.Login(&controller, &request, &response, nullptr); // RpcChannel::callMethod集中来做所有rpc方法调用的参数序列化和网络发送

    //一次rpc调用完成
    if (controller.Failed())
    {
        // std::cout << controller.ErrorText() << std::endl;
        LOG_ERR("call rpc faild, erro message:%s",controller.ErrorText().c_str());
    }
    else
    {
        if (0 == response.result().errcode())
        {
            // std::cout << "rpc login response success:" << response.success() << std::endl;
            LOG_INFO("rpc login response success:%d", response.success());
        }
        else
        {
            //std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
            LOG_ERR( "rpc login response error:%s", response.result().errmsg().c_str());
        }
    }

    //演示调用远程发布的rpc方法Register
    usermodel::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("66666");
    usermodel::RegisterResponse rsp;

    stub.Register(nullptr, &req, &rsp, nullptr);
    if (controller.Failed())
    {
        //std::cout << controller.ErrorText() << std::endl;
        LOG_ERR("call rpc faild, erro message:%s",controller.ErrorText().c_str());
    }
    else
    {
        if (0 == rsp.result().errcode())
        {
            //std::cout << "rpc register response success:" << rsp.success() << std::endl;
            LOG_INFO("rpc register response success:%d",  rsp.success());
        }
        else
        {
            //std::cout << "rpc register response error:" << rsp.result().errmsg() << std::endl;
            LOG_ERR("rpc register response error:%s", response.result().errmsg().c_str());
        }
    }

    return 0;
}