int sockfd;
int portno;
int n;
char buffer[256];
struct sockaddr_in serverAddressInfo;
struct hostent *serverIPAddress;
void error(char *msg);
int netopen(const char* pathname, int flags);
ssize_t netread(int fildes, void* buf, size_t nbyte);
ssize_t netwrite(int filedes, const void* buf, size_t nbyte);
int netclose(int fd);
int netserverinit(char* hostname);