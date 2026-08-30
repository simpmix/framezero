#ifndef FRAMEZERO_REPLAY_SYSTEM_H
#define FRAMEZERO_REPLAY_SYSTEM_H

#include "input.h"
#include <fstream>
#include <vector>
#include <cstring>

namespace FrameZero {

struct ReplayHeader {
    uint32_t magic;          // 'FRZ0'
    uint32_t version;
    uint32_t frameCount;
    uint32_t playerCount;
    uint64_t timestamp;
};

class ReplaySystem {
public:
    static constexpr uint32_t REPLAY_MAGIC = 0x46525A30;  // 'FRZ0'
    static constexpr uint32_t REPLAY_VERSION = 1;
    
    struct ReplayFrame {
        Input inputs[4];  // Support up to 4 players
        uint32_t stateChecksum = 0; // Checksum for the state AT THIS FRAME
    };
    
    std::vector<ReplayFrame> frames;
    uint32_t playerCount;
    
    ReplaySystem() : playerCount(0) {}
    
    void startRecording(uint32_t players) {
        frames.clear();
        playerCount = players;
    }
    
    void recordFrame(Input* playerInputs, uint32_t count, uint32_t checksum = 0) {
        ReplayFrame frame;
        count = (count > 4) ? 4 : count;
        
        for (uint32_t i = 0; i < count; i++) {
            frame.inputs[i] = playerInputs[i];
        }
        for (uint32_t i = count; i < 4; i++) {
            frame.inputs[i] = Input();  // Zero out unused
        }
        
        frame.stateChecksum = checksum;
        frames.push_back(frame);
    }
    
    bool saveToFile(const char* filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        ReplayHeader header;
        header.magic = REPLAY_MAGIC;
        header.version = REPLAY_VERSION;
        header.frameCount = static_cast<uint32_t>(frames.size());
        header.playerCount = playerCount;
        header.timestamp = 0;  // Could use real timestamp
        
        file.write(reinterpret_cast<const char*>(&header), sizeof(ReplayHeader));
        
        if (!frames.empty()) {
            file.write(reinterpret_cast<const char*>(frames.data()), 
                      sizeof(ReplayFrame) * frames.size());
        }
        
        file.close();
        return true;
    }
    
    bool loadFromFile(const char* filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        ReplayHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(ReplayHeader));
        
        if (header.magic != REPLAY_MAGIC) {
            file.close();
            return false;
        }
        
        playerCount = header.playerCount;
        frames.resize(header.frameCount);
        
        if (header.frameCount > 0) {
            file.read(reinterpret_cast<char*>(frames.data()), 
                     sizeof(ReplayFrame) * header.frameCount);
        }
        
        file.close();
        return true;
    }
    
    Input getFrameInput(uint32_t frameIndex, uint32_t playerIndex) const {
        if (frameIndex >= frames.size() || playerIndex >= 4) {
            return Input();
        }
        return frames[frameIndex].inputs[playerIndex];
    }
    
    uint32_t getChecksum(uint32_t frameIndex) const {
        if (frameIndex >= frames.size()) return 0;
        return frames[frameIndex].stateChecksum;
    }
    
    uint32_t getFrameCount() const { return static_cast<uint32_t>(frames.size()); }
    
    void clear() {
        frames.clear();
        playerCount = 0;
    }
    
    // Verify determinism by comparing two replays
    bool verifyDeterminism(const ReplaySystem& other) const {
        if (frames.size() != other.frames.size()) return false;
        if (playerCount != other.playerCount) return false;
        
        for (size_t i = 0; i < frames.size(); i++) {
            for (uint32_t p = 0; p < playerCount; p++) {
                if (frames[i].inputs[p] != other.frames[i].inputs[p]) {
                    return false;
                }
            }
        }
        return true;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_REPLAY_SYSTEM_H
