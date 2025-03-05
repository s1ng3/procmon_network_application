// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

// The ServerClient.hpp file contains the declarations of the functions used to create the server and client threads.
// The server thread listens for incoming connections and processes the commands sent by the client.
// The client thread sends commands to the server and receives the response.
// The main function displays a menu of available commands and allows the user to choose a command to execute.

#include "ServerClient.hpp"

atomic<bool> serverRunning(true);

void InitializeWinsock() {
    WSADATA wsaData;
    int iResult=WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr<<"WSAStartup failed: "<<iResult<<endl;
        exit(1);
    }
}

SOCKET CreateListenSocket() {
    struct addrinfo* result=NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;
    hints.ai_flags=AI_PASSIVE;

    int iResult=getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cerr<<"getaddrinfo failed: "<<iResult<<endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ListenSocket=socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket==INVALID_SOCKET) {
        cerr<<"Error at socket(): "<<WSAGetLastError()<<endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    iResult=bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult==SOCKET_ERROR) {
        cerr<<"bind failed: "<<WSAGetLastError()<<endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);

    iResult=listen(ListenSocket, SOMAXCONN);
    if (iResult==SOCKET_ERROR) {
        cerr<<"listen failed: "<<WSAGetLastError()<<endl;
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    cout<<"Server is listening on port "<<DEFAULT_PORT<<endl;

    return ListenSocket;
}

