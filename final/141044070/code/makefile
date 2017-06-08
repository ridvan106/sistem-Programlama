all:
	gcc -c server.c client.c -c -std=c99
	gcc server.o -o server -std=c99 -pthread -lm
	gcc client.o -o client -std=c99 -pthread -lm
clean:
	rm *.o server client	
