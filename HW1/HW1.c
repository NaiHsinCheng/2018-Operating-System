#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <fcntl.h>     // open(),close()
#include <unistd.h>    // fork(),pipe(),getcwd()
#include <ctype.h>     // isspace()
#include <sys/wait.h>  // waitpid(),wait()

#define NORMAL 0
#define RE_INPUT 1
#define RE_OUTPUT 2
#define PIPE 3

#define CMD_L 1000 

typedef struct parse_cmd{
	int background;   // 1 means background
	int type;         // how to deal with this cmd
	char* file;
	char** args2;
}CMD;

void read_cmd(char* cmd);
char* skip_space(char* cmd);
void split(char* args[],char* cmd);
CMD parse(char* args[]);
int exe(char* args[],CMD info);

int main()
{
	char cmd[CMD_L];
	char dir[CMD_L];
	char* args[CMD_L];
	CMD info;
	while(1)
	{
		getcwd(dir,CMD_L);        // get the directory
		printf("\x1b[;36;1;1m""[23_Shell]: ~%s >> ""\x1b[0m",dir);
				//[;36;1;1m => cyan_bold color
				//0m => reset color
			//printf("Enter read_cmd()\n");
		read_cmd(cmd);
			//printf("Exit read_cmd()\n");
		if(cmd == NULL) continue;
		if(strcmp(cmd,"exit") == 0) break;
			//printf("Enter split()\n");
		split(args,cmd);
			//printf("Exit split()\n");
			//printf("Enter parse()\n");
		info = parse(args);
			//printf("Exit parse()\n");
			//printf("Enter exe()\n");
		exe(args,info);
			//printf("Exit exe()\n");
	}
	return 0;
}

void read_cmd(char* cmd)
{
	memset(cmd,'\0',sizeof(char)*CMD_L);
	
	if(!fgets(cmd,CMD_L,stdin))
	{
		cmd = NULL;
		return;
	}
		
	skip_space(cmd);
	if(strlen(cmd) == 0)
	{
		cmd = NULL;
		return;
	}
}

char* skip_space(char* cmd)
{
	if(!cmd) 
		return cmd;
	while(isspace(*cmd))      // remove space from head
		++cmd;
		
	char* tail = cmd;
	while(*tail != '\0')
		++tail;
	--tail;
	
	while(isspace(*tail))    // replace spaces with '\0' from tail
	{
		*tail = '\0';
		--tail;
	}
	return cmd;
}

void split(char* args[],char* cmd)
{
	int i = 0;
	memset(args,'\0',sizeof(char *)*CMD_L);
	char* next = strtok(cmd," \t\r\n\a");
	while(next != NULL)
	{
		args[i] = next;
		++i;
		next = strtok(NULL," \t\r\n\a");   // cmd has load into ram
	}
	args[i] = NULL;  // command ends
}

CMD parse(char* args[])
{
	CMD info = {0,NORMAL,NULL,NULL};
	int count = 0;
	int error = 0;
	while(args[count])
		++count;
	--count;
	
	if(strcmp(args[count],"&") == 0)
	{
		info.background = 1;     // background program
		args[count] = NULL;
		--count;
	}
	
	if(strcmp(args[0],">") == 0)    // special condition
		args[0] = "touch\0";
	
	for(int i = 0 ; i <= count ; ++i)
	{
		if(strcmp(args[i],">") == 0)
		{
			info.type = RE_OUTPUT;
			info.file = args[i+1];
		}
		else if(strcmp(args[i],"<") == 0)
		{
			info.type = RE_INPUT;
			info.file = args[i+1];
		}
		else if(strcmp(args[i],"|") == 0)
		{
			char* next_args[CMD_L];
			memset(next_args,'\0',sizeof(char *)*CMD_L);
			info.type = PIPE;
			for(int j = i + 1 ; j <= count ; ++j)
				next_args[j-i-1] = args[j];
			info.args2 = next_args;
		} 
		
		if(!strcmp(args[i],">") || !strcmp(args[i],"<") || !strcmp(args[i],"|"))
		{
			args[i] = NULL;
			++error;
		}
		
		if( error > 1 || 
			(!strcmp(args[0],"cd") && error > 1) ||
			(error == 1 && (info.args2 == NULL && info.file == NULL)) )
		{
			fprintf(stderr,"Error: error command.\n");
			exit(EXIT_FAILURE);
		}
	}
	return info;
}
			
int exe(char* args[],CMD info)
{
	if(strcmp(args[0],"cd") == 0)
	{
		if(chdir(args[1]) == 0)    // change directory
			return 1;
		else
		{
			fprintf(stderr,"Error: Couldn't find the directory, or not exist\n");
			return 0;
		}
	}
	
	int status,file;
	int fd[2];
	pid_t pid,pid2;
	pid = fork();
	if(pid < 0)
	{
		fprintf(stderr,"Error: Can't fork!\n");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)   // child
	{
		if(info.type == PIPE)
		{
			if(pipe(fd) < 0)
			{
				fprintf(stderr,"Error: Can't pipe!\n");
				exit(EXIT_FAILURE);
			}
			
			pid2 = fork();     // for executing pipe command => ls | wc ...
			if(pid2 < 0)
			{
				fprintf(stderr,"Error: Can't fork!\n");
				exit(EXIT_FAILURE);
			}
			else if(pid2 == 0)  // grand-child....?
			{
				printf("Enter pid2\n");
				close(fd[0]);
				dup2(fd[1],STDOUT_FILENO);  // redirect STDOUT_FILENO to fd[1]
				close(fd[1]);               // STDOUT_FILENO has pointed to fd[1]
				
				if(execvp(args[0],args) == -1)  // execute cmd
				{
					fprintf(stderr,"Error: > %s < command can't found!\n",args[0]);
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				printf("Enter pid\n");
				waitpid(pid2,&status,0);    // wait pid2 finish
				close(fd[1]);
				dup2(fd[0],STDIN_FILENO);   // redirect STDIN_FILENO to fd[0]
				close(fd[0]);               // STDIN_FILENO has pointed to fd[0]
			
				if(execvp(info.args2[0],info.args2) == -1)
				{
					fprintf(stderr,"Error: > %s < command can't found!\n",info.args2[0]);
					exit(EXIT_FAILURE);
				}
			}
		}
		else
		{
			if(info.type == RE_INPUT)
			{
				file = open(info.file,O_RDONLY);   // open read-only file
				dup2(file,STDIN_FILENO);        // redirect STDIN_FILENO to file
				close(file);
			}
			else if(info.type == RE_OUTPUT)
			{
				//open write-only || creat || rewrite || append
				// 0666 => right -rw-rw-r-
				file = open(info.file,O_WRONLY||O_CREAT||O_TRUNC||O_APPEND,0666);
				dup2(file,STDOUT_FILENO);  // redirect STDOUT_FILENO to file
				close(file);
			}
			if(execvp(args[0],args) == -1)
			{
				fprintf(stderr,"Error: > %s < command can't found!\n",args[0]);
				exit(EXIT_FAILURE);
			}
		}
	}
	else
	{
		if(info.background == 1)
			fprintf(stdout,"[PID]: %u\n",pid);
		else
			waitpid(pid,&status,0);
	}

	return 1;
}			

