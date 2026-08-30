#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
typedef int SOCKET;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
typedef struct sockaddr SOCKADDR;
#define closesocket close
#endif
#include <cstdint>
#include <string>
#include <iostream>

namespace FrameZero {

class UDPSocket {
public:
    UDPSocket() : sock(INVALID_SOCKET) {
#ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << "\n";
            return;
        }
#endif

        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
#ifdef _WIN32
            std::cerr << "socket failed: " << WSAGetLastError() << "\n";
#else
            std::cerr << "socket failed: " << errno << "\n";
#endif
            return;
        }

        // Make non-blocking
#ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR) {
            std::cerr << "ioctlsocket failed: " << WSAGetLastError() << "\n";
        }
#else
        if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
            std::cerr << "fcntl failed: " << errno << "\n";
        }
#endif
    }

    ~UDPSocket() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // Delete copy constructor and assignment operator to prevent multiple WSACleanup calls or double closures
    UDPSocket(const UDPSocket&) = delete;
    UDPSocket& operator=(const UDPSocket&) = delete;

    bool bind(uint16_t port) {
        if (sock == INVALID_SOCKET) return false;

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (::bind(sock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
            std::cerr << "bind failed: " << WSAGetLastError() << "\n";
#else
            std::cerr << "bind failed: " << errno << "\n";
#endif
            return false;
        }
        return true;
    }

    bool send(const char* ip, uint16_t port, const uint8_t* data, size_t size) {
        if (sock == INVALID_SOCKET) return false;

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
            std::cerr << "Invalid IP address\n";
            return false;
        }

        int sent = sendto(sock, reinterpret_cast<const char*>(data), static_cast<int>(size), 0, (SOCKADDR*)&addr, sizeof(addr));
        if (sent == SOCKET_ERROR) {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                std::cerr << "sendto failed: " << err << "\n";
            }
#else
            int err = errno;
            if (err != EWOULDBLOCK) {
                std::cerr << "sendto failed: " << err << "\n";
            }
#endif
            return false;
        }
        return true;
    }

    int receive(uint8_t* data, size_t maxSize, std::string& outIp, uint16_t& outPort) {
        if (sock == INVALID_SOCKET) return -1;

        sockaddr_in senderAddr = {};
#ifdef _WIN32
        int senderAddrSize = sizeof(senderAddr);
#else
        socklen_t senderAddrSize = sizeof(senderAddr);
#endif

        int received = recvfrom(sock, reinterpret_cast<char*>(data), static_cast<int>(maxSize), 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
        if (received == SOCKET_ERROR) {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                return 0; // Non-blocking, no data available
            }
            std::cerr << "recvfrom failed: " << err << "\n";
#else
            int err = errno;
            if (err == EWOULDBLOCK) {
                return 0; // Non-blocking, no data available
            }
            std::cerr << "recvfrom failed: " << err << "\n";
#endif
            return -1;
        }

        char ipStr[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr)) != nullptr) {
            outIp = ipStr;
        } else {
            outIp = "";
        }
        outPort = ntohs(senderAddr.sin_port);

        return received;
    }

    bool isValid() const {
        return sock != INVALID_SOCKET;
    }

private:
    SOCKET sock;
};

} // namespace FrameZero
