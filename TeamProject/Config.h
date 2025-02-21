#pragma once

#include <filesystem>
#include <string_view>

#include <nlohmann/json.hpp>

namespace NCL::CSC8503 {
    // Very basic config class. Can store anything that can be parsed to JSON.
    // Uses two files:
    // - default-config.jsonc: Default config values. Not intended to be modified by users
    //                         and will be tracked by Git
    // - user-config.jsonc:    User config values. Not tracked by Git. Takes priority over defaults
    class Config {
    public:
        Config(std::filesystem::path userPath, std::filesystem::path defaultsPath);

        // Get a value from the config file. If a parser is not available, implement an
        // overload of `from_json` for it.
        // TODO: Graceful handling of an invalid value. Currently causes a CTD
        template<typename T>
        T get(std::string_view key) const {
            return userConfig.contains(key)
                ? userConfig[key]
                : defaultConfig[key];
        }

        private:
        nlohmann::json parse(std::filesystem::path path);

        nlohmann::json userConfig;
        nlohmann::json defaultConfig;
    };
}
