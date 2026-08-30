#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace FrameZero {

// A cross-platform Asset Manager that standardizes file paths (Linux/macOS vs Windows)
// and prevents loading duplicate textures or sounds into VRAM.
class AssetManager {
private:
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Sound> sounds;

    // Normalizes file paths to ensure compatibility across Windows and POSIX
    std::string normalizePath(std::string path) {
        // Convert Windows backslashes to forward slashes for cross-platform safety
        std::replace(path.begin(), path.end(), '\\', '/');
        // Convert to lowercase to avoid case-sensitivity issues on Linux (ext4) vs Windows (NTFS)
        std::transform(path.begin(), path.end(), path.begin(), ::tolower);
        return path;
    }

public:
    ~AssetManager() {
        clear();
    }

    Texture2D getTexture(std::string path) {
        std::string key = normalizePath(path);
        
        // If it already exists, return the cached version (saves VRAM)
        if (textures.find(key) != textures.end()) {
            return textures[key];
        }
        
        // Otherwise load it from disk
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id == 0) {
            std::cerr << "[AssetManager] ERROR: Failed to load texture on OS: " << path << "\n";
        } else {
            textures[key] = tex;
        }
        return tex;
    }

    Sound getSound(std::string path) {
        std::string key = normalizePath(path);
        
        if (sounds.find(key) != sounds.end()) {
            return sounds[key];
        }
        
        Sound snd = LoadSound(path.c_str());
        if (snd.stream.buffer == nullptr) {
            std::cerr << "[AssetManager] ERROR: Failed to load sound on OS: " << path << "\n";
        } else {
            sounds[key] = snd;
        }
        return snd;
    }

    // Unload everything when shutting down or switching levels
    void clear() {
        for (auto& pair : textures) {
            UnloadTexture(pair.second);
        }
        textures.clear();

        for (auto& pair : sounds) {
            UnloadSound(pair.second);
        }
        sounds.clear();
    }
};

} // namespace FrameZero
