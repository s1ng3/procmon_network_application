// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

#ifndef SERVER_CLIENT_HPP
#define SERVER_CLIENT_HPP

#include <atomic>
#include <winsock2.h>
#include <string>
#include "ProcessInfo.hpp"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

void InitializeWinsock();
SOCKET CreateListenSocket();
void ProcessClient(SOCKET ClientSocket);
void ServerThread();
SOCKET CreateSocket(const char* serverName);
void SendCommand(SOCKET ConnectSocket, const char* command);
void Cleanup(SOCKET ConnectSocket);
void ClientThread(const string& command);
void DisplayMenu();
string GetCommand(int choice);

extern atomic<bool> serverRunning;

#endif // SERVER_CLIENT_HPP