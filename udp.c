#include <stdio.h>
#include <winsock2.h>
#include "udp.h"

#pragma comment(lib,"ws2_32.lib") // Winsock Library

#define BUFLEN 128	//Max length of buffer

struct sockaddr_in si_other;
int s, slen=sizeof(si_other);
unsigned char message[BUFLEN];
WSADATA wsa;

void UDP_init(void)
{
  //Initialise winsock
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
		printf("Failed. Error Code : %d",WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	//create socket
	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		printf("socket() failed with error code : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	//setup address structure
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
} /* UDP_init */


void UDP_terminate(void)
{
  closesocket(s);
	WSACleanup();
} /* UDP_terminate */


void UDP_send(unsigned char *msg, int msg_size)
{
  int res = sendto(s, (char *)msg, msg_size , 0 , (struct sockaddr *)&si_other, slen);
  if (res == SOCKET_ERROR) {
    printf("Socket error detected, reinitializing\n\r");
    UDP_terminate();
    UDP_init();
  }
} /* UDP_send */
