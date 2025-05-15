// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

// The ServerClient.hpp file contains the declarations of the functions used to create the server and client threads.
// The server thread listens for incoming connections and processes the commands sent by the client.
// The client thread sends commands to the server and receives the response.
// The main function displays a menu of available commands and allows the user to choose a command to execute.

#include "ServerClient.hpp"
#include "ProcessInfo.hpp"

atomic<bool> serverRunning(true);

void InitializeWinsock() {
    WSADATA wsaData;
    int iResult=WSAStartup(MAKEWORD(2,2),&wsaData);
    if (iResult != 0) {
        cerr<<"WSAStartup failed: "<<iResult<<endl;
        exit(1);
    }
}

SOCKET CreateListenSocket() {
    struct addrinfo* result=nullptr,hints{};
    ZeroMemory(&hints,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;
    hints.ai_flags=AI_PASSIVE;

    int iResult=getaddrinfo(nullptr,DEFAULT_PORT,&hints,&result);
    if (iResult != 0) {
        cerr<<"getaddrinfo failed: "<<iResult<<endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ListenSocket=socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    if (ListenSocket==INVALID_SOCKET) {
        cerr<<"Error at socket(): "<<WSAGetLastError()<<endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    iResult=bind(ListenSocket,result->ai_addr,(int)result->ai_addrlen);
    if (iResult==SOCKET_ERROR) {
        cerr<<"bind failed: "<<WSAGetLastError()<<endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);

    iResult=listen(ListenSocket,SOMAXCONN);
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
    int iResult,iSendResult;
    ProcessInfo processInfo;

    unordered_map<string,bool (ProcessInfo::*)(const char*)> commandMap= {
            {"ProcessLog",            &ProcessInfo::ProcessLog},
            {"ProcessSearch",         &ProcessInfo::ProcessSearch},
            {"KillProcess",           &ProcessInfo::KillProcess},
            {"GetProcessMemoryUsage", &ProcessInfo::GetProcessMemoryUsage},
            {"GetProcessUserName",    &ProcessInfo::GetProcessUserName},
            {"GetProcessStatus",      &ProcessInfo::GetProcessStatus},
            {"GetProcessDescription", &ProcessInfo::GetProcessDescription},
            {"GetProcessPriority",    &ProcessInfo::GetProcessPriority},
            {"GetProcessStartTime",   &ProcessInfo::GetProcessStartTime},
            {"GetProcessCPUUsage",    &ProcessInfo::GetProcessCPUUsage},
            {"GetProcessPath",        &ProcessInfo::GetProcessPath},
            {"VerifyProcessIntegrity", &ProcessInfo::VerifyProcessIntegrity},
    };

    unordered_map<string,bool (ProcessInfo::*)()> commandMapNoParam={
        {"ProcessDisplay",&ProcessInfo::ProcessDisplay},
        {"DisplayHardwareInfo",&ProcessInfo::DisplayHardwareInfo},
        {"FastLimitRAM",&ProcessInfo::FastLimitRAM},
        {"LimitRAMWithJobObjects",&ProcessInfo::LimitRAMWithJobObjects},
        {"LimitLogicalProcessors",&ProcessInfo::LimitLogicalProcessors},
        {"OpenGivenProcess",&ProcessInfo::OpenGivenProcess},
        {"DisplayRaspberryProcesses",&ProcessInfo::DisplayRaspberryProcesses},
        {"LimitArduinoPowerForAnalogAndDigital",&ProcessInfo::LimitArduinoPowerForAnalogAndDigital},
        {"OptimizeProcessPerformance",&ProcessInfo::OptimizeProcessPerformance},
    };

    do {
        iResult=recv(ClientSocket,recvbuf,DEFAULT_BUFLEN,0);
        if (iResult > 0) {
            string command(recvbuf,iResult);
            cout<<"Received command: "<<command<<endl;

            istringstream iss(command);
            string method;
            string param;
            iss>>method>>param;

            ostringstream response;

            if (commandMap.find(method) != commandMap.end()) {
                response<<((processInfo.*commandMap[method])(param.c_str()) ? "Success" : "Failure");
            } else if (commandMapNoParam.find(method) != commandMapNoParam.end()) {
                response<<((processInfo.*commandMapNoParam[method])() ? "Success" : "Failure");
            } else if (method=="DisplayHelp") {
                processInfo.DisplayHelp();
                response<<"Help displayed";
            } else {
                response<<"Unknown command";
            }

            string responseStr=response.str();
            iSendResult=send(ClientSocket,responseStr.c_str(),responseStr.length(),0);
            if (iSendResult==SOCKET_ERROR) {
                cerr<<"send failed: "<<WSAGetLastError()<<endl;
                closesocket(ClientSocket);
                WSACleanup();
                exit(1);
            }
            cout<<"\nCommand processed and response sent: "<<responseStr<<endl;
        } else if (iResult==0) {
            // Connection closing
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

    cout<<"Waiting for client to connect...\n\n";

    while (serverRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(ListenSocket,&readfds);

        timeval timeout{};
        timeout.tv_sec=1;
        timeout.tv_usec=0;

        int selectResult=select(0,&readfds,nullptr,nullptr,&timeout);
        if (selectResult==SOCKET_ERROR) {
            cerr<<"select failed: "<<WSAGetLastError()<<endl;
            break;
        }

        if (selectResult==0) {
            continue; // Timeout, check serverRunning a flag again
        }

        SOCKET ClientSocket=accept(ListenSocket,nullptr,nullptr);
        if (ClientSocket==INVALID_SOCKET) {
            if (serverRunning) {
                cerr<<"accept failed: "<<WSAGetLastError()<<endl;
            }
            break;
        }

        cout<<"Client connected."<<endl;

        thread clientThread(ProcessClient,ClientSocket);
        clientThread.detach(); // Detach the thread to handle multiple clients
    }

    closesocket(ListenSocket);
    WSACleanup();
}

SOCKET CreateSocket(const char* serverName) {
    struct addrinfo* result=nullptr,* ptr=nullptr,hints{};
    ZeroMemory(&hints,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_protocol=IPPROTO_TCP;

    int iResult=getaddrinfo(serverName,DEFAULT_PORT,&hints,&result);
    if (iResult != 0) {
        cerr<<"getaddrinfo failed: "<<iResult<<endl;
        WSACleanup();
        exit(1);
    }

    auto ConnectSocket=INVALID_SOCKET;
    for (ptr=result; ptr != nullptr; ptr=ptr->ai_next) {
        ConnectSocket=socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol);
        if (ConnectSocket==INVALID_SOCKET) {
            cerr<<"Error at socket(): "<<WSAGetLastError()<<endl;
            freeaddrinfo(result);
            WSACleanup();
            exit(1);
        }

        iResult=connect(ConnectSocket,ptr->ai_addr,(int)ptr->ai_addrlen);
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

void SendCommand(SOCKET ConnectSocket,const char* command) {
    int iResult=send(ConnectSocket,command,(int)strlen(command),0);
    if (iResult==SOCKET_ERROR) {
        cerr<<"send failed: "<<WSAGetLastError()<<endl;
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }
}

void Cleanup(SOCKET ConnectSocket) {
    int iResult=shutdown(ConnectSocket,SD_SEND);
    if (iResult==SOCKET_ERROR) {
        cerr<<"shutdown failed: "<<WSAGetLastError()<<endl;
    }
    closesocket(ConnectSocket);
}

void ClientThread(const string& command) {
    InitializeWinsock();

    const char* serverName="192.168.100.7"; //192.168.100.8 for home
    SOCKET ConnectSocket=CreateSocket(serverName);

    SendCommand(ConnectSocket,command.c_str());

    char recvbuf[DEFAULT_BUFLEN];
    int iResult=recv(ConnectSocket,recvbuf,DEFAULT_BUFLEN,0);
    if (iResult > 0) {
        cout<<"Received response from server: "<<string(recvbuf,iResult)<<endl<<endl;
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
    cout<<"18. LimitLogicalProcessors\n";
    cout<<"19. DisplayRaspberryProcesses\n";
    cout<<"20. LimitArduinoPowerForAnalogAndDigital\n";
    cout<<"21. OptimizeProcessPerformance\n";
    cout<<"22. VerifyProcessIntegrity\n";
    cout<<"\n0. Exit\n";
}

string GetCommand(int choice) {
    unordered_map<int,pair<string,bool>> commandMap={
        {1,{"ProcessDisplay",false}},
        {2,{"ProcessLog",true}},
        {3,{"ProcessSearch",true}},
        {4,{"KillProcess",true}},
        {5,{"DisplayHardwareInfo",false}},
        {6,{"GetProcessMemoryUsage",true}},
        {7,{"GetProcessUserName",true}},
        {8,{"GetProcessStatus",true}},
        {9,{"GetProcessDescription",true}},
        {10,{"GetProcessPriority",true}},
        {11,{"GetProcessStartTime",true}},
        {12,{"GetProcessCPUUsage",true}},
        {13,{"GetProcessPath",true}},
        {14,{"DisplayHelp",false}},
        {15,{"OpenGivenProcess",false}},
        {16,{"FastLimitRAM",false}},
        {17,{"LimitRAMWithJobObjects",false}},
        {18,{"LimitLogicalProcessors",false}},
        {19,{"DisplayRaspberryProcesses",false}},
        {20,{"LimitArduinoPowerForAnalogAndDigital",false}},
        {21,{"OptimizeProcessPerformance",false}},
        {22,{"VerifyProcessIntegrity",true}},

    };

    string command;
    if (commandMap.find(choice) != commandMap.end()) {
        command=commandMap[choice].first;
        if (commandMap[choice].second) {
            string processName;
            cout<<"Enter process name: ";
            cin>>processName;
            command += " "+processName;
        }
    }

    return command;
}