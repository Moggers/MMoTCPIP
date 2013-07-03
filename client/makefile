all: clean client.cpp cell.o player.o  
	g++ -o client.o client.cpp -lncurses

player.o: player.cpp
	g++ -c player.cpp -lncurses

cell.o: cell.cpp
	g++ -c cell.cpp -lncurses

clean:
	rm -rf client.o cell.o main.o player.o server.o
