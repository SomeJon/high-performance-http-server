#pragma once

#include <winsock2.h>
#include <string>
#include <ctime>
#include "http/HttpRequest.h" // Include the HttpRequest class definition

// Defines all possible states a socket can be in.
enum class SocketStatus {
    EMPTY,
    LISTENING,
    RECEIVING,
    PROCESSING,
    SENDING
};

const int BUFFER_SIZE = 1024;

// Holds all state information for a single socket connection.
struct SocketState
{
    SOCKET id = 0;
    SocketStatus status = SocketStatus::EMPTY;

    // Buffers and tracking for network I/O
    char buffer[BUFFER_SIZE];
    std::string messageData; // Accumulates the full request/response string
    int bytesSent = 0;
    int bytesToSend = 0;

    // Timeout tracking
    time_t lastActivityTime = 0;

    // The parsed request object associated with this connection.
    HttpRequest request;
};

