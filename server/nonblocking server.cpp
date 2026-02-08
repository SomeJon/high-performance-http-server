#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <iomanip>
#include <ctime>

#include "SocketManager.h"
#include "SocketData.h"
#include "http/HttpStatusCodes.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/IEndpoint.h"
#include "http/Endpoints.h"


IEndpoint* findEndpoint(const std::map<std::string, std::map<HttpMethod, IEndpoint*>>& routes, const HttpRequest& request)
{
    const std::string& path = request.getPath();
    const HttpMethod method = request.getMethod();
    const std::string fileRoutePrefix = "/file/";

    if (path.rfind(fileRoutePrefix, 0) == 0) {
        if (path.find('/', fileRoutePrefix.length()) != std::string::npos ||
            (path.length() < 5 || path.substr(path.length() - 4) != ".txt")) {
            return nullptr; // Invalid file path format.
        }

        if (routes.count(fileRoutePrefix)) {
            const std::map<HttpMethod, IEndpoint*>& methodMap = routes.at(fileRoutePrefix);
            if (methodMap.count(method)) {
                return methodMap.at(method);
            }
        }
    }

    if (routes.count(path)) {
        const std::map<HttpMethod, IEndpoint*>& methodMap = routes.at(path);
        if (methodMap.count(method)) {
            return methodMap.at(method);
        }
    }

    return nullptr;
}


int main()
{
    SocketManager manager;
    if (!manager.init()) {
        return 1;
    }

    // --- Controller Setup ---
    HomeEndpoint homeEndpoint;
    PostMessageEndpoint postMessageEndpoint;
    PutFileEndpoint putFileEndpoint;
    GetFileEndpoint getFileEndpoint;
    DeleteFileEndpoint deleteFileEndpoint;
    TraceEndpoint traceEndpoint;

    OptionsEndpoint homeOptions({{HttpMethod::GET, homeEndpoint.getDescription()}});
    OptionsEndpoint postMessageOptions({{HttpMethod::POST, postMessageEndpoint.getDescription()}});
    OptionsEndpoint traceOptions({{HttpMethod::TRACE, traceEndpoint.getDescription()}});
    OptionsEndpoint fileOptions({
        {HttpMethod::GET, getFileEndpoint.getDescription()},
        {HttpMethod::PUT, putFileEndpoint.getDescription()},
        {HttpMethod::DELETE_0, deleteFileEndpoint.getDescription()}
    });

    std::map<std::string, std::map<HttpMethod, IEndpoint*>> routes;

    routes["/home"][HttpMethod::GET] = &homeEndpoint;
    routes["/home"][HttpMethod::OPTIONS] = &homeOptions;
    routes["/postmessage"][HttpMethod::POST] = &postMessageEndpoint;
    routes["/postmessage"][HttpMethod::OPTIONS] = &postMessageOptions;
    routes["/trace"][HttpMethod::TRACE] = &traceEndpoint;
    routes["/trace"][HttpMethod::OPTIONS] = &traceOptions;

    routes["/file/"][HttpMethod::GET] = &getFileEndpoint;
    routes["/file/"][HttpMethod::PUT] = &putFileEndpoint;
    routes["/file/"][HttpMethod::DELETE_0] = &deleteFileEndpoint;
    routes["/file/"][HttpMethod::OPTIONS] = &fileOptions;


    while (true)
    {
        fd_set waitRecv, waitSend;
        manager.buildFdSets(waitRecv, waitSend);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int nfd = select(0, &waitRecv, &waitSend, NULL, &timeout);
        if (nfd == SOCKET_ERROR) {
            std::cout << "Server: Error at select(): " << WSAGetLastError() << std::endl;
            break;
        }

        // Handle incoming data and parse requests
        for (int i = 0; i < SocketManager::MAX_SOCKETS; i++) {
            SocketState& socket = manager.getSocketState(i);
            if (socket.status == SocketStatus::LISTENING && FD_ISSET(socket.id, &waitRecv)) {
                manager.acceptNewConnection(i);
            }
            else if (socket.status == SocketStatus::RECEIVING && FD_ISSET(socket.id, &waitRecv)) {
                if (manager.receiveData(i) != SOCKET_ERROR) {
                    ParseResult result = socket.request.parse(socket.messageData);
                    if (result == ParseResult::Success) {
                        socket.messageData.clear();
                        socket.status = SocketStatus::PROCESSING;
                    } else if (result == ParseResult::Error) {
                        HttpResponse response(HttpStatusCode::BadRequest);
                        socket.messageData = response.toString();
                        socket.bytesToSend = socket.messageData.length();
                        socket.status = SocketStatus::SENDING;
                    }
                }
            }
        }

        // Process complete requests
        for (int i = 0; i < SocketManager::MAX_SOCKETS; i++) {
            SocketState& socket = manager.getSocketState(i);
            if (socket.status == SocketStatus::PROCESSING) {
                const HttpRequest& originalRequest = socket.request;

                bool isHeadRequest = (originalRequest.getMethod() == HttpMethod::HEAD);

                HttpRequest routingRequest = originalRequest;
                if (isHeadRequest) {
                    routingRequest.setMethod(HttpMethod::GET);
                }

                IEndpoint* handler = findEndpoint(routes, routingRequest);
                HttpResponse response;

                if (handler) {
                    response = handler->handle(originalRequest);
                } else {
                    response = HttpResponse(HttpStatusCode::NotFound);
                }

                // Single-line logging
                auto now = std::chrono::system_clock::now();
                auto time_t_now = std::chrono::system_clock::to_time_t(now);
                std::tm tm_buf;
                localtime_s(&tm_buf, &time_t_now);

                std::cout << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] "
                          << httpMethodToString(originalRequest.getMethod()) << " " << originalRequest.getRawUrl()
                          << " -> " << static_cast<int>(response.getStatusCode()) << " "
                          << getReasonPhrase(response.getStatusCode()) << std::endl;

                // HEAD response generation
                std::string fullResponseStr = response.toString();
                if (isHeadRequest) {
                    size_t headersEnd = fullResponseStr.find("\r\n\r\n");
                    if (headersEnd != std::string::npos) {
                        socket.messageData = fullResponseStr.substr(0, headersEnd + 4);
                    } else {
                        socket.messageData = fullResponseStr;
                    }
                } else {
                    socket.messageData = fullResponseStr;
                }

                // Prepare socket for sending
                socket.bytesToSend = socket.messageData.length();
                socket.bytesSent = 0;
                socket.status = SocketStatus::SENDING;
            }
        }

        // Send responses
        for (int i = 0; i < SocketManager::MAX_SOCKETS; i++) {
            SocketState& socket = manager.getSocketState(i);
            if (socket.status == SocketStatus::SENDING && FD_ISSET(socket.id, &waitSend)) {
                manager.sendData(i);
            }
        }

        manager.checkTimeouts();
    }

    return 0;
}

