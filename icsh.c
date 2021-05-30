#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h> //for PATH_MAX
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100
#define PRMTSIZ 255
#define MAXARGS 63
int exitCode = 0;

//declared global variable to get access in signal handler and kill child processes
int childpid = -1;

void execute(char* args[])
{
	int childpid = fork();
	int status;
	if(childpid > 0)		//process is parent
	{
		while (wait(&status) != childpid)       // wait for completion to avoid zombie process
        ;
	}
	else if(childpid < 0)
	{
		printf("Error forking child");
	}
	else	//child process
	{
		if(execvp(args[0], args) < 0)
       	{
           	printf("Bad Command\n");
            exit(0);
       	}
	}
}

void out(char* args[], int i)
{
	args[i] = NULL;		//making '>' token null to execute command
	int oFile = open(args[i+1], O_RDWR | O_CREAT | O_APPEND, 0600);	//opening file
	if (oFile==-1)
	{ 
		printf("opening %s\n", args[2]); 		//error if file doesnt open
		return; 
	}
	int save_out = dup(fileno(stdout));		//copying original descriptor
	if (dup2(oFile, fileno(stdout)	) == -1) 	//assigning descrptor stdout '>'
	{ 
		perror("redirection of output failed\n");
		return;
	}
	execute(args);		//execute the command
	fflush(stdout);		
	close(oFile);
	dup2(save_out, fileno(stdout));		//restoring original file descriptor
	close(save_out);
}

void in(char* args[], int i)
{
	args[i] = NULL;		//making '<' token null to execute command
	int oFile = open(args[i+1], O_RDWR | O_CREAT | O_APPEND, 0600);	//opening file
	if (oFile==-1)
	{ 
		printf("opening %s\n", args[2]); 		//error if file doesnt open
		return; 
	}
	int save_out = dup(fileno(stdin));		//copying original descriptor
	if (dup2(oFile, fileno(stdin)) == -1) 	//assigning descrptor stdout '>'
	{ 
		perror("redirection of input failed\n");
		return;
	}
	// execute(args);		//execute the command
	fflush(stdin);		
	close(oFile);
	dup2(save_out, fileno(stdin));		//restoring original file descriptor
	close(save_out);
}

void single_in(char* args[], int i)
{
	args[i] = NULL;		//making '<' token null to execute command
	int oFile = open(args[i+1], O_RDWR | O_CREAT | O_APPEND, 0600);	//opening file
	if (oFile==-1)
	{ 
		printf("opening %s\n", args[2]); 		//error if file doesnt open
		return; 
	}
	int save_out = dup(fileno(stdin));		//copying original descriptor
	if (dup2(oFile, fileno(stdin)) == -1) 	//assigning descrptor stdout '>'
	{ 
		perror("redirection of input failed\n");
		return;
	}
	execute(args);		//execute the command
	fflush(stdin);		
	close(oFile);
	dup2(save_out, fileno(stdin));		//restoring original file descriptor
	close(save_out);
}


