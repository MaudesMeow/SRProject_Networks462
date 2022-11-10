output: HSTserver.o HSTclient.o
	g++ HSTserver.o -o server
	g++ HSTclient.o -o client

sender.o: HSTserver.cpp
	g++ -c HSTserver.cpp

receiver.o: HSTclient.cpp
	g++ HSTclient.cpp

clean:
	rm *.o output
