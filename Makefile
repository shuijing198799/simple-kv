objects = hashmap.o str.o hash.o skiplist.o server.o zmalloc.o epoll.o sortmap.o inet.o

cc = gcc

main : $(objects)
		cc -o main $(objects)

hashmap.o : str.h hash.h tools.h zmalloc.h
skiplist.o : str.h tools.h zmalloc.h
server.o : str.h hash.h skiplist.h tools.h hashmap.h zmalloc.h epoll.h inet.h
epoll.o : zmalloc.h 
sortmap.o : hashmap.h skiplist.h str.h zmalloc.h
str.o : zmalloc.h
inet.o : epoll.h server.h

clean :
	rm main $(objects)
