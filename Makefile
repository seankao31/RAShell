OBJ = server.o serverlib.o
DEPS = serverlib.h

all: server

server: $(OBJ)
	g++ -o $@ $+

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $<

clean:
	rm -f server $(OBJ)
