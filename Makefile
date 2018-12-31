sobjects = hashmap.o str.o hash.o skiplist.o server.o zmalloc.o epoll.o sortmap.o inet.o threadpool.o
cobjects = client.o

cc = gcc

server : $(sobjects)
		cc -o server $(sobjects) -lpthread

hashmap.o : str.h hash.h tools.h zmalloc.h
skiplist.o : str.h tools.h zmalloc.h
server.o : str.h hash.h skiplist.h tools.h hashmap.h zmalloc.h epoll.h inet.h threadpool.h
epoll.o : zmalloc.h
sortmap.o : hashmap.h skiplist.h str.h zmalloc.h
str.o : zmalloc.h
inet.o : epoll.h server.h
threadpool.o : zmalloc.h

client : $(cobjects)
	cc -o client $(cobjects)

cleanall :
	cleanserver cleanclient

cleanserver :
	rm server $(sobjects)

cleanclient :
	rm client $(cobjects)
