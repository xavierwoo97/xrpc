# 简介
    本项目使用muduo网络库，protobuf以及zookeeper搭建了一个分布式网络通信框架。使用C++队列容器，配合互斥锁和信号量实现异步写日志队列。autobuild.sh为一件编译脚本。本项目基于以下测试环境:
    * Ubuntu20.04
    * gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
    * cmake version 3.16.3
    * muduo-2.0.2
    * apache-zookeeper-3.7.1
    * Protocol Buffers v21.2
# 使用
    * 使用时包含lib下的共享库文件即可。
    * example中为两个示例,test为测试文件夹

# 参考资料
    本项目参考施磊腾讯课程《实现分布式网络通信框架-rpc通信原理》 


