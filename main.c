/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh / Krishna Agarwala
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include <signal.h>
#include "parse.h"

extern char **environ;
void buildpath();
char *path;
char **path_list;
int priority_standard=4;
int pipe1[2];
int pipe2[2];

char inp_read[100]="";
char load;
char *inp_text;
int tin_flag = 0;
int child_stat=1;
int niklo=0;
char *host = "armadillo";

int is_builtin(char *cmd){
	if((strcmp(cmd, "cd") == 0)||(strcmp(cmd, "echo") == 0)||( strcmp(cmd, "logout") == 0 )||( strcmp(cmd, "nice") == 0 )||( strcmp(cmd, "pwd") == 0 )||( strcmp(cmd, "setenv") == 0 )||( strcmp(cmd, "unsetenv") == 0 )||( strcmp(cmd, "where") == 0 ))
		return 1;
	else return 0;
}

void cmd_nice(Cmd c) {
	int check;
	int priority;

	if(c->nargs==1){
		check = setpriority(PRIO_PROCESS, getpid(), 4);
	} else if(c->nargs==2){
		if (c->args[1][0]=='-') {
			priority = (atoi(c->args[1]+1)) * (-1);
			if (priority<-20)
				priority = -20;
			else if (priority>19)
				priority = 19;
		} else if (c->args[1][0]=='+') {
			priority = (atoi(c->args[1]+1));
			if (priority<-20)
				priority = -20;
			else if (priority>19)
				priority = 19;
		} else {
			priority = (atoi(c->args[1]));
			if (priority<-20)
				priority = -20;
			else if (priority>19)
				priority = 19;
		}
		if (priority!=0) {
			check = setpriority(PRIO_PROCESS, getpid(), priority+20);
			if (check = -1)
				printf("error while using nice command\n");
		} else {
			pid_t p = fork();
			if (p==0){
				check = setpriority(PRIO_PROCESS, getpid(), priority+20);
				if (check == -1){
					printf("error while using nice command\n");
				}
				if(execvp(c->args[1], &c->args[1]) < 0) 
					perror("error executing command: ");
				exit(0);
			} else wait(5);
		}

	} else {
		if (c->args[1][0]=='-') {
			priority = (atoi(c->args[1]+1)) * (-1);
			if (priority<-20)
				priority = -20;
			else if (priority>19)
				priority = 19;
		} else if (c->args[1][0]=='+') {
			priority = (atoi(c->args[1]+1));
			if (priority<-20)
				priority = -20;
			else if (priority>19)
				priority = 19;
		} else {
			priority = (atoi(c->args[1]));
			if (priority<-20)
				priority = -20;
			else if (priority>19)
				priority = 19;
		}
		pid_t p = fork();
		if (p==0){
			check = setpriority(PRIO_PROCESS, getpid(), priority+20);
			if (check == -1){
				printf("error while using nice command\n");
			}
			if(execvp(c->args[2], &c->args[2]) < 0) 
				perror("error executing command: ");
			exit(0);
		} else {
			wait(5);
		}

	}
}

void cmd_unsetenv(Cmd c) {
	int length;
	char **environment;
	char *temp;
	if(c->args[1] == NULL)
		printf("\n no args avaialable \n");
	else {
		length = strlen(c->args[1]);
		environment = environ;

		while(*environment!=NULL) {
			if(strncmp(*environment,c->args[1],length)==0 && (*environment)[length]=='='){
				temp = *environment;
				while (*environment != NULL) {
					*environment = *(environment+1);
					environment++;
				}
				environment = temp;
			}
			environment++;
		}
	}
}

void cmd_setenv(Cmd c) {
	char **environment;
	int flag=0, length;
	char *env;
	if(c->args[1]==NULL){
		environment = environ;
		while(*environment!=NULL) {
			printf("%s\n", *environment);
			environment++;
		}
		return;
	} else if(c->args[2]==NULL) {
		environment = environ;
		env=(char *)malloc(strlen(c->args[1])+2);
		strcpy(env,c->args[1]);
		strcat(env,"=");
		if (putenv(env) != 0)
			perror("setenv: ");
		return;

	} else {
		env=malloc(strlen(c->args[1])+2+strlen(c->args[2]));
		strcpy(env,c->args[1]);
		strcat(env,"=");
		strcat(env,c->args[2]);
		
		if (putenv(env) != 0)
			perror("setenv: ");
		return;
	}
	printf("setenv: Too many arguments.\n");
	return;
}

