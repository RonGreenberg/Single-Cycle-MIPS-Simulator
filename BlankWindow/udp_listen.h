
int UDP_init();
void UDP_terminate(void);

unsigned char *UDP_get_msg(void); // blocking until message is ready (not used here)
int UDP_get_msg_non_blocking(unsigned char **buffer); // non blockign polling of message
