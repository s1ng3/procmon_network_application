#ifndef RASPBERRY_PROCESSES_HPP
#define RASPBERRY_PROCESSES_HPP

#include <string>
#include <libssh/libssh.h>
#include <conio.h>

using namespace std;

string executeRemoteCommand(ssh_session,const string&);
string GetRaspberryProcesses(const string&,const string&,const string&);
bool DisplayRaspberryProcesses();

#endif // RASPBERRY_PROCESSES_HPP