void cmd_echo(Cmd c) {
	int i = 1;
	while(c->args[i] != NULL) {
		if(strncmp(c->args[i],"$",1)==0){
			char *ech, *ech_val;
			ech = (char *)malloc(sizeof(strlen(c->args[i])));
			memcpy(ech, c->args[i]+1, strlen(c->args[i])-1);
			ech[strlen(c->args[i])] = '\0';
			ech_val = getenv(ech);
			if (ech_val!=NULL)
				printf("%s ", ech_val);
			else printf("%s: Not defined. ", c->args[i]);
		} else printf("%s ", c->args[i]);
		i++;
	}
	 printf("\n");
	 return;
}

void cmd_cd(Cmd c) {
	if(c->next==NULL) {
		if(c->in==Tin && tin_flag==1){
			if (chdir(inp_text) < 0) {
				perror("change directory failed");
			}
		} else{
			if(c->args[1] == NULL){
				if(chdir(getenv("HOME"))<0)
					printf("cd: home: No such file or directory\n");
			} else {
				if(chdir(c->args[1])<0)
					printf("cd: %s: No such file or directory\n",c->args[1]);
			}
		}

	} else {
		pid_t pid=fork();
		if (pid == 0) {
			if (c->in==Tin && tin_flag==1) {
				if (chdir(inp_text) < 0) {
					perror("change directory failed");
				}
			} else {
				if(c->args[1] == NULL){
					if(chdir(getenv("HOME"))<0)
						printf("cd: home: No such file or directory\n");
				} else {
					if(chdir(c->args[1])<0)
						printf("cd: %s: No such file or directory\n",c->args[1]);
				}
			}
			exit(0);
		} else wait(5);
	}
	return;
}

void cmd_pwd(Cmd c) {
	char curr_dir[1000];
	if(getcwd(curr_dir, 1000) != NULL)
		printf("%s\n",curr_dir);
	else printf("pwd error\n");
	return;
}


void cmd_where(Cmd c){
	if(c->args[1]!=NULL){
		int i = 1, j=0;
		buildpath();
		char *build_cmd;

		while (c->args[i]!=NULL) {
			if(is_builtin(c->args[i])==1) {
				printf("%s is built-in\n", c->args[i]);
			}
			for (j=0; path_list[j]!= NULL; j++) {
				build_cmd=malloc(sizeof(char)*(strlen(path_list[j])+sizeof(c->args[i]))+1);
				strcpy(build_cmd, path_list[j]);
				strcat(build_cmd, "/");
				strcat(build_cmd, c->args[i]);
				if (is_cmd(build_cmd)>0)
					printf("%s\n",build_cmd);
			}
			i++;
		}
	} else
		printf("where [command]\n");
	return;
}

int is_cmd(char *ncmd) {
	int check;
	check = open(ncmd, O_RDONLY,0660);
	close(check);
	if (check>=0)
		return 1;
	else
		return -1;
}

void buildpath() {
  int pathnos=0;
  path=(char*)malloc(500);
  char **environment;
  char *temp;

  environment = environ;
  while (*environment!=NULL) {
   if(strncmp(*environment,"PATH",4)==0 && (*environment)[4]=='='){
      strcpy(path,*environment+5);
      break;
    }
    environment++;
  }
  temp = path;
  int j=0;
  for(; *path; j+= (*path++ == ':'));
  j++; path = temp;
  path_list = (char*)malloc(sizeof(char*) * j);
  
  char * pch; 
  j=0;
  pch = strtok (path,":");
  while (pch != NULL)
  {
    path_list[j]=malloc(sizeof(char)*(strlen(pch)));
    path_list[j]=strndup(pch,strlen(pch));
    j++;
    pch = strtok (NULL, ":");
  }
  path_list[j]=NULL;
}

