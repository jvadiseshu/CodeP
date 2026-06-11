# Specify the compiler
CC = gcc

# Specify the compiler flags (optional, turns on warnings)
CFLAGS = -Wall -g

# The default target that runs when you just type 'make'
all: myprogram.exe

# Rule to compile the executable
myprogram.exe: main.c
	$(CC) $(CFLAGS) main.c -o myprogram.exe

# Rule to clean up generated files
clean:
	rm -f myprogram.exe

