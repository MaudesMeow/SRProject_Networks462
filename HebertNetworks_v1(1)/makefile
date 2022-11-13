
G++_FLAGS=

# can 'make client' to only compile the client, or just 'make' to compile both
all: server client
server:
	g++ ${G++_FLAGS} HKMserver.cpp -o bin/HKMserver.out
client:
	g++ ${G++_FLAGS} HKMclient.cpp -o bin/HKMclient.out

# these commands run either client or server
# eg. 'make runclient' runs the client
runclient:
	./bin/HKMclient.out
runserver:
	./bin/HKMserver.out

# use 'make clean' to remove executable files
clean:
	if [ -f bin/HKMserver.out ]; then rm bin/HKMserver.out; fi
	if [ -f bin/HKMclient.out ]; then rm bin/HKMclient.out; fi



