#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    bool load(const std::string& path = ".env") {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);

            // Trim (opcional)
            data_[key] = val;
        }
        return true;
    }

    std::string get(const std::string& key, const std::string& def = "") {
        if (data_.find(key) == data_.end()) return def;
        return data_[key];
    }

    int getInt(const std::string& key, int def = 0) {
        if (data_.find(key) == data_.end()) return def;
        return std::stoi(data_[key]);
    }

    float getFloat(const std::string& key, float def = 0.0f) {
        if (data_.find(key) == data_.end()) return def;
        return std::stof(data_[key]);
    }

private:
    Config() = default;
    std::map<std::string, std::string> data_;
};

#endif // CONFIG_HPP
