all:compile
	gcc  timeServer.o -o timeServer  -lm
	gcc  SeeWhat.o -o SeeWhat -lm
	gcc  showResult.o -o showResult -lm

compile:
	gcc -c timeServer.c -std=c99
	gcc -c SeeWhat.c	-std=c99
	gcc -c showResult.c -std=c99
	
clean:
	rm *.o SeeWhat showResult timeServer	 	
