#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "SocketManager.h"
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

SocketManager::SocketManager() : activeSocketsCount(0)
{
	sockets.resize(MAX_SOCKETS);
}

SocketManager::~SocketManager()
{
	for (int i = 0; i < MAX_SOCKETS; ++i)
	{
		if (sockets[i].status != SocketStatus::EMPTY)
		{
			closesocket(sockets[i].id);
		}
	}
	WSACleanup();
}

bool SocketManager::init()
{
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << "Server: Error at WSAStartup()\n";
		return false;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "Server: Error at socket(): " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(HTTP_PORT);

	if (bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR)
	{
		std::cout << "Server: Error at bind(): " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	if (listen(listenSocket, LISTEN_BACKLOG) == SOCKET_ERROR)
	{
		std::cout << "Server: Error at listen(): " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	if (!addSocket(listenSocket, SocketStatus::LISTENING))
	{
		std::cout << "Server: Failed to add listening socket." << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	std::cout << "Server is listening on port " << HTTP_PORT << std::endl;
	return true;
}

void SocketManager::buildFdSets(fd_set& waitRecv, fd_set& waitSend)
{
	FD_ZERO(&waitRecv);
	FD_ZERO(&waitSend);

	for (int i = 0; i < MAX_SOCKETS; ++i)
	{
		if (sockets[i].status == SocketStatus::LISTENING || sockets[i].status == SocketStatus::RECEIVING) {
			FD_SET(sockets[i].id, &waitRecv);
		}
		else if (sockets[i].status == SocketStatus::SENDING)
		{
			FD_SET(sockets[i].id, &waitSend);
		}
	}
}

bool SocketManager::acceptNewConnection(int listenerSocketIndex)
{
	sockaddr_in from;
	int fromLen = sizeof(from);
	SOCKET newSocket = accept(sockets[listenerSocketIndex].id, (SOCKADDR*)&from, &fromLen);

	if (newSocket == INVALID_SOCKET)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			std::cout << "Server: Error at accept(): " << WSAGetLastError() << std::endl;
		}
		return false;
	}

	std::cout << "Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << std::endl;

	if (!addSocket(newSocket, SocketStatus::RECEIVING))
	{
		std::cout << "\t\tToo many connections, dropped client." << std::endl;
		closesocket(newSocket);
	}
	return true;
}

int SocketManager::receiveData(int socketIndex)
{
	SocketState& socket = sockets[socketIndex];

	int bytesRead = recv(socket.id, socket.buffer, BUFFER_SIZE, 0);

	if (bytesRead == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			std::cout << "Server: Error at recv(): " << WSAGetLastError() << std::endl;
			removeSocket(socketIndex);
		}
		return SOCKET_ERROR;
	}

	if (bytesRead == 0)
	{
		removeSocket(socketIndex);
		return 0;
	}

	// Append received data from the temporary char buffer to the main message string.
	socket.messageData.append(socket.buffer, bytesRead);
	socket.lastActivityTime = time(nullptr);

	return bytesRead;
}

int SocketManager::sendData(int socketIndex)
{
	SocketState& socket = sockets[socketIndex];
	int bytesRemaining = socket.bytesToSend - socket.bytesSent;

	if (bytesRemaining <= 0)
	{
		return 0;
	}

	// Send data directly from the messageData string, using an offset for partial sends.
	const char* dataToSend = socket.messageData.c_str();
	int bytesSent = send(socket.id, dataToSend + socket.bytesSent, bytesRemaining, 0);

	if (bytesSent == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			std::cout << "Server: Error at send(): " << WSAGetLastError() << std::endl;
			removeSocket(socketIndex);
		}
		return SOCKET_ERROR;
	}

	socket.bytesSent += bytesSent;
	socket.lastActivityTime = time(nullptr);

	// If all data has been sent, reset the state for the next request.
	if (socket.bytesSent >= socket.bytesToSend)
	{
		socket.bytesSent = 0;
		socket.bytesToSend = 0;
		socket.messageData.clear();
		socket.status = SocketStatus::RECEIVING;
	}

	return bytesSent;
}

void SocketManager::removeSocket(int socketIndex)
{
	if (socketIndex < 0 || socketIndex >= MAX_SOCKETS || sockets[socketIndex].status == SocketStatus::EMPTY)
	{
		return;
	}

	std::cout << "Server: Closing connection for socket " << sockets[socketIndex].id << std::endl;

	closesocket(sockets[socketIndex].id);
	sockets[socketIndex].status = SocketStatus::EMPTY;
	sockets[socketIndex].id = 0;
	activeSocketsCount--;
}

void SocketManager::checkTimeouts()
{
	time_t currentTime = time(nullptr);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].status != SocketStatus::EMPTY && sockets[i].status != SocketStatus::LISTENING)
		{
			if (difftime(currentTime, sockets[i].lastActivityTime) > SOCKET_TIMEOUT_SECONDS)
			{
				std::cout << "Server: Socket " << sockets[i].id << " timed out." << std::endl;
				removeSocket(i);
			}
		}
	}
}

SocketState& SocketManager::getSocketState(int socketIndex)
{
	return sockets[socketIndex];
}

const std::vector<SocketState>& SocketManager::getSockets() const
{
	return sockets;
}

bool SocketManager::addSocket(SOCKET id, SocketStatus status)
{
	// For the listening socket, we want to ensure it gets the first slot.
	if (status == SocketStatus::LISTENING) {
		if (sockets[0].status == SocketStatus::EMPTY) {
			sockets[0].id = id;
			sockets[0].status = status;
			sockets[0].lastActivityTime = time(nullptr);
			activeSocketsCount++;
			return true;
		}
		else {
			return false;
		}
	}

	if (activeSocketsCount >= MAX_SOCKETS)
	{
		return false;
	}

	for (int i = 1; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].status == SocketStatus::EMPTY)
		{
			sockets[i].id = id;
			sockets[i].status = status;
			sockets[i].messageData.clear();
			sockets[i].bytesSent = 0;
			sockets[i].bytesToSend = 0;
			sockets[i].lastActivityTime = time(nullptr);

			unsigned long flag = 1;
			if (ioctlsocket(id, FIONBIO, &flag) != 0)
			{
				std::cout << "Server: Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
				closesocket(id);
				sockets[i].status = SocketStatus::EMPTY;
				return false;
			}

			activeSocketsCount++;
			return true;
		}
	}

	return false;
}

