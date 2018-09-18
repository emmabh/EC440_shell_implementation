#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include "parser.h"
#define DELIM " <>&|\t\r\n\a"

/*Types of tokens
	0 - Command
	1 - Argument
	2 - Pipe
	3 - Metachar
*/

int main(int argc, char** argv){

	printf("my_shell\n");
	while(1){

		char input[512];
		printf("$ ");
		fgets(input, 512, stdin);

		struct Token** tokens = malloc(64 * sizeof(struct Token*));
		int tokenCount = parser(input, tokens);
	}	


	return 0;
}
