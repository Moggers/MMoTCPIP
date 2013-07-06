all: clean client.cpp cell.o player.o net.o
	g++ -o client.o client.cpp -lncurses -pthread

player.o: player.cpp
	g++ -c player.cpp -lncurses

cell.o: cell.cpp
	g++ -c cell.cpp -lncurses
net.o: net.cpp
	g++ -c net.cpp -pthread -lncurses

clean:
	rm -rf client.o cell.o main.o player.o net.o
