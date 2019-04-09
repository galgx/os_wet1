/*
 * smash.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: os
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include "commands.h"
#include "signals.h"
using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::cerr;
#define MAX_LINE_SIZE 80
#define MAXARGS 20



char* L_Fg_Cmd;

void* jobs = NULL; //This represents the list of jobs. Please change to a preferred type (e.g array of char*)
char lineSize[MAX_LINE_SIZE];
char last_pwd[MAX_LINE_SIZE]="";



//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char *argv[])
{
    char cmdString[MAX_LINE_SIZE];


	//signal declaretions
	//NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
	 /* add your code here */

	/************************************/
	//NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
	//set your signal handlers here
	/* add your code here */

	/************************************/
    struct sigaction act;
	sigset_t sigmask;
	sigfillset(&sigmask);
	act.sa_mask = sigmask;
	act.sa_flags = 0;
	act.sa_handler = &signal_handler;
	if ((sigaction(SIGINT, &act, NULL) == -1) || (sigaction(SIGTSTP, &act, NULL) == -1) ){ // sets signal_handler function to handle ctrl+c and z
		perror("not able to run sigaction");
	}

	/************************************/
	// Init globals



	L_Fg_Cmd =(char*)malloc(sizeof(char)*(MAX_LINE_SIZE+1));

	if (L_Fg_Cmd == NULL)
			exit (-1);
	L_Fg_Cmd[0] = '\0';

    	while (1)
    	{
	 	printf("smash > ");
		fgets(lineSize, MAX_LINE_SIZE, stdin);
		strcpy(cmdString, lineSize);
		cmdString[strlen(lineSize)-1]='\0';
					// perform a complicated Command
		if(!ExeComp(lineSize, cmdString)) continue;
					// background command
	 	if(!BgCmd(lineSize,jobs,cmdString)) continue;
					// built in commands
		ExeCmd(jobs, lineSize, cmdString,last_pwd);

		/* initialize for next line read*/
		lineSize[0]='\0';
		cmdString[0]='\0';
	}
    return 0;
}
