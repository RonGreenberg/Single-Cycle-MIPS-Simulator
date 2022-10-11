#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library

#define BUFLEN 64	// Max length of buffer
#define PORT 9999 // The port on which to listen for incoming data

SOCKET s;
struct sockaddr_in server, si_other;
int slen , recv_len;
unsigned char buf[BUFLEN];
WSADATA wsa;

int UDP_init()
{
	slen = sizeof(si_other) ;
	
	//Initialise winsock
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
		printf("Failed. Error Code : %d",WSAGetLastError());
		//exit(EXIT_FAILURE);
    return EXIT_FAILURE;
	}
	
	//Create a socket
	if ((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET) {
		printf("Could not create socket : %d" , WSAGetLastError());
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );
	
	//Bind
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d" , WSAGetLastError());
		//exit(EXIT_FAILURE);
    return EXIT_FAILURE;
	}
  return 0;
} /* UDP_init */


void UDP_terminate(void)
{
	closesocket(s);
	WSACleanup();
} /* UDP_terminate */


unsigned char *UDP_get_msg(void)
{
  recv_len = recvfrom(s, (char *)buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
  return buf;
} /* UDP_get_msg */


int UDP_get_msg_non_blocking(unsigned char **buffer)
{
    struct timeval timeout;
    struct fd_set fds;
    int res;

    *buffer = buf; // setting buffer pointer
    timeout.tv_sec = 0; // 0 seconds
    timeout.tv_usec = 10; // 10 useconds

    // Setup fd_set structure
    FD_ZERO(&fds);
    FD_SET(s, &fds); // Return value: -1: error occurred 0: timed out > 0: data ready to be read
    res = select(0, &fds, 0, 0, &timeout);
    if (res <= 0) {
      return -1; // -1 means an error had occured
    } else {
      recv_len = recvfrom(s, (char *)buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
      if (recv_len <= 0) { // reinitializing UPD server in case of socket error
        UDP_terminate();
        UDP_init();
        return -1;
      }
      return 0; // 0 means a valid message was received
    }
} /* UDP_get_msg_non_blocking */
