CC = gcc
objdump = objdump --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.data

FLAGS = --static -O0

main : main.o
	$(CC) $(FLAGS) -o main main.o
	$(objdump) $@ > dump

main.o : main.c
	$(CC) $(FLAGS) -c main.c

clean :
	rm main main.o dump
