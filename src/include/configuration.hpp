#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <exception>

#include <libconfig.h++>

class ConfigurationException : public std::exception {
public:
    explicit ConfigurationException(const std::string& message)
        : message_(message) {}

    virtual const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

class Configuration {
public:

    struct ConfigBase {
        std::string worksheet;
        std::string cell;
    };

    struct VersionConfig : public ConfigBase
    {
        std::string signal;
    };

    struct BlocksConfig : public ConfigBase
    {
        int parent_column;
    };

    struct FieldsConfig : public ConfigBase
    {
        int block_column;
        int descr_column;
        char text_delimiter;
    };

    VersionConfig version;
    ConfigBase header;
    BlocksConfig blocks;
    FieldsConfig fields;
    std::unordered_map<std::string, std::string> missing;
    std::unordered_map<std::string, std::string> keys;

    void readConfig(const std::filesystem::path& filename);

private:

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

};