int executeCmds(char* args[], char* lastCmd[], int noArgs)
{   
    //For IO redirection
	int outFlag = 0;
	int inFlag = 0;
	int rFlag = 0;
	for(int i=0; i<noArgs; i++)
	{
		if(strcmp(">", args[i]) == 0)
		{
			outFlag = i;
			rFlag++;
		}
		if(strcmp("<", args[i]) == 0)
		{
			inFlag = i;
			rFlag++;
		}
	}
    
    if(strcmp("!!", args[0]) == 0)	//	implementation of !!
    {
        //printing last cmd before executing it
        int i=0;
        while (lastCmd[i])
        {
            printf("%s ", lastCmd[i]);
            i+=1;
        }
        printf("\n");

        //copying last cmd to current arguments
        i=0;
        while (lastCmd[i])
        {
            args[i] = strdup(lastCmd[i]);
            lastCmd[i] = "";
            i+=1;

        }
    }

    if(strcmp("echo", args[0]) == 0)	//	implementation of echo
    {
        if(strcmp("$?", args[1]) == 0 && args[2] == NULL)
        {
            printf("%d\n",exitCode);
        }
        else
        {
            int i=0;
            while (args[i+1])
            {
                i+=1;
                printf("%s ",args[i]);
            }
            printf("\n");
        }
    }
    else if(strcmp("exit", args[0]) == 0)   // implementation of exit
    {
        if(args[1] == NULL)
        {
            printf("Bad Command\n");
        }
        else
        {
            exitCode = atoi(args[1]);
            printf("bye\n");
            return (atoi(args[1]));
        } 
    }
    else if(rFlag == 0)	//if any external cmd is detected
    {
        //printf("Else\n");
        execute(args);
    }
    else if(strcmp(">",args[outFlag]) == 0 && rFlag==1)	//if > is user's input
    {
        out(args, outFlag);
    }
    else if(strcmp("<",args[inFlag]) == 0 && rFlag==1)	//if < is user's input
    {
        single_in(args, inFlag);
    }
    else if(strcmp(">",args[outFlag]) == 0 && rFlag==2 && outFlag < inFlag)	 //if > & < is user's input
    {
        out(args, outFlag);
        if(strcmp("<",args[inFlag]) == 0)	//if < is user's input
        {
            in(args, inFlag);	
        }
        else 
            {
                return -1;
            }
    }
    else if(strcmp("<",args[inFlag]) == 0 && rFlag==2 && outFlag > inFlag)	 //if < & > is user's input
    {
        in(args, inFlag);
        if(strcmp(">",args[outFlag]) == 0)	//if > is user's input
        {
            out(args, outFlag);
        }
        else 
            {
                return -1;
            }
    }
    
    //store last cmd entered except "!!"
    if (strcmp("!!", args[0]) != 0)
    {
        int i=0;
        //initializing lastCmd array to null
        while (lastCmd[i])
        {
            lastCmd[i] = NULL;
            i+=1;
        }
        //copying the cmd in lastCmd
        i=0;
        while (args[i])
        {
            lastCmd[i] = strdup(args[i]);
            i+=1;
        }
    }
    return -1;
}

void handle_SIGINT()    //Signal handler function when CTRL-C is pressed
{
    //to kill child processes
    if(childpid == 0)
    {
        kill(childpid, SIGTERM);
    }
    printf("process killed if any\n");
}

// Signal Handler for SIGTSTP
void sighandler(int sig_num)    //Signal handler function when CTRL-Z is pressed
{
    if (childpid == 0)
    {
        kill(childpid, SIGTSTP);
    }
    printf("Process suspended\n");
}



int main(int argc, char* argv[]) {

    printf("Starting IC Shell\n");
    char* lastCmd[MAXARGS + 1] = { NULL };
    int flag = 1;   //flag to run file commands for once
	while (1)
	{
        //setup the signal handler for CTRL-C
        struct sigaction handler;
        handler.sa_handler = handle_SIGINT;
        handler.sa_flags = SA_RESTART;
        sigaction(SIGINT, &handler, NULL);
        signal(SIGTSTP, sighandler);

		// shell code here
		//Input
		char input[PRMTSIZ + 1] = { 0x0 };
		char* ptr = input;
		char* args[MAXARGS + 1] = { NULL };
		int noArgs = 0;
		
        if(argv[1] != NULL && flag == 1) 
        {
            flag = 0;
            printf("\n");
            
            //read file
            FILE *fp = fopen(argv[1], "r");
            if(fp == NULL)
            {
                perror("Unable to open file!");
                exit(1);
            }
    		//Input
            char line[255];
            char *p = NULL;
            while(fgets(line,PRMTSIZ +1,fp) != NULL)
            {
                char* args[MAXARGS + 1] = { NULL };
		        int noArgs = 0;

                p = strtok(line," ");
                while(p != NULL)
                {
                    //printf("%s ",p); /* your word */
                    args[noArgs] = strdup(p);
                    //printf("%s", args[noArgs]);
                    noArgs += 1;
                    p = strtok(NULL," ");
                }
                //execute cmd of file until 
                if(executeCmds(args, lastCmd, noArgs-1) >= 0)
                {
                    break;
                }
            }
            fclose(fp);
        }

        printf("icsh $ ");
        fgets(input, PRMTSIZ, stdin);

        /*Tokenizing given input*/

        // convert input line to list of arguments	
        for (int i = 0; i < sizeof(args) && *ptr; ptr++) {
            if (*ptr == ' ' || *ptr == '\t') continue;
            noArgs += 1;
            if (*ptr == '\n') break;
            for (args[i++] = ptr; *ptr && *ptr != ' ' && *ptr != '\n'; ptr++);
            *ptr = '\0';
        }

        if(args[0] == NULL)     //loop again if no input (to avoid segmentation fault)
        {
            continue;
        }

        int x = executeCmds(args, lastCmd);
        //return with exitcode
        if(x >=0)
        {
            return x;
        }

    }
}