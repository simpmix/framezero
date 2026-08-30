#pragma once
#include <queue>
#include <vector>
#include <random>
#include <chrono>
#include "network_socket.h"

namespace FrameZero {

struct Packet {
    std::vector<uint8_t> data;
    std::string ip;
    uint16_t port;
    std::chrono::steady_clock::time_point deliveryTime;
};

// Wraps UDPSocket to artificially simulate bad network conditions for Rollback testing.
class NetworkSimulator {
private:
    UDPSocket socket;
    std::queue<Packet> inboundQueue;
    std::queue<Packet> outboundQueue;
    
    int latencyMs;
    int jitterMs;
    float packetLossPercent; // 0.0 to 1.0

    std::mt19937 rng;
    std::uniform_real_distribution<float> lossDist;
    std::uniform_int_distribution<int> jitterDist;

public:
    NetworkSimulator(int latencyMs = 50, int jitterMs = 10, float packetLoss = 0.05f) 
        : latencyMs(latencyMs), jitterMs(jitterMs), packetLossPercent(packetLoss),
          rng(std::random_device{}()), lossDist(0.0f, 1.0f) {
        if (jitterMs > 0) {
            jitterDist = std::uniform_int_distribution<int>(-jitterMs, jitterMs);
        }
    }

    bool bind(uint16_t port) {
        return socket.bind(port);
    }

    bool send(const char* ip, uint16_t port, const uint8_t* data, size_t size) {
        // Packet Loss simulation
        if (lossDist(rng) < packetLossPercent) {
            return true; // Pretend we sent it, but drop it into the void!
        }

        // Latency + Jitter simulation
        int currentJitter = (jitterMs > 0) ? jitterDist(rng) : 0;
        int delay = latencyMs + currentJitter;
        if (delay < 0) delay = 0;

        Packet p;
        p.data.assign(data, data + size);
        p.ip = ip;
        p.port = port;
        p.deliveryTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
        
        outboundQueue.push(p);
        return true;
    }

    void update() {
        auto now = std::chrono::steady_clock::now();
        
        // Process outbound packets that have reached their delivery time
        while (!outboundQueue.empty() && outboundQueue.front().deliveryTime <= now) {
            Packet p = outboundQueue.front();
            outboundQueue.pop();
            socket.send(p.ip.c_str(), p.port, p.data.data(), p.data.size());
        }
    }

    int receive(uint8_t* data, size_t maxSize, std::string& outIp, uint16_t& outPort) {
        // Read everything from the real socket and queue it with delay
        uint8_t temp[1024];
        std::string tempIp;
        uint16_t tempPort;
        
        while (true) {
            int bytes = socket.receive(temp, sizeof(temp), tempIp, tempPort);
            if (bytes <= 0) break;

            if (lossDist(rng) < packetLossPercent) continue; // Drop incoming packet

            int currentJitter = (jitterMs > 0) ? jitterDist(rng) : 0;
            int delay = latencyMs + currentJitter;
            if (delay < 0) delay = 0;

            Packet p;
            p.data.assign(temp, temp + bytes);
            p.ip = tempIp;
            p.port = tempPort;
            p.deliveryTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
            inboundQueue.push(p);
        }

        auto now = std::chrono::steady_clock::now();
        if (!inboundQueue.empty() && inboundQueue.front().deliveryTime <= now) {
            Packet p = inboundQueue.front();
            inboundQueue.pop();
            
            size_t copySize = (p.data.size() < maxSize) ? p.data.size() : maxSize;
            memcpy(data, p.data.data(), copySize);
            outIp = p.ip;
            outPort = p.port;
            return (int)copySize;
        }

        return -1; // EWOULDBLOCK
    }
};

} // namespace FrameZero
