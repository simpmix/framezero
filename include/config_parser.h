#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

namespace FrameZero {

// A lightweight, cross-platform INI Configuration Parser
// Used for saving/loading user settings (Resolution, Keybinds, Volume, etc.)
class ConfigParser {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data;

    // Helper to trim whitespace
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

public:
    // Load config from file
    bool load(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        std::string line;
        std::string currentSection = "Default";

        while (std::getline(file, line)) {
            line = trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;

            // Check for section [SectionName]
            if (line[0] == '[' && line.back() == ']') {
                currentSection = trim(line.substr(1, line.length() - 2));
                continue;
            }

            // Parse key=value
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                data[currentSection][key] = value;
            }
        }
        return true;
    }

    // Save config back to file
    bool save(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) return false;

        for (const auto& sectionPair : data) {
            file << "[" << sectionPair.first << "]\n";
            for (const auto& keyPair : sectionPair.second) {
                file << keyPair.first << "=" << keyPair.second << "\n";
            }
            file << "\n";
        }
        return true;
    }

    // Getters
    std::string getString(const std::string& section, const std::string& key, const std::string& defaultVal = "") {
        if (data.count(section) && data[section].count(key)) {
            return data[section][key];
        }
        return defaultVal;
    }

    int getInt(const std::string& section, const std::string& key, int defaultVal = 0) {
        std::string val = getString(section, key);
        if (val.empty()) return defaultVal;
        try { return std::stoi(val); } catch (...) { return defaultVal; }
    }

    float getFloat(const std::string& section, const std::string& key, float defaultVal = 0.0f) {
        std::string val = getString(section, key);
        if (val.empty()) return defaultVal;
        try { return std::stof(val); } catch (...) { return defaultVal; }
    }
    
    bool getBool(const std::string& section, const std::string& key, bool defaultVal = false) {
        std::string val = getString(section, key);
        if (val.empty()) return defaultVal;
        if (val == "true" || val == "1" || val == "yes") return true;
        return false;
    }

    // Setters
    void setString(const std::string& section, const std::string& key, const std::string& value) {
        data[section][key] = value;
    }

    void setInt(const std::string& section, const std::string& key, int value) {
        data[section][key] = std::to_string(value);
    }

    void setFloat(const std::string& section, const std::string& key, float value) {
        data[section][key] = std::to_string(value);
    }
    
    void setBool(const std::string& section, const std::string& key, bool value) {
        data[section][key] = value ? "true" : "false";
    }
};

} // namespace FrameZero