void prCmd(Cmd c,int input,int output){
	int i,pid,pathnos,j,cmd_exist;
	char *command;
	char **env,**temp_env;
	char *searchenv;
	char *newenv;
	int len;
	int env_found;
	int backup;
	int infile,outfile;
	int file_des;
	int outputffd;

	backup=dup(STDOUT_FILENO);
	int backup_err = dup(STDERR_FILENO);

	tin_flag=0;
	if(c->in==Tin) {
		file_des = open(c->infile,O_RDONLY,0660);
		i=0;
		while(read(file_des, &load, 1) > 0 && load != '\n' && load != '\0') {
			inp_read[i]=load;
			tin_flag=1;
			i++;
		}
		inp_read[i] = '\0';
		inp_text=inp_read;
		close(file_des);
	}

	if (c->out != Tnil)
		switch ( c->out ) {
			case Tout:
				outputffd=open(c->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0660);
				dup2(outputffd, output);
				break;
			case Tapp:
				outputffd=open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0660);
				dup2(outputffd, output);
				break;
			case ToutErr:
				outputffd=open(c->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0660);
				dup2(outputffd, output);
				dup2(outputffd, STDERR_FILENO);
				break;
			case TappErr:
				outputffd=open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0660);
				dup2(outputffd, output);
				dup2(outputffd, STDERR_FILENO);
		}

	if(output!=STDOUT_FILENO)
		dup2(output,STDOUT_FILENO);
	if(c->out == TpipeErr)
		dup2(output, STDERR_FILENO);

	if(strcmp(c->args[0],"where")==0){cmd_where(c);}
	if(strcmp(c->args[0],"pwd")==0){cmd_pwd(c);}
    if(strcmp(c->args[0],"cd")==0){cmd_cd(c);}
    if(strcmp(c->args[0],"echo")==0){cmd_echo(c);}
	if(strcmp(c->args[0],"logout")==0){exit(0);}
	if(strcmp(c->args[0],"setenv")==0){cmd_setenv(c);}
    if(strcmp(c->args[0],"unsetenv")==0) {cmd_unsetenv(c);}
    if(strcmp(c->args[0],"nice")==0) {cmd_nice(c);}
	
	dup2(backup,STDOUT_FILENO);
	dup2(backup_err,STDERR_FILENO);
}

void build_executive (Cmd c, int input, int output){
	int cmd_exist,lastfound;
	int pid;
	
	int i=0, j;
	char **arguments;
	char *build_cmd;
	int inputffd, outputffd;
	int backup_input, backup_output, backup_err;
	backup_input = STDIN_FILENO;
	backup_output = STDOUT_FILENO;
	backup_err = STDERR_FILENO;

	if (c) {

		while(c->args[i]!=NULL) {
			i++;
		}
		arguments = malloc(i*sizeof(char*));

		i = 0;
		while(c->args[i]!=NULL) {
			arguments[i] = malloc(strlen(c->args[i])+1);
			strcpy(arguments[i], c->args[i]);
			i++;
		}
		arguments[i]=NULL;

		pid=fork();
    
    	if(pid==0){
			if(c->in == Tin)
				if((inputffd=open(c->infile, O_RDONLY, 0660))<0)
					printf("\nError on opening input file: %d\n",inputffd);
				else
					dup2(inputffd,input);

			if (c->out != Tnil)
				switch ( c->out ) {
					case Tout:
						outputffd=open(c->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0660);
						dup2(outputffd, output);
						break;
					case Tapp:
						outputffd=open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0660);
						dup2(outputffd, output);
						break;
					case ToutErr:
						outputffd=open(c->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0660);
						dup2(outputffd, output);
						dup2(outputffd, STDERR_FILENO);
						break;
					case TappErr:
						outputffd=open(c->outfile, O_CREAT|O_APPEND|O_WRONLY, 0660);
						dup2(outputffd, output);
						dup2(outputffd, STDERR_FILENO);
				}

			if(output != STDOUT_FILENO){
				close(output-1);
				dup2(output,STDOUT_FILENO);
			}
			if(input != STDIN_FILENO){
				close(input+1);
				dup2(input,STDIN_FILENO);
			}
			if(c->out == TpipeErr) {
				dup2(output, STDERR_FILENO);
			}

			
			if ((strchr(c->args[0],'/')) != NULL) {
				build_cmd = malloc(sizeof(char)*(sizeof(int)+sizeof(c->args[0]))+1);
				strcpy(build_cmd,c->args[0]);
			}
			else {
				buildpath();
				for (j=0; path_list[j]!= NULL; j++) {
					build_cmd=malloc(sizeof(char)*(strlen(path_list[j])+sizeof(c->args[0]))+1);
					strcpy(build_cmd, path_list[j]);
					strcat(build_cmd, "/");
					strcat(build_cmd, c->args[0]);
					if (is_cmd(build_cmd)>0)
						break;
				}
			}

			if(is_cmd(build_cmd) < 0) {
				printf("command not found\n");
				exit(-1);
				return ;
			}
			if(execv(build_cmd,arguments)<0){
				printf("-1");
				perror("command cannot be found...\n");
				exit(-1);
			}
			exit(0);

		}else { //pid>0
			wait(5);
			if(output != STDOUT_FILENO)
				dup2(STDOUT_FILENO,output);

			if(input != STDIN_FILENO){
				if(input == pipe1[0]){
					close(pipe1[1]);
				}
				if(input == pipe2[0]){
					close(pipe2[1]);
				}
				dup2(STDIN_FILENO,input);
			}
			dup2(backup_output, STDOUT_FILENO);
			dup2(backup_err, STDERR_FILENO);
			int exit_status;
			waitpid(pid,&exit_status,NULL);
			if(WIFEXITED(exit_status)==1){
				if(WEXITSTATUS(exit_status)<0){
					printf("Unknown command in Pipe\n");
					niklo=1;
				}
			}
			return;
		}
	}
}


