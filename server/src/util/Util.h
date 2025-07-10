#ifndef __JSONUTIL_H__
#define __JSONUTIL_H__
#include <nlohmann/json.hpp>

class Util
{
public:
    static nlohmann::json ReadJson(const std::string &json_file);
    static bool WriteToFile(const std::string &url, const std::vector<u_char> &data);
    static bool CreatePath(const std::string &path);
    static std::vector<u_char> Base64ToBinary(const std::string &base64);
    static std::string FileToBase64(const std::string &url);

private:
    Util();
};

#endif