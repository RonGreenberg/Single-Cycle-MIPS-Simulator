
#define SERVER "127.0.0.1"	// IP address of the virtual screen
#define PORT   9999 // UDP port on which the virtual screen listens to data

void UDP_init(void);
void UDP_terminate(void);

void UDP_send(unsigned char *msg, int msg_size);
