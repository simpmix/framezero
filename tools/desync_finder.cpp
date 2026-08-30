#define NOMINMAX
#include <FrameZero.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

using namespace FrameZero;

int main(int argc, char** argv) {
    std::cout << "--- FrameZero Desync Finder ---\n";
    
    if (argc < 3) {
        std::cout << "Usage: FrameZeroDesyncFinder <p1_replay.frz> <p2_replay.frz>\n";
        return 1;
    }
    
    const char* p1File = argv[1];
    const char* p2File = argv[2];
    
    ReplaySystem p1Replay;
    ReplaySystem p2Replay;
    
    if (!p1Replay.loadFromFile(p1File)) {
        std::cerr << "Failed to load " << p1File << "\n";
        return 1;
    }
    
    if (!p2Replay.loadFromFile(p2File)) {
        std::cerr << "Failed to load " << p2File << "\n";
        return 1;
    }
    
    std::cout << "Loaded " << p1File << " (" << p1Replay.getFrameCount() << " frames)\n";
    std::cout << "Loaded " << p2File << " (" << p2Replay.getFrameCount() << " frames)\n";
    
    int minFrames = std::min(p1Replay.getFrameCount(), p2Replay.getFrameCount());
    
    bool desyncFound = false;
    for (int i = 0; i < minFrames; ++i) {
        // Compare checksums
        if (p1Replay.getChecksum(i) != p2Replay.getChecksum(i)) {
            std::cout << "\n[!] DESYNC FOUND AT FRAME " << i << "\n";
            std::cout << "P1 Checksum: " << p1Replay.getChecksum(i) << "\n";
            std::cout << "P2 Checksum: " << p2Replay.getChecksum(i) << "\n";
            
            // Give some context
            std::cout << "\n--- Input History around Desync ---\n";
            for (int j = std::max(0, i - 5); j <= i; ++j) {
                std::cout << "Frame " << j << ": ";
                std::cout << "P1 (X:" << (int)p1Replay.getFrameInput(j, 0).moveX << ") ";
                std::cout << "P2 (X:" << (int)p1Replay.getFrameInput(j, 1).moveX << ") | ";
                
                std::cout << "Remote P1 (X:" << (int)p2Replay.getFrameInput(j, 0).moveX << ") ";
                std::cout << "Remote P2 (X:" << (int)p2Replay.getFrameInput(j, 1).moveX << ")\n";
            }
            
            desyncFound = true;
            break;
        }
    }
    
    if (!desyncFound) {
        std::cout << "\nNo desync found! Both simulations remained perfectly deterministic.\n";
    }
    
    return 0;
}
