
#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <signal.h>
#include<iostream>

using std::string;
//a function that sets the behavior for ctrl+c and ctrl+z
void signal_handler(int signum);
//a function that will send the signum to the pid process. will return true if success.
bool signal_sender(int pid, int signum);
//will return the name of the signal from it's int representation
string signal_name(int signum);


#endif

