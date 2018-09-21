#include "errorChecks.h"

bool andEndOfLineCheck(struct Token** tokens, int tokenCount){
	//Check if & is anywhere but at the end of the line
	for (int i = 0; i < num_tokens - 1; i++) {
            if (tokens[i]->value[0] == '&') {
                printf("ERROR: & can only occur at the end of a line\n");
                return 1;
            }
    }

    return 0;

}

bool numRedirsCheck(struct Token** tokens, int tokenCount){
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
			return 1;
		}

	return 0;
}