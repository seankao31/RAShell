OBJ = server.o
DEPS = serverlib.h shell.h
FLAGS = -std=c++11

all: server

server: $(OBJ)
	g++ -o $@ $+ $(FLAGS)

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(FLAGS)

clean:
	rm -f server $(OBJ)
