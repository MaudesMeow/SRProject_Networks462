
G++_FLAGS=

# can 'make client' to only compile the client, or just 'make' to compile both
all: server client
server:
	g++ ${G++_FLAGS} HSTserver.cpp -o bin/server.out
client:
	g++ ${G++_FLAGS} HSTclient.cpp -o bin/client.out

# these commands run either client or server
# eg. 'make runclient' runs the client
runclient:
	./bin/client.out
runserver:
	./bin/server.out

# use 'make clean' to remove executable files
clean:
	if [ -f bin/server.out ]; then rm bin/server.out; fi
	if [ -f bin/client.out ]; then rm bin/client.out; fi



