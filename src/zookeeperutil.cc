#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>
#include <unistd.h>
#include "logger.h"

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}


ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        // 关闭句柄，释放资源
        zookeeper_close(m_zhandle);
    }
}

// 全局的watcher观察器，zkserver给zkclient发送的通知
void global_watcher(zhandle_t *zh, int type,
                    int state,
                    const char *path,
                    void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) //与会话相关的回调消息类型
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient 与zkserver连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem); // 信号量资源+1
        }
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    // 加载IP地址和端口号
    std::string host = MprpcApplication::GetInstance().GetMprpcConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetMprpcConfig().Load("zookeeperport");
    std::string constr = host + ":" + port;

    /*
    zookeeper_mt 多线程版本，异步
    zookeeper客户端提供三个线程,API调用线程，网络I/O线程，watcher回调线程
    */
    m_zhandle = zookeeper_init(constr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) // 句柄创建失败
    {
        // std::cout << "zookeeper_init error!" << std::endl;
        LOG_ERR("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }

    // 给句柄创建信号量
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem); //当连接成功时，继续向下运行
    std::cout << "zookeeper_init success!" << std::endl;
}


// 在zkserver上根据指定的path创建znode节点,state=0永久性节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			//std::cout << "znode create success... path:" << path << std::endl;
            LOG_INFO("znode create success... path:%s", path);
		}
		else
		{
			// std::cout << "flag:" << flag << std::endl;
			// std::cout << "znode create error... path:" << path << std::endl;
            LOG_INFO("flag::%d", flag);
            LOG_ERR("znode create error... path:%s",path);
			exit(EXIT_FAILURE);
		}
	}
}


//获取参数指定的znode节点路径，或者znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64] = {0};
    int bufferlen = sizeof(buffer);
    // int flag = zoo_aget(m_zhandle, path, 0, getdata_completion, buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    sleep(1);
    if (flag != ZOK)
    {
        // std::cout << "get znode error...path:" << path << std::endl;
        LOG_ERR("get znode error...path:%s",path);
        return "";
    }
    else
    {
        // std::cout << path << " data:" << buffer << std::endl;
        LOG_INFO("%s data: %s",path,buffer);
        return buffer;
    }
}