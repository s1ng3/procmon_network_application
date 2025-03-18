//
// Author: Tudor-Cristian Sîngerean
// Date: 15.03.2024
//

#include "RaspberryProcesses.hpp"
#include <libssh/libssh.h>
#include <iostream>
#include <stdexcept>

using namespace std;

string executeRemoteCommand(ssh_session session,const string& command) {
    ssh_channel channel=ssh_channel_new(session);
    if (channel==nullptr) {
        throw runtime_error("Error creating SSH channel");
    }

    if (ssh_channel_open_session(channel) != SSH_OK) {
        ssh_channel_free(channel);
        throw runtime_error("Error opening SSH channel");
    }

    if (ssh_channel_request_exec(channel,command.c_str()) != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        throw runtime_error("Error executing command");
    }

    char buffer[256];
    string result;
    int nbytes;
    while ((nbytes=ssh_channel_read(channel,buffer,sizeof(buffer),0)) > 0) {
        result.append(buffer,nbytes);
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return result;
}

string GetRaspberryProcesses(const string& hostname,const string& username,const string& password) {
    ssh_session session=ssh_new();
    if (session==nullptr) {
        throw runtime_error("Error creating SSH session");
    }

    ssh_options_set(session,SSH_OPTIONS_HOST,hostname.c_str());
    ssh_options_set(session,SSH_OPTIONS_USER,username.c_str());

    if (ssh_connect(session) != SSH_OK) {
        string error=ssh_get_error(session);
        ssh_free(session);
        throw runtime_error("Error connecting to Raspberry Pi: "+error);
    }

    if (ssh_userauth_password(session,nullptr,password.c_str()) != SSH_AUTH_SUCCESS) {
        string error=ssh_get_error(session);
        ssh_disconnect(session);
        ssh_free(session);
        throw runtime_error("Error authenticating with password: "+error);
    }

    string output;
    try {
        output=executeRemoteCommand(session,"ps -aux");
    } catch (const exception& e) {
        ssh_disconnect(session);
        ssh_free(session);
        throw;
    }

    ssh_disconnect(session);
    ssh_free(session);

    return output;
}