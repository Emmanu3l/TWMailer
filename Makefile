# Server kompilieren
all: myserver

myserver: myserver.cpp
	g++ -std=c++17 -pthread -o myserver.o myserver.cpp

# Kompilierten Server löschen
clean:
	rm myserver myserver.o
