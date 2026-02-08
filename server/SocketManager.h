#pragma once

#include <vector>
#include "SocketData.h"

const int HTTP_PORT = 8080;
const int LISTEN_BACKLOG = 5;
const int SOCKET_TIMEOUT_SECONDS = 120; // 2 minutes

class SocketManager
{
public:
    // This constant is now part of the class, making it accessible from outside.
    static constexpr int MAX_SOCKETS = 60;

    SocketManager();
    ~SocketManager();

    bool init();
    void buildFdSets(fd_set& waitRecv, fd_set& waitSend);
    bool acceptNewConnection(int listenerSocketIndex);
    int receiveData(int socketIndex);
    int sendData(int socketIndex);
    void removeSocket(int socketIndex);
    void checkTimeouts();

    SocketState& getSocketState(int socketIndex);
    const std::vector<SocketState>& getSockets() const;

private:
    bool addSocket(SOCKET id, SocketStatus status);

    std::vector<SocketState> sockets;
    int activeSocketsCount;
};

