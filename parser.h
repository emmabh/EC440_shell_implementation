#ifndef PARSER_H
#define PARSER_H

struct Token
{
	int type;
	char* value;
};

int isMetachar(char j);
struct Token* splitString(char* input, char* delims, int newSet);
int parser(char* input, struct Token** tokens);
void classifyTokens(struct Token** tokens, int tokenCount);
int numCommands(struct Token** tokens, int tokenCount);

#endif