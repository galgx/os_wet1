/*
 * commands.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: os
 */
#include "commands.h"
#include "signals.h"
#include <iostream>
#include <errno.h>
#include <vector>
#include <sys/wait.h>
extern char* L_Fg_Cmd;
using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::cerr;
const char* SHELL = "/bin/csh";
#define MAX_HISTORY_LENGTH 50
//global history_vector
vector<string> cmd_history;
vector<job> jobs_vector;
int pid_last_fg=-1;
string cmd_last_fg="";
//********************************************
// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************

void add_cmd_to_history(char* cmdString,char* arg){
	if(!strcmp(arg,"history")) return;
	if(cmd_history.size()>=MAX_HISTORY_LENGTH){ // history too long
		cmd_history.erase(cmd_history.begin());
		cmd_history.push_back(cmdString);
	}else{
		cmd_history.push_back(cmdString);
	}
}

void delete_jobs_finish(vector<job> &jobs_vector){
	for(int i=0;i<(int)jobs_vector.size();){
		if(waitpid(jobs_vector[i].pid,NULL,WNOHANG)!=0){
			jobs_vector.erase(jobs_vector.begin()+i);
		}else{
			i++;
		}
	}

}

int ExeCmd(void* jobs, char* lineSize, char* cmdString, char* last_pwd)
{
	char* cmd;
	char* args[MAX_ARG];
	char temp[10]=" \t\n";
	char* delimiters = temp;
	int i = 0, num_arg = 0;
	bool illegal_cmd = false; // illegal command
    	cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0;
   	args[0] = cmd;
	for (i=1; i<MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters);
		if (args[i] != NULL)
			num_arg++;

	}
	add_cmd_to_history(cmdString,args[0]);
