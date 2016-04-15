CC=g++
DEBUG=
LDFLAGS=-lpthread
SOURCES=server.cpp client.cpp
EXECUTABLES=$(SOURCES:.cpp=)

SERVER_SOURCE=server.cpp
CLIENT_SOURCE=client.cpp

all: clean $(SOURCES) $(EXECUTABLES)

debug: DEBUG+=-g
debug: clean $(SOURCES) $(EXECUTABLES)


$(EXECUTABLES): 
	$(CC) $(DEBUG) $@.cpp -o $@ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES)
