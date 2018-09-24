#include "parser.h"


#ifndef EXECUTE_H
#define EXECUTE_H


void executeLine(struct Token** tokens, int cmdCount, int tokenCount, int numPipes, int numInRedirs, int numOutRedirs, int backgroundFlag);
void executeArgs(char** argv, char* cmd, int backgroundFlag);
int findNextMetacharIndex(struct Token** tokens, int startIndex, int tokenCount);




#endif