#all:	x

simulation:	listdriver.o list.o
	gcc -o simulation list.o listdriver.o

listdriver.o:	listdriver.c
	gcc -c listdriver.c

.PHONY: clean
clean:	
	rm  simulation listdriver.o
