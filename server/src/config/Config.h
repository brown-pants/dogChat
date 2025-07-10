#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <iostream>

class Config
{
public:
    static bool Init();
    static inline const std::string &mysql_address() { return m_mysql_address; }
    static inline const std::string &mysql_port() { return m_mysql_port; }
    static inline const std::string &mysql_user() { return m_mysql_user; }
    static inline const std::string &mysql_password() { return m_mysql_password; }
    static inline const std::string &mysql_schema() { return m_mysql_schema; }
    static inline const std::string &server_port() { return m_server_port; }

private:
    Config() {}
    static std::string m_mysql_address;
    static std::string m_mysql_port;
    static std::string m_mysql_user;
    static std::string m_mysql_password;
    static std::string m_mysql_schema;
    static std::string m_server_port;
};

#endif