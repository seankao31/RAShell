OBJ = server.o

all: server

server: $(OBJ)
	g++ -o $@ $+

%.o: %.cpp
	g++ -c -o $@ $<

clean:
	rm -f server $(OBJ)
