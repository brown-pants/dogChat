#include "Config.h"
#include "../util/Util.h"
#include <iostream>

std::string Config::m_mysql_address;
std::string Config::m_mysql_port;
std::string Config::m_mysql_user;
std::string Config::m_mysql_password;
std::string Config::m_server_port;
std::string Config::m_mysql_schema;

bool Config::Init()
{
    // 读取配置文件
    nlohmann::json json_data = Util::ReadJson("config/config.json");
    if (json_data == nullptr) 
    {
        return false;
    }

    // 获取配置数据
    try {
        m_mysql_address = json_data["mysql"]["address"];
        m_mysql_port = json_data["mysql"]["port"];
        m_mysql_user = json_data["mysql"]["user"];
        m_mysql_password = json_data["mysql"]["password"];
        m_mysql_schema = json_data["mysql"]["schema"];
        m_server_port = json_data["server"]["port"];
    }
    catch (const nlohmann::json::exception& e)
    {
        std::cerr << "JSON error: " << e.what() << std::endl;
        return false;
    }
    return true;
}