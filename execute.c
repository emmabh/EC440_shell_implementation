#include "execute.h"
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include<unistd.h>

void executeLine(struct Token** tokens, int cmdCount, int tokenCount, int numPipes, int numInRedirs, int numOutRedirs, int backgroundFlag){
	//If no metachars, execute normally
	if(numPipes == 0 && numInRedirs == 0 && numOutRedirs == 0 && backgroundFlag == 0){
		char** argv = malloc(sizeof(char)*100);
		for(int i = 0; i < tokenCount; i++){
			argv[i] = tokens[i]->value;
		}

		//Get command
		char* cmd = tokens[0]->value;

		//Terminate array
		argv[tokenCount + 1] = NULL;

		//Execute command
		executeArgs(argv, cmd);
		printf("\n");

		return;
	}

	//Starting index of the array that we've already processed
	int startIndex = 0;
	int* startIndexp = &startIndex;

	//Starting index to find metacharacter
	int metaIndex = 0;
	int *metaIndexp = &metaIndex;

	//Keep cycling through commands until we are at the end of the loop
	//while(start index isnt reached the end????)
		//Find next metacharacter
		*metaIndexp = findNextMetacharIndex(tokens, *startIndexp, tokenCount);

		//Place all strings up to the metacharacter in an argv array for the execvp
		char** argv1 = malloc(sizeof(char)*100);
		char** argv2 = malloc(sizeof(char)*100);

		for(int i = startIndex; i < *metaIndexp; i++){
			argv1[i] = tokens[i]->value;
		}
		//Terminate array
		argv1[*metaIndexp] = NULL;

		//If we've reached the last set of arguments and there are no more metachars
		if(*metaIndexp >= tokenCount){
			executeArgs(argv1, tokens[*startIndexp]->value);
		}

		//If it's a pipe, act accordingly
		if(tokens[metaIndex]->type == 2){
			//Find the next metachar as well, and make a second argv array
		}
		//If other, act accordingly
		else if(tokens[metaIndex]->type == 3){
			//&
			if(tokens[metaIndex]->value[0] == '&'){

			// In and out redirect
			}else if(tokenCount > 3 && tokens[metaIndex]->value[0] == '<' && tokens[tokenCount - 2]->value[0] == '>'){
				//Find the next metachar as well, and make a second argv array

			//<
			}else if(tokens[metaIndex]->value[0] == '<'){
				//Find the next metachar as well, and make a second argv array

			//>
			}else{
				//Find the next metachar as well, and make a second argv array

			}

		}else{
			//Iterating through, no metachars left
		}

		//*startIndexp += *metaIndexp do within functions BUT we have to figure out how to do multiple pipes

	//}close while loop

	return;

}

void executeArgs(char** argv, char* cmd) 
{ 
    //Fork a child
    pid_t pid = fork();  
  
    if (pid == -1) { 
        printf("ERROR: Child could not be forked \n"); 
        return; 
    } else if (pid == 0) { 
        if (execvp(cmd, argv) < 0) { 
            printf("ERROR: Could not execute command \n"); 
        } 
        exit(0); 
    } else { 
        // Wait for child to terminate
        wait(NULL);  
        return; 
    } 
}

void executeArgsPipe(struct Token** tokens, int * startIndexp, int *metaIndexp, char** argv1, char** argv2){
	int pipefd[2];
	pid_t p1, p2;

	if (pipe(pipefd) < 0){
		printf("ERROR: Pipe could not be initialized\n");
		return;
	}

	p1 = fork();
	if(p1 < 0){
		printf("ERROR: Could not fork\n");
		return;
	}

	if(p1 == 0){
		//Executing  child 1, write at the write end
		close(pipefd[0]);
		dup2(pipefd[1], fileno(stdout));
		close(pipefd[1]);

		if(execvp(tokens[(*startIndexp)]->value, argv1) < 0){
			printf("ERROR: Could not execute command\n");
			exit(0);
		}
	}else{
		//Executing parent
		p2 = fork();

		if(p2 < 0){
			printf("ERROR: Could not fork\n");
			return;
		}

		//Execute child 2 read at end

		if(p2 == 0){
			close(pipefd[1]);
			dup2(pipefd[0], fileno(stdin));
			close(pipefd[0]);

			if(execvp(tokens[(*metaIndexp)+1]->value, argv2) < 0){
				printf("ERROR: Could not execute command\n");
				exit(0);
			}
		}else{
			//Parent waiting for 2 kids
			wait(NULL);
			wait(NULL);
		}

	}
}

void executeArgsInRedir(struct Token** tokens){

}

void executeArgsOutRedir(struct Token** tokens){

}

void executeArgsInOutRedir(struct Token** tokens){

}

void executeArgsBackground(struct Token** tokens){

}

int findNextMetacharIndex(struct Token** tokens, int startIndex, int tokenCount){
	int metaIndex = startIndex;

	//Find a way to loop this so that we go until there are no more metachars left or no more commands left
	//Else find first metachar
	while(tokens[metaIndex]->type != 2 && tokens[metaIndex]->type != 3 && metaIndex < tokenCount){
		metaIndex++;
	}

	return metaIndex;

}