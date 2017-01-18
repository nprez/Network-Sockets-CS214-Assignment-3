#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int sockfd;
int portno;
int n;
char buffer[256];
struct sockaddr_in serverAddressInfo;
struct hostent *serverIPAddress;

void error(char *msg){
	perror(msg);
	exit(0);
}

/* flags must equal O_RDONLY, O_WRONLY, or O_RDWR
 * returns the new file descriptor or -1 in the case of an error
 * properly sets errno when an error occurs
 */
int netopen(const char* pathname, int flags){
	if(flags!=O_RDONLY && flags!=O_WRONLY && flags!=O_RDWR){
		error("ERROR with netopen flags");
		return -1;
	}

	// zero out the message buffer
	bzero(buffer, 256);
	sprintf(buffer, "%c%c%c%c%c%s%c",
			'o',
			flags>>24, (flags<<8)>>24, (flags<<16)>>24, (flags<<24)>>24,
			pathname, '\0');

	errno = 0;
	// try to write it out to the server
	n = write(sockfd, buffer, 5+strlen(pathname)+1);
	
	// if we couldn't write to the server for some reason, complain and exit
	if(n < 0){
		error("ERROR writing to socket");
	}

	// read a message from the server into the buffer
	n = read(sockfd, buffer, 255);
	errno = 0;
	errno |= buffer[0]<<24;
	errno |= buffer[1]<<16;
	errno |= buffer[2]<<8;
	errno |= buffer[3];
	
	// if we couldn't read from the server for some reason, complain and exit
	if(n < 0){
		error("ERROR reading from socket");
	}

	// print out server's message
	printf("%s\n", &buffer[4]);

	if(atoi(buffer) == -1){
		error("ERROR opening file");
	}

	return atoi(&buffer[4]);
}

/* returns the number of bytes read or -1 in the case of an error
 * properly sets errno when an error occurs
 */
ssize_t netread(int fildes, void* buf, size_t nbyte){
	// zero out the message buffer
	bzero(buffer, 256);
	sprintf(buffer, "%c%c%c%c%c%c%c%c%c",
			'r',
			fildes>>24, (fildes<<8)>>24, (fildes<<16)>>24, (fildes<<24)>>24,
			(int)nbyte>>24, ((int)nbyte<<8)>>24, ((int)nbyte<<16)>>24, ((int)nbyte<<24)>>24);

	errno = 0;
	// try to write it out to the server
	n = write(sockfd, buffer, 9+strlen(buf)+1);
	
	// if we couldn't write to the server for some reason, complain and exit
	if(n < 0){
		error("ERROR writing to socket");
	}

	// read a message from the server into the buffer
	n = read(sockfd, buf, 255);
	errno = 0;
	errno |= ((char*)buf)[0]<<24;
	errno |= ((char*)buf)[1]<<16;
	errno |= ((char*)buf)[2]<<8;
	errno |= ((char*)buf)[3];
	
	// if we couldn't read from the server for some reason, complain and exit
	if(n < 0){
		error("ERROR reading from socket");
	}

	// print out server's message
	printf("%s\n", &((char*)buf)[4]);

	if(atoi(buffer) == -1){
		error("ERROR reading file");
	}

	return n;
}

/* returns the number of bytes written or -1 in the case of an error
 * properly sets errno when an error occurs
 */
ssize_t netwrite(int fildes, const void* buf, size_t nbyte){
	// zero out the message buffer
	bzero(buffer, 256);
	sprintf(buffer, "%c%c%c%c%c%c%c%c%c%s%c",
			'w',
			fildes>>24, (fildes<<8)>>24, (fildes<<16)>>24, (fildes<<24)>>24,
			(int)nbyte>>24, ((int)nbyte<<8)>>24, ((int)nbyte<<16)>>24, ((int)nbyte<<24)>>24,
			(char*)buf, '\0');

	errno = 0;
	// try to write it out to the server
	n = write(sockfd, buffer, 9+strlen(buf)+1);
	
	// if we couldn't write to the server for some reason, complain and exit
	if(n < 0){
		error("ERROR writing to socket");
	}

	// read a message from the server into the buffer
	n = read(sockfd, buffer, 255);
	errno = 0;
	errno |= buffer[0]<<24;
	errno |= buffer[1]<<16;
	errno |= buffer[2]<<8;
	errno |= buffer[3];
	
	// if we couldn't read from the server for some reason, complain and exit
	if(n < 0){
		error("ERROR reading from socket");
	}

	// print out server's message
	printf("%s\n", &buffer[4]);

	if(atoi(buffer) == -1){
		error("ERROR writing to file");
	}

	return atoi(&buffer[4]);
}

