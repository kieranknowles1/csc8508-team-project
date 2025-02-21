#include "Config.h"

#include <fstream>
#include <iostream>

namespace NCL::CSC8503 {
    Config::Config(std::filesystem::path userPath, std::filesystem::path defaultsPath)
    {
        userConfig = parse(userPath);
        defaultConfig = parse(defaultsPath);
    }

    nlohmann::json Config::parse(std::filesystem::path path)
    {
        std::ifstream file(path);
        if (!file.good()) {
            // The user may not have their own config file
            std::cerr << "Failed to open file " << path << std::endl;
            return nlohmann::json();
        }

        return nlohmann::json::parse(
            file,
            /* Callback */ nullptr,
            // We want to always be able to start up, even if something goes wrong
            // Plus, PS5 doesn't support exceptions so they'll cause an abort
            /* Throw exceptions */ false,
            /* Ignore comments */ true
        );
    }
}
