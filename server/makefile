all: server.cpp clean clthread.o
	g++ -o server.o server.cpp -pthread

clthread.o: clthread.cpp
	g++ -c -o clthread.o clthread.cpp -pthread

clean:
	rm -f server.o	