/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
	if (!strcmp(cmd, "cd") )
	{
		if(num_arg!=1){ // wrong amount of arguments
			illegal_cmd=true;
			cerr << "smash error: > \"" << cmdString << "\"" << endl;
			return 0;
		}
		if(!strcmp(args[1],"-")){ // - option
			if(!strcmp(last_pwd,"")){ //- option failed
				return 0;
			}else{ //- option
				char pwd[MAX_LINE_SIZE];
				getcwd(pwd,sizeof(pwd));
				if(chdir(last_pwd)==-1){
					cerr << "smash error: > \"" << last_pwd << "\" - path not found" << endl;
				}else{
					cout << last_pwd << endl;
					strcpy(last_pwd,pwd);
				}
			}
		}else{ // path option
			char pwd[MAX_LINE_SIZE];
			getcwd(pwd,sizeof(pwd));
			if(chdir(args[1])==-1){
				cerr << "smash error: > \"" << args[1] << "\" - path not found" << endl;
			}else{
				strcpy(last_pwd,pwd);
			}
		}
	}

	/*************************************************/
	else if (!strcmp(cmd, "pwd"))
	{
		if(num_arg!=0){
			cerr << "smash error: > \"" << cmdString << "\"" << endl;
			return 0;
		}
		char pwd[MAX_LINE_SIZE];
		if(getcwd(pwd,sizeof(pwd))!=NULL){
			cout << pwd << endl;
		}else{
			cerr << "system function failed: getcwd" << endl;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "mv"))
	{
		if(num_arg!=2){
			illegal_cmd=true;
		}else{
			if(rename(args[1],args[2])!=0){
				perror("system function failed: rename");
			}else{
				cout << args[1] << " has been renamed to " << args[2] << endl;
			}
		}
	}

	/*************************************************/
	else if (!strcmp(cmd, "history"))
	{
		if(num_arg!=0){
			illegal_cmd=true;
		}else{
			for(int i=0;i<(int)cmd_history.size();i++){
				cout << cmd_history[i] << endl;
			}
		}
	}
	/*************************************************/

	else if (!strcmp(cmd, "jobs"))
	{
		delete_jobs_finish(jobs_vector);
		if(num_arg!=0){
			illegal_cmd=true;
		}else{
			time_t cur_t;
			time(&cur_t);
			for(int i=0;i<(int)jobs_vector.size();i++){
				time_t time_d=cur_t-jobs_vector[i].start_time;
				cout << "[" << i+1 << "] " << jobs_vector[i].command << ": " << jobs_vector[i].pid << " " << time_d << " secs";
				if(jobs_vector[i].is_suspended==true){
					cout << "(Stopped)" <<endl;
				}else{
					cout << endl;
				}
			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "showpid"))
	{
		if(num_arg!=0){
			illegal_cmd=true;
		}else{
			cout << "smash pid is " << getppid() << endl;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg"))
	{
		delete_jobs_finish(jobs_vector);
		if(num_arg>1){
			illegal_cmd=true;
		}else{
			if(num_arg==0){
				if(jobs_vector.size()==0){
					cout <<	"no such job"<< endl;
					return 0;
				}
				cout << jobs_vector[jobs_vector.size()-1].command << endl;
				pid_last_fg=jobs_vector[jobs_vector.size()-1].pid;
				cmd_last_fg=jobs_vector[jobs_vector.size()-1].command;
				int status;
				kill(pid_last_fg,SIGCONT); // continue in case of stopped
				jobs_vector.erase(jobs_vector.end());
				waitpid(pid_last_fg,&status,WUNTRACED);
				pid_last_fg=-1;
			}else{
				int num_job=atoi(args[1]);
				if(num_job<=0 || num_job>(int)jobs_vector.size()){
					cerr << "job not found" << endl;
				}else{
					cout << jobs_vector[num_job-1].command << endl;
					pid_last_fg=jobs_vector[num_job-1].pid;
					cmd_last_fg=jobs_vector[num_job-1].command;
					int status;
					kill(pid_last_fg,SIGCONT); // continue in case of stopped
					jobs_vector.erase(jobs_vector.begin()+num_job-1);
					waitpid(pid_last_fg,&status,WUNTRACED);
					pid_last_fg=-1;
				}

			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "bg"))
	{
		delete_jobs_finish(jobs_vector);
		int last_stopped;
		if(num_arg>1){
			illegal_cmd=true;
		}else{
			if(num_arg==0){
				if(jobs_vector.size()==0){
					cout <<	"no such job"<< endl;
					return 0;
				}
				for(last_stopped=jobs_vector.size()-1;last_stopped>=0;last_stopped--){
					if(jobs_vector[last_stopped].is_suspended==true){
						break;
					}
				}
				if(last_stopped<0){
					cout <<	"no such job"<< endl;
					return 0;
				}
				cout << jobs_vector[last_stopped].command << endl;
				kill(jobs_vector[last_stopped].pid,SIGCONT); // continue in case of stopped
				jobs_vector[last_stopped].is_suspended=false;
			}else{
				int num_job=atoi(args[1]);
				if(num_job<=0 || num_job>(int)jobs_vector.size()){
					cerr << "job not found" << endl;
				}else{
					if(jobs_vector[num_job-1].is_suspended==false){
						cout << "job " << num_job << " already in background" << endl;
						return 0;
					}
					cout << jobs_vector[num_job-1].command << endl;
					kill(jobs_vector[num_job-1].pid,SIGCONT); // continue in case of stopped
					jobs_vector[num_job-1].is_suspended=false;
				}

			}
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		delete_jobs_finish(jobs_vector);
		if(num_arg!=2){
			illegal_cmd=true;
		}else{
			int num_signal=(atoi(args[1]));
			if(num_signal > 0 ){
				illegal_cmd=true;
			}else{
				num_signal=abs(num_signal);
				int num_job=atoi(args[2]);
				if(num_job<=0 || num_job>(int)jobs_vector.size()){
					cerr << "smash error: > kill " << args[2] << "-job does not exist" << endl;
				}else{
					int pid=jobs_vector[num_job-1].pid;
					if(signal_sender(pid,num_signal)==true){
						if(num_signal==19 || num_signal==20){
							//handle stop signal
							jobs_vector[num_job-1].is_suspended=true;
						}else if(num_signal==18){
							jobs_vector[num_job-1].is_suspended=false;
						}
					}
				}
			}

		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
		delete_jobs_finish(jobs_vector);
		if(num_arg>1){
			illegal_cmd=true;
		}else{
			if(num_arg==0){
				exit(-1);
			}
			time_t cur_time1, cur_time2;
			for(int i=0;i<(int)jobs_vector.size();i++){
				cout << "[" << i+1 << "]" <<jobs_vector[i].command << "-Sending SIGTERM...";
				kill(jobs_vector[i].pid,SIGTERM);
				//wait 5 secs
				time(&cur_time1);
				cur_time2=cur_time1 + 5;
				bool is_dead=false;
				while(cur_time1<cur_time2){
					if(waitpid(jobs_vector[i].pid,NULL,WNOHANG)!=0){
						is_dead=true;
						break;
					}
					time(&cur_time1);
				}
				if( is_dead==true){
					cout<< "Done." <<endl;
				}else{
					cout<< "(5 sec passed) Sending SIGKILL... Done."<< endl;
					if((waitpid(jobs_vector[i].pid,NULL,WNOHANG)==0)){
						kill(jobs_vector[i].pid,SIGKILL);
					}
				}

			}

		}
		exit(-1);
	}
	/*************************************************/
	else // external command
	{
 		ExeExternal(args, args[0],cmdString,lineSize);
	 	return 0;
	}
	if (illegal_cmd == true)
	{
		printf("smash error: > \"%s\"\n", cmdString);
		return 1;
	}
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char *args[MAX_ARG], char* cmd, char* cmdString, char* lineSize)
{
	int pID,status;
	string new_cmd(cmdString);
		switch (pID = fork())
		{
			case -1: //error
				cerr << "failed to run external command: " << args[0] << endl;
				return;
			case 0:// Child Process
				setpgrp();
				execvp(args[0], args);
				cerr << "failed to run external command: " << args[0] << endl;
				kill(getpid(),SIGKILL);
				return;

			default: //parent
				L_Fg_Cmd=cmdString;
				pid_last_fg=pID;
				cmd_last_fg=args[0];
				waitpid(pID,&status,WUNTRACED);
				pid_last_fg=-1;
		}

}
//**************************************************************************************
// function name: ExeComp
// Description: executes complicated command
// Parameters: command string
// Returns: 0- if complicated -1- if not
//**************************************************************************************
int ExeComp(char* lineSize, char* cmdString)
{
	int pid, state;
		if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
		{
			char *args[MAX_ARG];
			char* cmd;
			char* cmd_cpy=(char*)malloc(sizeof(*lineSize)+1);
			strcpy(cmd_cpy,lineSize);
			char temp[10]=" \t\n";
			char* delimiters = temp;
			cmd = strtok(lineSize, delimiters);
			args[0]=cmd;
			if (cmd == NULL)
				return 0;
			switch (pid = fork())
			{
				case -1:
				{
					cerr << "can't execute fork for complicated cmd" << endl;
					return -1;
					break;
				}

				case 0: //this is child proc
				{
					setpgrp();
					//args[0]="csh";
					//args[1]="-fc";
					//args[2]=cmd_cpy;
					//execvp(args[0], args);
					execl(SHELL,SHELL,"-cf",cmd_cpy,NULL);
					cerr << "couldn't excecute external command" << endl;
					//send_signal(getpid(), SIGTERM); //killing child process because execvp failed
					kill(getpid(),SIGKILL);
					return -1;
					break;
				}

				default:
				{
					//COMPLECATED COMMAND IN BG
					if (lineSize[strlen(lineSize) - 2] == '&') //bg comp command
					{
						time_t cur_time;
						time(&cur_time);
						job j(args[0],pid,cur_time,false);
						jobs_vector.push_back(j);
						break;
					}
					else //regular comp command
					{
						pid_last_fg=pid;
						cmd_last_fg=args[0];
						waitpid(pid, &state, WUNTRACED);
						pid_last_fg= -1;
					}
					return 0;
					break;

				}
			}
		}
		return -1;
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, void* jobs, char* cmdString)
{
	const char* delimiters = " \t\n";
	char *args[MAX_ARG];
	int pid, i;

	if (lineSize[strlen(lineSize) - 2] == '&'){
		lineSize[strlen(lineSize) - 2] = '\0';
		args[0] = strtok(lineSize, delimiters);
		if (args[0] == NULL) return 0; //command was just &
		for (i = 1; i<MAX_ARG; i++){
			args[i] = strtok(NULL, delimiters);
		}

		switch (pid = fork())
		{
			case -1: //error
				cerr << "Command aimed for bg: failed to create child process" << endl;
				return -1;
			case 0: //child process
				setpgrp();
				execvp(args[0], args);
				cerr << "Command aimed for bg: execvp failed" << endl;
				kill(getpid(),SIGKILL);
				return(-1);
			default:
				time_t cur_time;
				time(&cur_time);
				char temp[100];
				strcpy(temp,args[0]);
				job j(temp,pid,cur_time,false);
				jobs_vector.push_back(j);
				break;
		}
		return 0;
	}
	return -1;
	}


	/*
	char* Command;
	char* delimiters = " \t\n";
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		lineSize[strlen(lineSize)-2] = '\0';
		char* cmd;
		char* args[MAX_ARG];
		char pwd[MAX_LINE_SIZE];
		char* delimiters = " \t\n";
		int i = 0, num_arg = 0;
		bool illegal_cmd = false; // illegal command
			cmd = strtok(lineSize, delimiters);
		if (cmd == NULL)
			return 0;
		args[0] = cmd;
		for (i=1; i<MAX_ARG; i++)
		{
			args[i] = strtok(NULL, delimiters);
			if (args[i] != NULL)
				num_arg++;

		}
		// Add your code here (execute a in the background)
		cout << args[0] << args[1] <<endl;
		ExeExternal(args, args[0], false, true, cmdString,lineSize);
	}
	return -1;
}*/