/* returns 0 on success or -1 on error
 * properly sets errno when an error occurs
 */
int netclose(int fd){
	// zero out the message buffer
	bzero(buffer, 256);
	sprintf(buffer, "%c%c%c%c%c",
			'c',
			fd>>24, (fd<<8)>>24, (fd<<16)>>24, (fd<<24)>>24);

	errno = 0;
	// try to write it out to the server
	n = write(sockfd, buffer, 5);
	
	// if we couldn't write to the server for some reason, complain and exit
	if(n < 0){
		error("ERROR writing to socket");
	}
	
	// read a message from the server into the buffer
	n = read(sockfd, buffer, 255);
	errno = 0;
	errno |= buffer[0]<<24;
	errno |= buffer[1]<<16;
	errno |= buffer[2]<<8;
	errno |= buffer[3];
	
	// if we couldn't read from the server for some reason, complain and exit
	if(n < 0){
		error("ERROR reading from socket");
	}

	// print out server's message
	printf("%s\n", &buffer[4]);

	if(atoi(buffer) == -1){
		error("ERROR closing file");
	}

	return atoi(&buffer[4]);
}

/* verifies that the given host exists
 * returns 0 on success or -1 on error
 * properly sets h_errno when an error occurs
 */
int netserverinit(char* hostname){
	sockfd = -1;
	portno = 8462;
	n = -1;

	// look up the IP address that matches up with the name given - the name given might
	// BE an IP address, which is fine, and store it in the 'serverIPAddress' struct
	serverIPAddress = gethostbyname(hostname);
	if(serverIPAddress == NULL){
		fprintf(stderr,"ERROR, no such host\n");
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	// try to build a socket .. if it doesn't work, complain and exit
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		error("ERROR creating socket");
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	/** We now have the IP address and port to connect to on the server, we have to get    **/
	/** that information into C's special address struct for connecting sockets          **/

	// zero out the socket address info struct .. always initialize!
	bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));

	// set a flag to indicate the type of network address we'll be using 
	serverAddressInfo.sin_family = AF_INET;
	
	// set the remote port .. translate from a 'normal' int to a super-special 'network-port-int'
	serverAddressInfo.sin_port = htons(portno);

	// do a raw copy of the bytes that represent the server's IP address in 
	// the 'serverIPAddress' struct into our serverIPAddressInfo struct
	bcopy((char *)serverIPAddress->h_addr, (char *)&serverAddressInfo.sin_addr.s_addr, serverIPAddress->h_length);

	/** We now have a blank socket and a fully parameterized address info struct .. time to connect **/
	
	// try to connect to the server using our blank socket and the address info struct 
	// if it doesn't work, complain and exit
	if(connect(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0){
		error("ERROR connecting");
		h_errno = HOST_NOT_FOUND;
		return -1;
	}	
	
	/** If we're here, we're connected to the server .. w00t!  **/

	return 0;
}

int main(int argc, char** argv){
	//usage: hostname, filepath
	if(argc!=3){
		error("ERROR with args");
		return -1;
	}

	char* hostname = argv[1];
	char* filepath = argv[2];

	int test = netserverinit(hostname);
	if(test==-1){
		error("GOOFED in main");
		return -1;
	}

	int fd = netopen(filepath, O_RDWR);
	printf("open errno: %i\n", errno);
	char buf1[256];
	char* buf2 = "hello";
	netread(fd, buf1, 10);
	printf("read errno: %i\n", errno);
	netwrite(fd, buf2, 5);
	printf("write errno: %i\n", errno);
	netclose(fd);
	printf("close errno: %i\n", errno);

	return 0;
}
