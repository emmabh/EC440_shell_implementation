#include "parser.h"


#ifndef EXECUTE_H
#define EXECUTE_H


void executeLine(struct Token** tokens, int cmdCount, int tokenCount, int numPipes, int numInRedirs, int numOutRedirs, int backgroundFlag);
void executeArgsPipe(struct Token** tokens, int * startIndexp, int *metaIndexp, char** argv, char** argv2);
void executeArgsInRedir(struct Token** tokens);
void executeArgsOutRedir(struct Token** tokens);
void executeArgsInOutRedir(struct Token** tokens);
void executeArgsBackground(struct Token** tokens);
void executeArgs(char** argv, char* cmd);
int findNextMetacharIndex(struct Token** tokens, int startIndex, int tokenCount);




#endif