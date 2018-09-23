#include "execute.h"
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include<unistd.h>
#include <fcntl.h> // for open

//IF ONE FAILS DO THEY ALL FAIL TA
//TODO: IF THE & FLAG, run all this in the background

//IDEA: UNTIL PIPES ARE DONE, DO THEM IN A WHILE LOOOOOP


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

	//File redirection in
	int extInFid = 0;

	//Pipe file ids
	int pfids[2];

	
	
	//Keep cycling through commands until we are at the end of the loop
	while(startIndex < tokenCount){
		//Find next metacharacter
		lastMetaIndex = metaIndex;
		metaIndex = findNextMetacharIndex(tokens, *startIndexp, tokenCount);

		//Place all strings up to the metacharacter in an argv array for the execvp
		char** argv = malloc(sizeof(char)*100);

		for(int i = startIndex; i < metaIndex; i++){
			argv[i] = tokens[i]->value;
		}
		//Terminate array
		argv[metaIndex] = NULL;

		//If we've reached the last set of arguments and there are no more metachars
		if(metaIndex >= tokenCount || tokens[metaIndex]->value[0] == '&'){
			if(tokens[lastMetaIndex]->value[0] != '>' && tokens[lastMetaIndex]->value[0] != '<'){
				executeArgs(argv, tokens[startIndex]->value);
			}

			return;
		}


		//If it's a pipe, act accordingly
		else if(tokens[metaIndex]->type == 2){
			//use array of pipe file ids, keep iterating until no more pipes
			pid_t pid;
			int inFid = 0;
			while(pipeCnt <= numPipes){

				//int nextMetaIndex = metaIndex = findNextMetacharIndex(tokens, metaIndex + 1, tokenCount);

				char** argv1 = malloc(sizeof(char)*100);
				for(int i = 0; i < metaIndex-startIndex; i++){
					argv1[i] = tokens[i+ startIndex]->value;
				}
				argv1[metaIndex] = NULL;

				if (pipe(pfids) < 0){
					printf("ERROR: Pipe could not be initialized\n");
					return;
				}

				pid = fork();

				if(pid < 0){
					printf("ERROR: Could not fork a new process\n");
					return;
				}else if(pid == 0){
					//Child process

					//Get input from last command OR last pipe
					if(extInFid > 0 && pipeCnt == 0){
						if(dup2(extInFid, 0) < 0){
							printf("ERROR: Could not complete pipe\n");
							return;
						}
					}else{
						if(dup2(inFid, 0) < 0){
							printf("ERROR: Could not complete pipe\n");
							return;
						}
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
							printf("OPENING OUT REDIRECT FILE\n");
							outFid = open((tokens[metaIndex + 1]->value), O_RDWR|O_CREAT|O_APPEND, 0600);

							if(dup2(outFid, 1) < 0){
								printf("ERROR: Could not redirect output\n");
								return;
							}

							//After out redirect, nothing else should execute
							startIndex = tokenCount;

						}
					}

					close(pfids[0]);
					execvp(argv1[0], argv1);
					exit(EXIT_FAILURE);
				}else{
					//Parent
					wait(NULL);
					close(pfids[1]);
					close(outFid);
					inFid = pfids[0];

					pipeCnt++;
					startIndex = metaIndex + 1;

					if(pipeCnt != numPipes+1){
						metaIndex = findNextMetacharIndex(tokens, startIndex, tokenCount);
					} 

				}

		}

		continue;

		}
		//If other, act accordingly
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
							printf("TRYING TO PIPE OUTPUT\n");
							if(dup2(extInFid, fileno(stdout)) < 0){
								printf("ERROR: Could not pipe output\n" );
								return;
							}
					
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
					        // Wait for child to terminate
					 wait(NULL);
			} 
			

		}else{
			//Out redir
			//Error check if the next metachar index is not & or null this is incorrect
			if(findNextMetacharIndex(tokens, metaIndex + 1, tokenCount) < tokenCount){
				if(tokens[findNextMetacharIndex(tokens, metaIndex + 1, tokenCount)]->value[0] != '&'){
					printf("ERROR: Out redirection must be the last metacharacter\n");
					return;
				}
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
		        // Wait for child to terminate
		        wait(NULL);  
		    } 

		}

		lastStartIndex = startIndex;
		startIndex = metaIndex + 1;


	}

	//Restore stdin and stdout

	dup2(out_orig, 1);
	close(out_orig);

	dup2(in_orig, 0);
	close(in_orig);


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
        if (execvp(argv[0], argv) < 0) { 
            printf("ERROR: Could not execute command \n"); 
        } 
        exit(0); 
    } else { 
        // Wait for child to terminate
        wait(NULL);  
        return; 
    } 
}

/*

void executeArgsPipe(struct Token** tokens, int *startIndexp, int *metaIndexp, char** argv1, char** argv2){
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
		//Executing  child 1, write at the end
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

	//TODO: Check if next arg is pipe or redir and act accordingly
}

void executeArgsInRedir(struct Token** tokens, int * startIndexp, int *metaIndexp, char** argv1){
	pid_t pid = fork();  
  
    if (pid == -1) { 
        printf("ERROR: Child could not be forked \n"); 
        return; 
    } else if (pid == 0) { 

    	int fd = open((tokens[*metaIndexp + 1]->value), O_RDWR|O_CREAT|O_APPEND, 0600);

		if(dup2(fd, fileno(stdin)) == -1){
			printf("ERROR: Could not redirect stdout\n");
			return;
		}

		close(fd);

        if (execvp(argv1[0], argv1) < 0) { 
            printf("ERROR: Could not execute command \n"); 
        } 

        exit(0); 
    } else { 
        // Wait for child to terminate
        wait(NULL);  
        return; 
    } 
}

void executeArgsOutRedir(struct Token** tokens, int startIndex, int metaIndex, char** argv1){
	//Fork a child
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

        if (execvp(argv1[0], argv1) < 0) { 
            printf("ERROR: Could not execute command \n"); 
        } 

        exit(0); 
    } else { 
        // Wait for child to terminate
        wait(NULL);  
        return; 
    } 
		

}

*/
int findNextMetacharIndex(struct Token** tokens, int startIndex, int tokenCount){
	int metaIndex = startIndex;

	//Find a way to loop this so that we go until there are no more metachars left or no more commands left
	//Else find first metachar
	while(metaIndex < tokenCount && tokens[metaIndex]->type != 2 && tokens[metaIndex]->type != 3){
		metaIndex++;
	}

	return metaIndex;

}







