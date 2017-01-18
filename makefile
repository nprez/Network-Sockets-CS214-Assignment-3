ALL: client server
client:
	gcc -g -Wall libnetfiles.c -o client.out
server:
	gcc -g -Wall -pthread netfileserver.c -o server.out
clean:
	rm -rf *.out