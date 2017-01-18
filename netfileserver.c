/* A simple server in the internet domain using TCP
 * The port number is passed as an argument
 */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>


struct sockaddr_in serverAddressInfo;	// Super-special secret C struct that holds address info for building our server socket
struct sockaddr_in clientAddressInfo;	// Super-special secret C struct that holds address info about our client socket

struct argstruct{
	char* buffer;
	int newsockfd;
};

void error(char *msg){
	perror(msg);
	exit(1);
}

void* clienthandler(void* args){
	char* buffer = ((struct argstruct*)args)->buffer;
	int newsockfd = ((struct argstruct*)args)->newsockfd;
	while(1){
		// zero out the char buffer to receive a client message
		bzero(buffer, 256);

		// try to read from the client socket
		int n = read(newsockfd, buffer, 255);
		
		// if the read from the client blew up, complain and exit
		if(n < 0){
			error("ERROR reading from socket");
		}

		if(buffer[0]=='o' || buffer[0]=='r' || buffer[0]=='w' || buffer[0]=='c')
			printf("Here is the message: %s\n", buffer);
		char* msg;
		
		errno = 0;
		int newerrno = 0;
		int bytesToSend = 0;

		if(buffer[0]=='o'){
			//open ("%c%d%s", 'o', flags, pathname)
			int flags = 0;
			flags |= buffer[1]<<24;
			flags |= buffer[2]<<16;
			flags |= buffer[3]<<8;
			flags |= buffer[4];

			char* pathname = malloc(strlen(&buffer[5]) + 1);
			int i = 5;
			while(buffer[i]!='\0'){
				pathname[i-5] = buffer[i];
				i++;
			}
			pathname[i] = '\0';

			printf("o flags: %d\n", flags);
			printf("o pathname: %s\n", pathname);

			int ret = open(pathname, flags);
			newerrno = errno;
			if(ret==-1){
				msg = malloc(4 + 3);
				sprintf(msg, "%c%c%c%c%d", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, -1);
			}
			else{
				int digits = 1;
				while(ret/10 > 0){
					ret/=10;
					digits++;
				}
				msg = malloc(4 + digits + 1);
				sprintf(msg, "%c%c%c%c%d", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, ret);
				bytesToSend = 4+digits+1;
			}
			free(pathname);
		}
		else if(buffer[0]=='r'){
			//read ("%c%d%d%s", 'r', filedes, nbyte, buf)
			int filedes = 0;
			filedes |= buffer[1]<<24;
			filedes |= buffer[2]<<16;
			filedes |= buffer[3]<<8;
			filedes |= buffer[4];

			int nbyte = 0;
			nbyte |= buffer[5]<<24;
			nbyte |= buffer[6]<<16;
			nbyte |= buffer[7]<<8;
			nbyte |= buffer[8];
			
			char* buf = malloc(nbyte+1);
			printf("r filedes: %d\n", filedes);
			printf("r nbyte: %d\n", nbyte);
			buf[nbyte] = '\0';

			int ret = read(filedes, buf, nbyte);
			newerrno = errno;
			msg = malloc(4 + strlen(buf) + 1);
			sprintf(msg, "%c%c%c%c%s", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, buf);
			msg[4 + strlen(buf)] = '\0';
			bytesToSend = 4 + strlen(buf) + 1;
			free(buf);
		}
		else if(buffer[0]=='w'){
			//write ("%c%d%d%s", 'w', filedes, nbyte, buf)
			int filedes = 0;
			filedes |= buffer[1]<<24;
			filedes |= buffer[2]<<16;
			filedes |= buffer[3]<<8;
			filedes |= buffer[4];

			int nbyte = 0;
			nbyte |= buffer[5]<<24;
			nbyte |= buffer[6]<<16;
			nbyte |= buffer[7]<<8;
			nbyte |= buffer[8];
			
			char* buf = malloc(strlen(&buffer[9]) + 1);
			int i = 9;
			while(buffer[i]!='\0'){
				buf[i-9] = buffer[i];
				i++;
			}
			buf[i] = '\0';
			printf("w filedes: %d\n", filedes);
			printf("w nbyte: %d\n", nbyte);
			printf("w buf: %s\n", buf);

			int ret = write(filedes, buf, nbyte);
			newerrno = errno;
			if(ret==-1){
				msg = malloc(4 + 3);
				sprintf(msg, "%c%c%c%c%d", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, -1);
				bytesToSend = 4 + 3;
			}
			else{
				int digits = 1;
				while(ret/10 > 0){
					ret/=10;
					digits++;
				}
				msg = malloc(4 + digits + 1);
				sprintf(msg, "%c%c%c%c%d", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, ret);
				bytesToSend = 4 + digits + 1;
			}
			free(buf);
		}
		else if(buffer[0]=='c'){
			//close ("%c%d", 'c', fd)
			int fd = 0;
			fd |= buffer[1]<<24;
			fd |= buffer[2]<<16;
			fd |= buffer[3]<<8;
			fd |= buffer[4];
			printf("c fd: %d\n", fd);

			int ret = close(fd);
			newerrno = errno;
			if(ret==-1){
				msg = malloc(4 + 3);
				sprintf(msg, "%c%c%c%c%d", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, -1);
			}
			else{
				int digits = 1;
				while(ret/10 > 0){
					ret/=10;
					digits++;
				}
				msg = malloc(4 + digits + 1);
				sprintf(msg, "%c%c%c%c%d", newerrno>>24, (newerrno<<8)>>24, (newerrno<<16)>>24, (newerrno<<24)>>24, ret);
				bytesToSend = 4 + digits + 1;
			}
		}
		else{
			//error
			/*msg = malloc(6);
			printf("LOOK HERE |%s|\n", buffer);
			sprintf(msg, "error");*/
			break;
		}

		// try to write to the client socket
		n = write(newsockfd, msg,  bytesToSend);
		free(msg);
		
		// if the write to the client below up, complain and exit
		if(n < 0){
			error("ERROR writing to socket");
		}
	}
	free(buffer);
	free(args);
	return NULL;
}