static void prPipe(Pipe p)
{
	int i = 0;
	Cmd c;

  	if ( p == NULL )
    	return;

    int count=0;
    pipe(pipe1);
    pipe(pipe2);

    int input=STDIN_FILENO;
  	int output=STDOUT_FILENO;
  	int serr = STDERR_FILENO;

  	for ( c = p->head; c != NULL; c = c->next ) {
  		if(niklo==1){
  			niklo=0;
  			break;
  		}
  		if(c->next!=NULL) {
  			if(count==0){
  				pipe(pipe1);
  				output=pipe1[1];
  			} else {
  				pipe(pipe2);
  				output=pipe2[1];
  			}
  		} else {
  			output=STDOUT_FILENO;
  		}
  		if ( !strcmp(c->args[0], "end") ) {exit(0);}
  		if(strcmp(c->args[0],"cd")==0||
  			strcmp(c->args[0],"echo")==0||
  			strcmp(c->args[0],"setenv")==0||
  			strcmp(c->args[0],"unsetenv")==0||
  			strcmp(c->args[0],"logout")==0||
  			strcmp(c->args[0],"pwd")==0||
  			strcmp(c->args[0],"where")==0||
  			strcmp(c->args[0],"nice")==0) {

  			if(c->next!=NULL) {
  				prCmd(c,input,output);
  			} else {
  				prCmd(c,STDIN_FILENO,STDOUT_FILENO);
			}
		} else build_executive(c,input,output);

		if(c->next!=NULL) {			
			if(count==0){
				input=pipe1[0];
				count++;
				output=STDOUT_FILENO;
			}
			else{
				input=pipe2[0];
				count--;
				output=STDOUT_FILENO;
			}
		} else {
			output=STDOUT_FILENO;
		}
	}
	prPipe(p->next);
}

void quit_handler(int sign){ 
	 signal(sign, SIG_IGN);
	 printf("\r\n");
	 fflush(STDIN_FILENO);
	 printf("%s%% ",host);
	 fflush(STDIN_FILENO);
	 signal(sign, quit_handler);
}
 void int_handler(int sign){
 	if(child_stat==1){
		printf("\r\n");	
		printf("%s%% ",host);
		fflush(STDIN_FILENO);
	}else{
		printf("\r\n");	
		fflush(STDIN_FILENO);
	}
}

void term_handler(int sign)
{
	signal(sign, SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    killpg(getpgrp(),SIGTERM);
    signal(sign, term_handler);
    exit(0);
}

int main(int argc, char *argv[]) {
	Pipe p;
	int rcfd, input, output, serr;

	//rename all signal functions.
	signal(SIGQUIT, quit_handler);
	signal(SIGINT, int_handler);
	signal(SIGTERM, SIG_IGN);

	char *home_path = getenv("HOME");
	char ush_rc_path[500];
	strcpy(ush_rc_path,home_path);
	strcat(ush_rc_path,"/.ushrc");
	rcfd=open(ush_rc_path, O_RDONLY, 0600);
	if (rcfd==-1) {
		perror("Error opening ushrc file: ");
	}
	input  = dup(STDIN_FILENO);
	output = dup(STDOUT_FILENO);
	serr   = dup(STDERR_FILENO);
	if(rcfd<0){
		printf("USHRC NOT OPEN\n");
	}
	else{
		if (dup2(rcfd, STDIN_FILENO) <0)
			perror("dup2 fail: ");

		p=parse();
		prPipe(p);
		freePipe(p);
		fflush(stdin);
		close(rcfd);
		dup2(input, STDIN_FILENO);
		dup2(output, STDOUT_FILENO);
		dup2(serr, STDERR_FILENO);
		close(input);
	}

	setpgid(0,0); //signals initialization
	while (1){
		child_stat=1;  //signal use

		printf("%s%% ", host);
		fflush(stdout);
		p = parse();
		
		child_stat=0;	//signal use
		
		prPipe(p);
		freePipe(p);
	}
}

/*........................ end of main.c ....................................*/