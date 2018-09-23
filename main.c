#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include "parser.h"
#include "execute.h"
#include<unistd.h>
#include <fcntl.h> // for open
#define DELIM " <>&|\t\r\n\a"

/*Types of tokens
	0 - Command
	1 - Argument
	2 - Pipe
	3 - Metachar
*/

int main(int argc, char** argv){
	int k = 0;

	int out_orig = dup(1);
	int in_orig = dup(0);


	while(k < 1){

		dup2(out_orig, 1);
		close(out_orig);

		dup2(in_orig, 0);
		close(in_orig);

		int numPipes = 0;
		int numInRedirs = 0;
		int numOutRedirs = 0;
		int cmdCount = 0;

		//Check for -n
		char checkN[6];

		//In the case of redirect, input & output files
		char* inFile;
		char* outFile;

		//Flag for background process (&)
		int backgroundFlag = 0;

		
		strcpy(checkN, "-n");
		
		if(argc > 1)
		{
			//check for "-n" if present do not print shell prompt
			if(strcmp(argv[1], checkN) == 0){
				printf("");
				fflush(stdout);
			}else{
				printf("my_shell>");
				fflush(stdout);
			}
		}else{
				printf("my_shell>");
				fflush(stdout);
		}

		char input[512];
		fgets(input, 512, stdin);
		input[strlen(input)-1] = '\0';

		//If nothing entered reprompt
		if(input[0] == '\0'){
			continue;
		}

		//If non alphabetic character is entered reprompt
		char c = input[0];
		if(!(c>='a' && c<='z') || (c>='A' && c<='Z')){
			continue;
		}

		//Parse and make array of tokens with descriptors
		struct Token** tokens = malloc(64 * sizeof(struct Token*));
		int tokenCount = parser(input, tokens);

		//Store number of commands
		cmdCount = numCommands(tokens, tokenCount);

		//Check to make sure at least one token
		if(tokenCount == 0){
			continue;
		}

		//Check if & is anywhere but at the end of the line, print error
		for (int i = 0; i < tokenCount - 1; i++) {
            if (tokens[i]->value[0] == '&') {
                printf("ERROR: & can only occur at the end of a line\n");
                continue;
            }
    }
		
		

		//Check number of redirections, if in or out greater than one, print error 
		for(int j = 0; j < tokenCount; j++){
			if(tokens[j]->value[0] == '>')
				numOutRedirs++;
			else if(tokens[j]->value[0] == '<')
				numInRedirs++;
			else if(tokens[j]->value[0] == '|')
				numPipes++;
		}

		if(numInRedirs > 1 || numOutRedirs > 1){
			printf("ERROR: Too many redirections \n");
			continue;
		}

		//Check for double << >>, print error 
		for(int j = 0; j < tokenCount; j++){
			if(tokens[j]->value[0] == '>'){
				if(!strcmp("<<", tokens[j]->value)){
					printf("ERROR: Too many redirections \n");
					continue;
				}
			}else if(tokens[j]->value[0] == '>'){
				if(!strcmp(">>", tokens[j]->value)){
					printf("ERROR: Too many redirections \n");
					continue;
				}
			}
		}

		//Check if &
		if (tokens[tokenCount - 1]->value[0] == '&') {
            backgroundFlag = 1;
        }

		//Making pipes, children, args & executing them all

		if (tokenCount > 0) {	
      		executeLine(tokens, cmdCount, tokenCount, numPipes, numInRedirs, numOutRedirs, backgroundFlag);
    	}
		


	}	


	return 0;
}
