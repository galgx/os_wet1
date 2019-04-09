#ifndef _COMMANDS_H
#define _COMMANDS_H
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_LINE_SIZE 80
#define MAX_ARG 20
//typedef enum { FALSE , TRUE } bool;

class job{

public:
	char* command;
	int pid;
	time_t start_time;
	bool is_suspended;
	job(const char* command,int pid,time_t start_time,bool is_suspended){
		this->command=(char*)malloc(sizeof(char)*100);
		strcpy(this->command,command);
		this->pid=pid;
		this->start_time=start_time;
		this->is_suspended=is_suspended;
	}
};

int ExeComp(char* lineSize, char* cmdString);
int BgCmd(char* lineSize, void* jobs, char* cmdString);
int ExeCmd(void* jobs, char* lineSize, char* cmdString,char* last_pwd);
void ExeExternal(char *args[MAX_ARG], char* cmd, char* cmdString,char* lineSize);
#endif

