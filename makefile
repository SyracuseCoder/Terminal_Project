all:
	gcc main.c -o terminal -pthread -lrt
clean:
	rm terminal
