#include "execute.h"
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include<unistd.h>
#include <fcntl.h> // for open


//TODO: IF THE & FLAG, run all this in the background should be good


void executeLine(struct Token** tokens, int cmdCount, int tokenCount, int numPipes, int numInRedirs, int numOutRedirs, int backgroundFlag){
	//If no metachars, execute normally
	if(numPipes == 0 && numInRedirs == 0 && numOutRedirs == 0){

		char** argv = malloc(sizeof(char)*100);
		for(int i = 0; i < tokenCount; i++){
			argv[i] = tokens[i]->value;
		}

		//Get command
		char* cmd = tokens[0]->value;

		//Terminate array
		argv[tokenCount + 1] = NULL;

		//Execute command
		executeArgs(argv, cmd, backgroundFlag);
		printf("\n");

		return;
	}

	int out_orig = dup(1);
	int in_orig = dup(0);
	//Starting index of the array that we've already processed
	int startIndex = 0;
	int* startIndexp = &startIndex;

	int lastStartIndex = 0;

	//Starting index to find metacharacter
	int metaIndex = 0;
	int *metaIndexp = &metaIndex;

	int lastMetaIndex = 0;

	//Number of pipes seen so far
	int pipeCnt = 0;

	//Out redirection file id
	int outFid;

	//File id if in redirection leads into a pipe
	int extInFid = 0;

	//Pipe file ids
	int pfids[2];

	
	//Keep cycling through commands until we are at the end of the loop
	while(startIndex < tokenCount){
		//Find next metacharacter
		lastMetaIndex = metaIndex;
		metaIndex = findNextMetacharIndex(tokens, *startIndexp, tokenCount);

		//Place all strings up to the metacharacter in an argv array for execution
		char** argv = malloc(sizeof(char)*100);

		for(int i = startIndex; i < metaIndex; i++){
			argv[i] = tokens[i]->value;
		}
		//Terminate array
		argv[metaIndex] = NULL;

		//If reached the last set of arguments and there are no more metachars
		if(metaIndex >= tokenCount){
			if(tokens[lastMetaIndex]->value[0] != '>' && tokens[lastMetaIndex]->value[0] != '<'){
				executeArgs(argv, tokens[startIndex]->value, backgroundFlag);
			}

			return;
		}


		//If the next metacharacter is a pipe
		else if(tokens[metaIndex]->type == 2){
			pid_t pid;
			int inFid = 0;

			//Handle multiple pipes in a row
			while(pipeCnt <= numPipes){

				//Open pipe
				if (pipe(pfids) < 0){
					printf("ERROR: Pipe could not be initialized\n");
					return;
				}

				//Put all arguments into an array for execution
				char** argv1 = malloc(sizeof(char)*100);
				for(int i = 0; i < metaIndex-startIndex; i++){
					argv1[i] = tokens[i+ startIndex]->value;
				}
				//Terminate array
				argv1[metaIndex] = NULL;

				//Fork a new process
				pid = fork();

				if(pid < 0){
					printf("ERROR: Could not fork a new process\n");
					return;
				}else if(pid == 0){
					//Child process

					//Get input from last command OR last pipe (If first command, inFid is 0)
					if(extInFid > 0 && pipeCnt == 0){
						inFid = extInFid;
					}
					
					if(dup2(inFid, 0) < 0){
						printf("ERROR: Could not complete pipe\n");
						return;
					}
					
					

					//If not the last pipe
					if(pipeCnt < numPipes){
						if(dup2(pfids[1], 1) < 0){
							printf("ERROR: Could not complete pipe\n");
							return;
						}
					}else{
						//If it is the last pipe
						close(pfids[1]);

						//If there is an out redirection after the last pipe
						if(numOutRedirs == 1){
							outFid = open((tokens[metaIndex + 1]->value), O_RDWR|O_CREAT|O_APPEND, 0600);

							if(dup2(outFid, 1) < 0){
								printf("ERROR: Could not redirect output\n");
								return;
							}

							//After out redirect, nothing else should execute (Hard coded)
							startIndex = tokenCount;

						}
					}

					close(pfids[0]);
					execvp(argv1[0], argv1);
					exit(EXIT_FAILURE);
				}else{
					//Parent
					//If the command is not being done in the background
					if(backgroundFlag!= 1){
						wait(NULL);
					}
					
					//Close end of pipe not using
					close(pfids[1]);
					close(outFid);
					inFid = pfids[0];

					pipeCnt++;
					startIndex = metaIndex + 1;

					//Find next meta index if there are more pipes to come
					if(pipeCnt != numPipes+1){
						metaIndex = findNextMetacharIndex(tokens, startIndex, tokenCount);
					} 

				}

		}

		continue;

		}
		//If metacharacter other than pipe, act accordingly
		else if(tokens[metaIndex]->type == 3 && tokens[metaIndex]->value[0] == '<'){
			if(startIndex != 0){
				printf("ERROR: In redirection needs to be the first argument\n");
				return;
			}
			
			//In & out redirect
			if(findNextMetacharIndex(tokens, metaIndex + 1, tokenCount) < tokenCount){
					//If next one out redirect
						if(tokens[findNextMetacharIndex(tokens, metaIndex + 1, tokenCount)]->value[0] == '>'){
							outFid = open((tokens[findNextMetacharIndex(tokens, metaIndex + 1, tokenCount) + 1]->value), O_RDWR|O_CREAT|O_APPEND, 0600);

							if(dup2(outFid, fileno(stdout)) < 0){
								printf("ERROR: Could not redirect output\n" );
								return;
							}

							close(outFid);
						}
						//If next one pipe, put output of this file into the input of the pipe
						else if(tokens[findNextMetacharIndex(tokens, metaIndex + 1, tokenCount)]->value[0] == '|'){
							if(dup2(extInFid, fileno(stdout)) < 0){
								printf("ERROR: Could not pipe output\n" );
								return;
							}

							close(extInFid);

							//SHORTCOMING: This hasn't worked yet (cat: stdin: bad file descriptior)
							//Adjust start index to the pipe + 1 & adjust number of pipes because we're already taking care of one
							startIndex = findNextMetacharIndex(tokens, metaIndex + 1, tokenCount) + 1;
							numPipes--;
					
						}

						
			}
			//Normal execution of input redirection

			pid_t pid = fork();  
  
			if (pid == -1) { 
					 printf("ERROR: Child could not be forked \n"); 
					        return; 
			} else if (pid == 0) { 

					int fd = open((tokens[metaIndex + 1]->value), O_RDWR|O_CREAT|O_APPEND, 0600);

					if(dup2(fd, fileno(stdin)) == -1){
						printf("ERROR: Could not redirect stdin\n");
						return;
					}

					close(fd);

					if (execvp(argv[0], argv) < 0) { 
						printf("ERROR: Could not execute command \n"); 
						return;
					} 

					exit(0); 
			} else { 
					 // Wait for child to terminate if not being done in the background

					if(backgroundFlag != 1){
						 wait(NULL);
					}
			} 
			

		}else{
			//Out redir
			//Error check if the next metachar index is not null this is incorrect
			if(findNextMetacharIndex(tokens, metaIndex + 1, tokenCount) < tokenCount){
					printf("ERROR: Out redirection must be the last metacharacter\n");
					return;
			}

			if(numPipes > 0 || numInRedirs == 1){
		    		lastStartIndex = startIndex;
					startIndex = metaIndex + 1;
					return;
			}

			pid_t pid = fork();  
		  
		    if (pid == -1) { 
		        printf("ERROR: Child could not be forked \n"); 
		        return; 
		    } else if (pid == 0) { 

			    	int fd = open((tokens[metaIndex + 1]->value), O_RDWR|O_CREAT|O_APPEND, 0600);

					if(dup2(fd, fileno(stdout)) == -1){
						printf("ERROR: Could not redirect stdout\n");
						return;
					}

					close(fd);

			    if (execvp(argv[0], argv) < 0) { 
			        printf("ERROR: Could not execute command \n"); 
			        return;
			    } 

			    exit(0); 
			    
		    } else { 
		        // Wait for child to terminate if not being done in the background
		        if(backgroundFlag != 1){
		        	wait(NULL); 
		        }
		         
		    } 

		}


		startIndex = metaIndex + 1;


	}

	//Restore stdin and stdout
	dup2(out_orig, 1);
	close(out_orig);

	dup2(in_orig, 0);
	close(in_orig);


	return;

}

void executeArgs(char** argv, char* cmd, int backgroundFlag) 
{ 
    //Fork a child
    pid_t pid = fork();  
  
    if (pid == -1) { 
        printf("ERROR: Child could not be forked \n"); 
        return; 
    } else if (pid == 0) { 
        if (execvp(argv[0], argv) < 0) { 
            printf("ERROR: Could not execute command \n"); 
        } 
        exit(0); 
    } else { 
        // Wait for child to terminate
        if(backgroundFlag != 1){
        	wait(NULL);  
        }
        
        return; 
    } 
}


int findNextMetacharIndex(struct Token** tokens, int startIndex, int tokenCount){
	int metaIndex = startIndex;

	//Find a way to loop this so that we go until there are no more metachars left or no more commands left
	//Else find first metachar
	while(metaIndex < tokenCount && tokens[metaIndex]->type != 2 && tokens[metaIndex]->type != 3){
		metaIndex++;
	}

	return metaIndex;

}







