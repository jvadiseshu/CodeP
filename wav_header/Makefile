# Specify the compiler
CC = gcc

# Specify the compiler flags (optional, turns on warnings)
CFLAGS = -g

# The default target that runs when you just type 'make'
all: myprogram.exe

#sources list
src: main.c

# Rule to compile the executable
myprogram.exe: $(src)
	$(CC) $(CFLAGS) main.c -o myprogram.exe -lwinmm

# Rule to clean up generated files
clean:
	rm -f myprogram.exe
#clean and build
rebuild: clean all
