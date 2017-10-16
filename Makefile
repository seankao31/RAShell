OBJ = server.o serverlib.o service.o
DEPS = serverlib.h service.h

all: server

server: $(OBJ)
	g++ -o $@ $+

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $<

clean:
	rm -f server $(OBJ)
