
void DRAW_init(void);
void DRAW_terminate(void);

void DRAW_rectangle(unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height);
void DRAW_pixel(unsigned char color, unsigned char x, unsigned char y);
void DRAW_bitmap(unsigned char *bitmap, unsigned char x, unsigned char y, unsigned char width, unsigned char height);
