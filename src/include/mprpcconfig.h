#pragma once
#include<unordered_map>

// 框架读取配置文件类
class MprpcConfig
{
public:
    //负责加载配置文件
    void LoadConfigFile(const char* config_file);
    
    //查询配置项信息
    std::string Load(const std::string &key);

private:
    // 去掉字符串中的空格
    void Trim(std::string &str);

    // 配置信息表,将配置信息记录在表中
    std::unordered_map<std::string,std::string> m_configMap;
};