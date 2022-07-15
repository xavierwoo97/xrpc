#include "mprpcconfig.h"
#include <iostream>
#include <fstream> //std::ifstream
#include "logger.h"

//负责解析配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    std::ifstream fstrm(config_file, std::ifstream::in);
    if (!fstrm.good())
    {
        // std::cout << config_file << " is not exist!" << std::endl;
        LOG_ERR("%s is not exist!", config_file);
        exit(EXIT_FAILURE);
    }
    std::string line;
    while (getline(fstrm, line))
    {
        // 跳过空行
        if (line.empty())
        {
            continue;
        }
        Trim(line);
        // 跳过注释行
        if (line[0] == '#')
        {
            continue;
        }
        int idx = line.find('=');
        if (-1 == idx)
        {
            continue;
        }
        std::string key = line.substr(0, idx);
        std::string value = line.substr(idx + 1, line.size() - idx);
        m_configMap.insert({key, value});
    }
    fstrm.close();
}

//查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return " ";
    }
    return it->second;
}

//去掉字符串中的空格
void MprpcConfig::Trim(std::string &str)
{
    //双指针
    int begin = 0, index = 0;
    while (index < str.size())
    {
        if(str[index]!=' ')
        {
            str[begin++] = str[index];
        }
        index++;
    }
}

