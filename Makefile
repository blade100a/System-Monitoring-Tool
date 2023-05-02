#########################
#Assignment 3: MakeFile
#Harri Pahirathan
#1003371549
#2023-04-07
#########################


# Define the compiler being used
CC = gcc

# The command to create executable
myprog: A3.c functions.c
	${CC} $^ -o $@

# Command to compile source files
.%o: %.c functions.h
	${CC} -c $<

# Clean by removing the created file
.PHONY: clean
clean:
	rm -f *.o
	rm -f myprog
