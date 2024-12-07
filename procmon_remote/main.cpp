// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

#include "ServerClient.hpp"

int main() {
    thread server(ServerThread);
    this_thread::sleep_for(chrono::seconds(2));

    int choice;
    string command;

    do {
        DisplayMenu();
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 0) break;

        command = GetCommand(choice);
        if (!command.empty()) {
            thread client(ClientThread, command);
            client.join();
        }

    } while (choice != 0);

    serverRunning = false;
    server.join();

    return 0;
}

// Description: This file contains the main function that runs the server and client threads.

// The server thread listens for incoming connections and processes the commands sent by the client.
// The client thread sends commands to the server and receives the response.

// The main function displays a menu of available commands and allows the user to choose a command to execute.
// The user can exit the program by entering 0.

// The server and client threads are implemented using Winsock API for socket programming.

// Use DisplayHelp function to display the available commands and their descriptions.