int main(int argc, char *argv[]){	
	int sockfd = -1;						// file descriptor for our server socket
	int portno = 8462;						// server port to connect to
	int clilen = -1;						// utility variable - size of clientAddressInfo below
	struct argstruct** args;				// char array to store data going to and coming from the socket
	
	// try to build a socket .. if it doesn't work, complain and exit
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		error("ERROR opening socket");
	}
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));
	
	/** We now have the port to build our server socket on .. time to set up the address struct **/

	// zero out the socket address info struct .. always initialize!
	bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));

	// set the remote port .. translate from a 'normal' int to a super-special 'network-port-int'
	serverAddressInfo.sin_port = htons(portno);
	
	// set a flag to indicate the type of network address we'll be using  
	serverAddressInfo.sin_family = AF_INET;
	
	// set a flag to indicate the type of network address we'll be willing to accept connections from
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;

	/** We have an address struct and a socket .. time to build up the server socket **/
	
	// bind the server socket to a specific local port, so the client has a target to connect to      
	if(bind(sockfd, (struct sockaddr *) &serverAddressInfo, sizeof(serverAddressInfo)) < 0){
		error("ERROR on binding");
	}
	
	// set up the server socket to listen for client connections
	listen(sockfd, 5);
	
	// determine the size of a clientAddressInfo struct
	clilen = sizeof(clientAddressInfo);

	args = malloc(sizeof(struct argstruct*));

	int i = 0;

	while(1){
		args = realloc(args, (i+1)*sizeof(struct argstruct*));
		if(args==NULL){
			error("ERROR with realloc");
		}
		args[i] = malloc(sizeof(struct argstruct));
		args[i]->buffer = malloc(256);
		// block until a client connects, when it does, create a client socket
		args[i]->newsockfd = accept(sockfd, (struct sockaddr *) &clientAddressInfo, (unsigned int*)&clilen);
		/** If we're here, a client tried to connect **/
		
		// if the connection blew up for some reason, complain and exit
		if(args[i]->newsockfd < 0){
			error("ERROR on accept");
		}

		pthread_t id;
		pthread_create(&id, NULL, clienthandler, args[i]);
		i++;
	}

	return 0; 
}