void ProcessClient(SOCKET ClientSocket) {
    char recvbuf[DEFAULT_BUFLEN];
    int iResult, iSendResult;
    ProcessInfo processInfo;

    do {
        iResult=recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0) {
            string command(recvbuf, iResult);
            cout<<"Received command: "<<command<<endl;

            istringstream iss(command);
            string method;
            string param;
            iss>>method>>param;

            ostringstream response;

            if (method=="ProcessDisplay") {
                response<<(processInfo.ProcessDisplay() ? "Success" : "Failure");
            } else if (method=="ProcessLog") {
                response<<(processInfo.ProcessLog(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="ProcessSearch") {
                response<<(processInfo.ProcessSearch(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="KillProcess") {
                response<<(processInfo.KillProcess(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="OpenGivenProcess") {
                response<<(processInfo.OpenGivenProcess() ? "Success" : "Failure");
            } else if (method=="DisplayHardwareInfo") {
                response<<(processInfo.DisplayHardwareInfo() ? "Success" : "Failure");
            } else if (method=="GetProcessMemoryUsage") {
                response<<(processInfo.GetProcessMemoryUsage(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="GetProcessUserName") {
                response<<(processInfo.GetProcessUserName(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="GetProcessStatus") {
                response<<(processInfo.GetProcessStatus(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="GetProcessDescription") {
                response<<(processInfo.GetProcessDescription(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="GetProcessPriority") {
                response<<(processInfo.GetProcessPriority(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="GetProcessStartTime") {
                response<<(processInfo.GetProcessStartTime(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="GetProcessCPUUsage") {
                response<<(processInfo.GetProcessCPUUsage(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="FastLimitRAM") {
                response<<(processInfo.FastLimitRAM() ? "Success" : "Failure");
            } else if (method=="LimitRAMWithJobObjects") {
                response<<(processInfo.LimitRAMWithJobObjects() ? "Success" : "Failure");
            } else if (method=="LimitCORE") {
                response<<(processInfo.LimitCORE() ? "Success" : "Failure");
            } else if (method=="GetProcessPath") {
                response<<(processInfo.GetProcessPath(const_cast<char*>(param.c_str())) ? "Success" : "Failure");
            } else if (method=="DisplayHelp") {
                processInfo.DisplayHelp();
                response<<"Help displayed";
            } else {
                response<<"Unknown command";
            }

            string responseStr=response.str();
            iSendResult=send(ClientSocket, responseStr.c_str(), responseStr.length(), 0);
            if (iSendResult==SOCKET_ERROR) {
                cerr<<"send failed: "<<WSAGetLastError()<<endl;
                closesocket(ClientSocket);
                WSACleanup();
                exit(1);
            }
            cout<<"\n Command processed and response sent: "<<responseStr<<endl;
        } else if (iResult==0) {
            // cout<<"Connection closing..."<<endl; //TODO fix in console
        } else {
            cerr<<"recv failed: "<<WSAGetLastError()<<endl;
            closesocket(ClientSocket);
            WSACleanup();
            exit(1);
        }
    } while (iResult > 0);

    closesocket(ClientSocket);
}

void ServerThread() {
    InitializeWinsock();

    SOCKET ListenSocket=CreateListenSocket();

    cout<<"Waiting for client to connect..."<<endl;

    while (serverRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(ListenSocket, &readfds);

        timeval timeout;
        timeout.tv_sec=1;
        timeout.tv_usec=0;

        int selectResult=select(0, &readfds, NULL, NULL, &timeout);
        if (selectResult==SOCKET_ERROR) {
            cerr<<"select failed: "<<WSAGetLastError()<<endl;
            break;
        }

        if (selectResult==0) {
            continue; // Timeout, check serverRunning flag again
        }

        SOCKET ClientSocket=accept(ListenSocket, NULL, NULL);
        if (ClientSocket==INVALID_SOCKET) {
            if (serverRunning) {
                cerr<<"accept failed: "<<WSAGetLastError()<<endl;
            }
            break;
        }

        cout<<"Client connected."<<endl;

        thread clientThread(ProcessClient, ClientSocket);
        clientThread.detach(); // Detach the thread to handle multiple clients
    }

    closesocket(ListenSocket);
    WSACleanup();
}

SOCKET CreateSocket(const char* serverName) {
    struct addrinfo* result=NULL, * ptr=NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;

    int iResult=getaddrinfo(serverName, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cerr<<"getaddrinfo failed: "<<iResult<<endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ConnectSocket=INVALID_SOCKET;
    for (ptr=result; ptr != NULL; ptr=ptr->ai_next) {
        ConnectSocket=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket==INVALID_SOCKET) {
            cerr<<"Error at socket(): "<<WSAGetLastError()<<endl;
            freeaddrinfo(result);
            WSACleanup();
            exit(1);
        }

        iResult=connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult==SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket=INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket==INVALID_SOCKET) {
        cerr<<"Unable to connect to server!"<<endl;
        WSACleanup();
        exit(1);
    }

    return ConnectSocket;
}

void SendCommand(SOCKET ConnectSocket, const char* command) {
    int iResult=send(ConnectSocket, command, (int)strlen(command), 0);
    if (iResult==SOCKET_ERROR) {
        cerr<<"send failed: "<<WSAGetLastError()<<endl;
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }
}

void Cleanup(SOCKET ConnectSocket) {
    int iResult=shutdown(ConnectSocket, SD_SEND);
    if (iResult==SOCKET_ERROR) {
        cerr<<"shutdown failed: "<<WSAGetLastError()<<endl;
    }
    closesocket(ConnectSocket);
}

void ClientThread(const string& command) {
    InitializeWinsock();

    const char* serverName="192.168.100.8"; //192.168.100.8 for home
    SOCKET ConnectSocket=CreateSocket(serverName);

    SendCommand(ConnectSocket, command.c_str());

    char recvbuf[DEFAULT_BUFLEN];
    int iResult=recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
    if (iResult > 0) {
        cout<<"Received response from server: "<<string(recvbuf, iResult)<<endl;
    } else if (iResult==0) {
        cout<<"Connection closed by server."<<endl;
    } else {
        cerr<<"recv failed: "<<WSAGetLastError()<<endl;
    }

    Cleanup(ConnectSocket);
}

void DisplayMenu() {
    cout<<"Menu:\n";
    cout<<"1. ProcessDisplay\n";
    cout<<"2. ProcessLog\n";
    cout<<"3. ProcessSearch\n";
    cout<<"4. KillProcess\n";
    cout<<"5. DisplayHardwareInfo\n";
    cout<<"6. GetProcessMemoryUsage\n";
    cout<<"7. GetProcessUserName\n";
    cout<<"8. GetProcessStatus\n";
    cout<<"9. GetProcessDescription\n";
    cout<<"10. GetProcessPriority\n";
    cout<<"11. GetProcessStartTime\n";
    cout<<"12. GetProcessCPUUsage\n";
    cout<<"13. GetProcessPath\n";
    cout<<"14. DisplayHelp\n";
    cout<<"15. OpenGivenProcess\n";
    cout<<"16. FastLimitRAM\n"; // Added option to limit RAM
    cout<<"17. LimitRAMWithJobObjects\n"; // Added option to limit RAM with Job Objects
    cout<<"18. LimitCORE\n";
    cout<<"0. Exit\n";
}

string GetCommand(int choice) {
    string command;
    string processName;

    switch (choice) {
    case 1:
        command="ProcessDisplay";
        command += " " + processName;
        break;
    case 2:
        command="ProcessLog";
        cout<<"Enter log file name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 3:
        command="ProcessSearch";
        cout<<"Enter search term: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 4:
        command="KillProcess";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 5:
        command="DisplayHardwareInfo";
        break;
    case 6:
        command="GetProcessMemoryUsage";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 7:
        command="GetProcessUserName";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 8:
        command="GetProcessStatus";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 9:
        command="GetProcessDescription";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 10:
        command="GetProcessPriority";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 11:
        command="GetProcessStartTime";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 12:
        command="GetProcessCPUUsage";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 13:
        command="GetProcessPath";
        cout<<"Enter process name: ";
        cin>>processName;
        command += " " + processName;
        break;
    case 14:
        command="DisplayHelp";
        break;
    case 15:
        command="OpenGivenProcess";
        break;
    case 16:
        command="FastLimitRAM";
        break;
    case 17:
        command="LimitRAMWithJobObjects";
        break;
    case 18:
        command="LimitCORE";
        break;
    default:
        command="";
        break;
    }

    return command;
}