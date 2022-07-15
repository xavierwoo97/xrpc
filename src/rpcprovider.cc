#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
/*
service_name 对应一个service描述
一个service描述包含service* 记录服务对象
一个服务对象method_name对应一个method方法对象
*/

// 框架提供给外部使用的,可以发布rpc方法的函数接口，服务使用Service类指针
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    std::string service_name = pserviceDesc->name();
    //获取服务对象service的方法数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service_name:" << service_name << std::endl;
    LOG_INFO("service name:%s", service_name.c_str());

    for (int i = 0; i < methodCnt; ++i)
    {
        //获取服务对象指定下标的服务方法的描述(抽象描述) Userservice Login
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "method_name:" << method_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
    }

    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

//启动rpc服务节点,开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetMprpcConfig().Load("rpcserverip");
    uint16_t port = stoi(MprpcApplication::GetInstance().GetMprpcConfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip, port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    //绑定连接回调和消息处理回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
                                        std::placeholders::_2, std::placeholders::_3));
    //设置muduo库线程数量
    server.setThreadNum(4);


    // 把当前rpc节点要发布的服务注册到zk上，让rpc client可以从zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    //service_name为永久性节点,method_name为临时性节点
    for(auto &sp: m_serviceMap)
    {
        // service_name/method_name/rpc服务主机的ip+端口号
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp:sp.second.m_methodMap)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            // ZOO_EPHEMERAL 临时性节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    // std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    LOG_INFO("RpcProvider start service at ip:%s, port:%d",ip.c_str(),port);
    //启动网络服务
    server.start();
    m_eventLoop.loop();
}

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        //和rpc client连接已断开
        conn->shutdown();
    }
}

/*
在框架内部,RpcProvider和RpcConsumer协商好通信的protobuf数据类型
RpcProvider接收信息:service_name method_name arg_size(tcp粘包) ->封装成message header
PrcConsumer传送信息:header_size(4字节)+ header_str + args_str
使用string insert和copy处理数字,直接存储数字的二进制,而不是转化为字符串
*/

// 已建立连接用户读写事件回调,如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp time)
{
    //获取网络上接收的远程rpc调用请求字节流
    std::string recv_buf = buffer->retrieveAllAsString();

    //从字节流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    //根据header_size读取数据流的原始字节流，反序列化数据,获取rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败 
        // std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        LOG_ERR("rpc_header_str:%s parse error1!",rpc_header_str.c_str());
        return;
    }

    //获取rpc方法参数的字节流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    //打印调试信息
    // std::cout << "=========================================" << std::endl;
    // std::cout << "head_size:" << header_size << std::endl;
    // std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    // std::cout << "service_name:" << service_name << std::endl;
    // std::cout << "method_name:" << method_name << std::endl;
    // std::cout << "args_str:" << args_str << std::endl;
    // std::cout << "=========================================" << std::endl;

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // std::cout << service_name << " is not exist!" << std::endl;
        LOG_ERR("%s is not exist!",service_name.c_str());
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        //std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        LOG_ERR("%s:%s is not exist!",service_name.c_str(), method_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.m_service;      //获取service对象 new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; //获取method对象 Login

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        // std::cout << "request parse error, content:" << args_str << std::endl;
        LOG_ERR("request parse error, content:%s",args_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //给下面的method方法调用绑定一个Closure的回调函数
    google::protobuf::Closure * done = google::protobuf::NewCallback
                <RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>
                (this,&RpcProvider::SendRpcResponse,conn,response);
    
    //在框架上根据远程rpc请求,调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response,done);
}

// Closure的回调操作,用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr & conn, google::protobuf::Message *response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str)) //response进行序列化
    {   
        //序列化成功后,通过网络把rpc方法执行的结果发送回rpc调用方
        conn->send(response_str);
    }
    else
    {
        LOG_ERR("serialize response_str error!");
        // std::cout <<"serialize response_str error!" << std::endl;
    }
    conn->shutdown(); //模拟http的短链接服务,由rpcprovider主动断开连接
}