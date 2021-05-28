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

int main(int argc, char* argv[]) {

    printf("Starting IC Shell\n");
    char* lastCmd[MAXARGS + 1] = { NULL };
	while (1)
	{
		// shell code here
        printf("icsh $ ");

		//Input
		char input[PRMTSIZ + 1] = { 0x0 };
		char* ptr = input;
		char* args[MAXARGS + 1] = { NULL };
		int noArgs = 0;
		
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

        if(strcmp("!!", args[0]) == 0)	//	implementation of !!
        {
            //printing last command before executing it
            int i=0;
            while (lastCmd[i])
            {
                printf("%s ", lastCmd[i]);
                i+=1;
            }
            printf("\n");

            //copying last comd to current aruments
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
            int i=0;
            while (args[i+1])
            {
                i+=1;
                printf("%s ",args[i]);
            }
           printf("\n");
        }
        else if(strcmp("exit", args[0]) == 0)   // implementation of exit
        {
            if(args[1] == NULL)
            {
                printf("Bad Command\n");
            }
            else
            {
                printf("bye\n");
                exit(atoi(args[1]));
            } 
        }
        

        //store last command entered except "!!"
        if (strcmp("!!", args[0]) != 0)
        {
            int i=0;
            //initializing lastCmd array to null
            while (lastCmd[i])
            {
                lastCmd[i] = NULL;
                i+=1;
            }
            //copying the command in lastCmd
            i=0;
            while (args[i])
            {
                lastCmd[i] = strdup(args[i]);
                i+=1;
            }
        }
    }
}