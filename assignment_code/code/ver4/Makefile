target: server client

server: server.o myQueue.o workerfunc.o
	g++ -g server.o myQueue.o workerfunc.o -o server

server.o: server.cpp serverHeaders.h
	g++ -g -c server.cpp -o server.o

myQueue.o: myQueue.cpp serverHeaders.h
	g++ -g -c myQueue.cpp -o myQueue.o

workerfunc.o: workerfunc.cpp serverHeaders.h
	g++ -g -c workerfunc.cpp -o workerfunc.o
	
client : client.cpp
	g++ -g -o client client.cpp
	
clean:
	rm *.o server client
