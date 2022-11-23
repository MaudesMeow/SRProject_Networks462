G++_FLAGS= -Wall

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
	if [ -f output.txt ]; then rm output.txt; fi
	./bin/HKMserver.out

# use 'make clean' to remove executable files
clean:
	if [ -f bin/HKMserver.out ]; then rm bin/HKMserver.out; fi
	if [ -f bin/HKMclient.out ]; then rm bin/HKMclient.out; fi

# create a zip file for our project, removing the old one if it exists
archive:
	if [ -f HKMSRP.zip ]; then rm HKMSRP.zip; fi
	zip HKMSRP.zip *.cpp *.hpp makefile 10kbtestfile.in

# create a zip file, then send it to poseidon server
olivertoposeidon: archive
	scp HKMSRP.zip mahano0661@poseidon1.cs.uwec.edu:/USERS/ALL/mahano0661/newCS462

jordantoposeidon: archive
	scp HKMSRP.zip hebertj4194@poseidon0.cs.uwec.edu:/USERS/ALL/HEBERTJ4194/CS462v2

alextoposeidon: archive
	scp HKMSRP.zip Kamkeam5583@poseidon0.cs.uwec.edu:/USERS/ALL/Kamkeam5583/networks/networks2

# remove the old code and replace it from the zip file, then get rid of the zip file
unzip:
	if [ -f HKMSRP.zip ]; then make actuallyunzip; fi

actuallyunzip:
	rm *.cpp *.hpp makefile 10kbtestfile.in
	unzip HKMSRP.zip
	rm HKMSRP.zip



