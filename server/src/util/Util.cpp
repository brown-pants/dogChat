#include "Util.h"
#include <fstream>
#include <iostream>
#include <boost/beast/core/detail/base64.hpp>

Util::Util() {}

nlohmann::json Util::ReadJson(const std::string &json_file)
{
    try {
        std::ifstream file(json_file);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open " + json_file);
        }
        nlohmann::json json_data = nlohmann::json::parse(file);
        return json_data;
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n" << "Byte position: " << e.byte << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return nullptr;
}

bool Util::CreatePath(const std::string &path)
{
    if (!std::filesystem::exists(path)) 
    {
        return std::filesystem::create_directories(path);
    }
    return false;
}

bool Util::WriteToFile(const std::string &url, const std::vector<u_char> &data)
{
    CreatePath(std::filesystem::path(url).parent_path());
    
    std::ofstream outFile(url, std::ios::binary);
    if (!outFile) 
    {
        return false;
    }
    
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
    return outFile.good();
}

std::vector<u_char> Util::Base64ToBinary(const std::string &base64)
{
    const std::size_t decoded_size = boost::beast::detail::base64::decoded_size(base64.size());
    std::vector<u_char> binary(decoded_size);
    auto [written, _] = boost::beast::detail::base64::decode(binary.data(), reinterpret_cast<const char*>(base64.data()), base64.size());
    binary.resize(written);
    return binary;
}

std::string Util::FileToBase64(const std::string &url)
{
    std::ifstream inFile(url, std::ios::binary);
    if (!inFile) 
    {
        std::cout << url << " is not exist" << std::endl;
        return "";
    }
    std::vector<u_char> binary((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    const std::size_t encode_size = boost::beast::detail::base64::encoded_size(binary.size());
    std::string base64;
    base64.resize(encode_size);
    std::size_t written = boost::beast::detail::base64::encode(base64.data(), reinterpret_cast<const char*>(binary.data()), binary.size());
    base64.resize(written);
    return base64;
}