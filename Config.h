#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

class Config {
public:
    void load(const std::string &filename) {
        std::ifstream file(filename);
        std::string line;
        std::string currentSection;

        while (std::getline(file, line)) {
            // Remove whitespace from the beginning and end of the line
            line = trim(line);

            // Skip comments and empty lines
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }

            // Handle section headers
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.size() - 2);
                continue;
            }

            // Parse key-value pairs
            auto delimiterPos = line.find('=');
            std::string key = trim(line.substr(0, delimiterPos));
            std::string value = trim(line.substr(delimiterPos + 1));

            // Store the key-value pair in the map
            if (!currentSection.empty()) {
                configData[currentSection + "." + key] = value;
            } else {
                configData[key] = value;
            }
        }
    }

    std::string get(const std::string &key, const std::string &defaultValue = "") const {
        auto it = configData.find(key);
        if (it != configData.end()) {
            return it->second;
        }
        return defaultValue;
    }

private:
    std::map<std::string, std::string> configData;

    static std::string trim(const std::string &str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
            return str;
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
};