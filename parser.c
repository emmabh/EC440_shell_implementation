#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include "parser.h"

#define DELIM " <>&|\t\r\n\a"

int isMetachar(char j){
	if(j == '>' || j == '<' || j == '&' || j == '|'){
		return 1;
	}else{
		return 0;
	}
}

struct Token* splitString(char* input, char* delims, int newSet){
	char* newString = malloc(sizeof(char)*100);

		static int currIndex = 0;

		//If new command
		if(newSet == 0){
			currIndex = 0;
		}

		struct Token* newToken = malloc(sizeof(struct Token));

		if(!input || !delims || input[currIndex] == '\0'){
    		return NULL;
    	}

    	int j = 0, i = currIndex, k = 0;


		while(input[i] != '\0'){
			j = 0;
			while(delims[j] != '\0'){
				if(input[i] != delims[j]){
						j++;
						continue;
				}else{
					//If the delim is a metacharacter, put it in the array and then return the string
					if(isMetachar(delims[j]) == 1){
						//If this is the first character, it is its own string
						if(k == 0){
							newString[0] = delims[j];
							newString[i+1] = '\0';
							currIndex = i + 1;
							input += currIndex;
							

							(*newToken).value = newString;
							(*newToken).type = 3;
							return newToken;
						}
					}

						//Continue to return because metacharacter found & processed
						if(k != 0){
							//Current index doesn't move because next value is 
							currIndex = i;
						}else{
							currIndex = i + 1;
						}

						newString[i] = '\0';
						input = input + currIndex;
						

						(*newToken).value = newString;
						(*newToken).type = 1;
						return newToken;
				}
				j++;
			}
			newString[k] = input[i];
			i++;
			k++;
		}

	//Return if no metacharacters found
	newString[i] = '\0';						//Terminate with null character
	currIndex = i+1;							//Move on to character after null character

	input = input + currIndex;					//Move input pointer to current index

	//Setting new token value & type
	(*newToken).value = newString;				
	(*newToken).type = 1;

	return newToken;
}

//Make this return int with number of tokens, and take in the array of tokens
int parser(char* input, struct Token** tokens){
	int tokenCount = 0;
	int position = 0;
	struct Token* token;

	

	if(!tokens){
		printf("Error \n");
	}


	//New command bool
	int newSet = 0;

	//Start parsing a new command
	token = splitString(input, DELIM, newSet);
	while(token != NULL){
		//Continue parsing until the string terminates
		if(strncmp((*token).value, "\0", 2)){
			tokens[position] = token;
			printf("%s \n", (*token).value);
			position++;
			tokenCount++;
			newSet = 1;
		}

		token = splitString(input, DELIM, newSet);
	}

	//Terminate the array
	tokens[position] = NULL;

	//Define token types
	classifyTokens(tokens, tokenCount);

	//Return the number of tokens
	return tokenCount;

}

void classifyTokens(struct Token** tokens, int tokenCount){
	//iterate through non metachars and see if they are a command or not
	//Change their types

	int prevState = 0;

	const char *metachars = "<>|&";

	int i;

	for(int i = 0; i < tokenCount; i++){
		if(i == 0 || prevState == 2){
			prevState = 0;
			tokens[i]->type = 0;
		}else if(strchr(metachars, (tokens[i]->value)[0])){
			if((tokens[i]->value)[0] == '|'){
				prevState = 2;
				tokens[i]->type = 2;
			}else{
				prevState = 3;
				tokens[i]->type = 3;
			}
		}else{
			prevState = 1;
			tokens[i]->type = 1;
		}
	}
}