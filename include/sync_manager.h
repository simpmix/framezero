#pragma once
#include <chrono>
#include "network_socket.h"
#include "vector2.h"

namespace FrameZero {

// Synchronizes the starting frame between two computers to ensure lockstep rollback
class SyncManager {
public:
    enum State { IDLE, SYNCING, RUNNING };
    
private:
    State currentState;
    int localPingCount;
    int remotePongCount;
    
    // RTT (Round Trip Time) in milliseconds
    long long averageRTT;
    long long pings[10];
    
    std::chrono::steady_clock::time_point syncStartTime;

public:
    SyncManager() : currentState(IDLE), localPingCount(0), remotePongCount(0), averageRTT(0) {
        std::memset(pings, 0, sizeof(pings));
    }

    State getState() const { return currentState; }

    void startSync() {
        currentState = SYNCING;
        localPingCount = 0;
        remotePongCount = 0;
        syncStartTime = std::chrono::steady_clock::now();
    }

    // Call this in your main loop to exchange UDP ping packets
    void update(UDPSocket& socket, const char* remoteIp, uint16_t remotePort) {
        if (currentState != SYNCING) return;
        
        auto now = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - syncStartTime).count();
        
        // Send a ping every 50ms until we have 10 good pongs
        if (localPingCount < 10 && (elapsedMs > localPingCount * 50)) {
            uint8_t packet[1] = { 0xFF }; // 0xFF designates a Ping packet
            socket.send(remoteIp, remotePort, packet, 1);
            localPingCount++;
        }
        
        if (remotePongCount >= 10) {
            currentState = RUNNING;
            // Calculate Frame Advantage (Assuming 60fps, meaning 16.6ms per frame)
            // If RTT is 60ms, the remote player is ~2 frames behind us.
            // We delay our local execution by (RTT / 2) to maintain fairness!
        }
    }
    
    void receivePong(long long rttValue) {
        if (currentState == SYNCING && remotePongCount < 10) {
            pings[remotePongCount] = rttValue;
            remotePongCount++;
            
            if (remotePongCount == 10) {
                long long sum = 0;
                for (int i = 0; i < 10; i++) sum += pings[i];
                averageRTT = sum / 10;
            }
        }
    }
    
    bool isReady() const {
        return currentState == RUNNING;
    }
    
    long long getAverageRTT() const {
        return averageRTT;
    }
    
    int getRecommendedFrameDelay() const {
        // Half of RTT converted to frames (16.6ms per frame)
        long long oneWayLatency = averageRTT / 2;
        return static_cast<int>(oneWayLatency / 16);
    }
};

} // namespace FrameZero
