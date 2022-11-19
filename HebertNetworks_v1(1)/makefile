
G++_FLAGS=

# can 'make client' to only compile the client, or just 'make' to compile both
all: server client
server:
	g++ ${G++_FLAGS} HKMserver.cpp HKMcommon.cpp -o bin/HKMserver.out
client:
	g++ ${G++_FLAGS} HKMclient.cpp HKMcommon.cpp -o bin/HKMclient.out

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

# to create a zip file to send to poseidon
archive:
	zip HKMSRP.zip *.cpp *.h makefile

#can copy this for each of us to make transfer easier
olivertoposeidon: archive
	scp HKMSRP.zip mahano0661@poseidon1.cs.uwec.edu:/USERS/ALL/mahano0661/CS462

