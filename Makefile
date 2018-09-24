GCC = gcc

OBJS = parser.o main.o execute.o

default: MyShell

MyShell: $(OBJS)
	$(GCC) -o MyShell $(OBJS)

parser.o: parser.c parser.h
	$(GCC) -c parser.c -o parser.o

execute.o: execute.c execute.h
	$(GCC) -c execute.c -o execute.o

main.o: main.c
	$(GCC) -c main.c -o main.o

clean: 
	rm $(OBJS)
