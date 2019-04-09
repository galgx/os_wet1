/*
 * signals.cpp
 *
 *  Created on: Apr 25, 2018
 *      Author: os
 */

#include "signals.h"
#include <vector>
#include "commands.h"
#include <string.h>
using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::cerr;
extern int pid_last_fg;
extern string cmd_last_fg;
extern std::vector<job> jobs_vector;

string signal_name(int signum)
{
	switch (signum)
	{
		case 1: return "SIGHUP";
		case 2: return "SIGINT";
		case 3: return "SIGQUIT";
		case 4: return "SIGILL";
		case 5: return "SIGTRAP";
		case 6: return "SIGABRT or SIGIOT";
		case 7: return "SIGBUS";
		case 8: return "SIGFPE";
		case 9: return "SIGKILL";
		case 10: return "SIGUSR1";
		case 11: return "SIGSEGV";
		case 12: return "SIGUSR2";
		case 13: return "SIGPIPE";
		case 14: return "SIGALRM";
		case 15: return "SIGTERM";
		case 16: return "SIGSTKFLT";
		case 17: return "SIGCHLD";
		case 18: return "SIGCONT";
		case 19: return "SIGSTOP";
		case 20: return "SIGTSTP";
		case 21: return "SIGTTIN";
		case 22: return "SIGTTOU";
		case 23: return "SIGURG";
		case 24: return "SIGXCPU";
		case 25: return "SIGXFSZ";
		case 26: return "SIGVTALRM";
		case 27: return "SIGPROF";
		case 28: return "SIGWINCH";
		case 29: return "SIGIO";
		case 30: return "SIGPWR";
		case 31: return "SIGSYS";
	default:
		return "no signal matching this id";
	}
}

void signal_handler(int signum){
	if((pid_last_fg!=-1) && (waitpid(pid_last_fg,NULL,WNOHANG)==0)){
		if(signal_sender(pid_last_fg,signum)){
			//handle ctrl c
			if(signum == SIGINT){
				pid_last_fg=-1;
			}else if(signum == SIGTSTP){ //handle ctrl z
				time_t cur_time;
				time(&cur_time);
				job j(cmd_last_fg.c_str(),pid_last_fg,cur_time,true);
				jobs_vector.push_back(j);
				pid_last_fg=-1;
			}
		}
	}else{
		cout << "no foreground process" << endl;
	}
}

bool signal_sender(int pid, int signum){
	if(signum>31 || signum<1){
		perror("illegal signum");
		return false;
	}
	bool kill_result= kill(pid, signum);
	if(kill_result){
		perror("couln't send signal");
	}else{
		cout << "smash > signal " << signal_name(signum) <<" was sent to pid " << pid << endl;
	}
	return true;
}
