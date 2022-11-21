
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
	./bin/HKMserver.out

# use 'make clean' to remove executable files
clean:
	if [ -f bin/HKMserver.out ]; then rm bin/HKMserver.out; fi
	if [ -f bin/HKMclient.out ]; then rm bin/HKMclient.out; fi

# create a zip file for our project, removing the old one if it exists
archive:
	if [ -f HKMSRP.zip ]; then rm HKMSRP.zip; fi
	zip HKMSRP.zip *.cpp *.h makefile 10kbtestfile.in

# create a zip file, then send it to poseidon server
olivertoposeidon: archive
	scp HKMSRP.zip mahano0661@poseidon1.cs.uwec.edu:/USERS/ALL/mahano0661/CS462

jordantoposeidon: archive
	scp HKMSRP.zip {JORDANS_UWEC_ID}@poseidon0.cs.uwec.edu:/USERS/ALL/{JORDANS_UWEC_ID}/{SOME_OTHER_FOLDER_IF_YOU_WANT}

alextoposeidon: archive
	scp HKMSRP.zip {ALEXS_UWEC_ID}@poseidon0.cs.uwec.edu:/USERS/ALL/{ALEXS_UWEC_ID}/{SOME_OTHER_FOLDER_IF_YOU_WANT}

# remove the old code and replace it from the zip file, then get rid of the zip file
unzip:
	if [ -f HKMSRP.zip ]; then make actuallyunzip; fi

actuallyunzip:
	rm *.cpp *.h makefile 10kbtestfile.in
	unzip HKMSRP.zip
	rm HKMSRP.zip


