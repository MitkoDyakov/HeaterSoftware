#ifndef __DISPLAYH__
#define __DISPLAYH__

void lcd_init(void);
void display_raw_image(const char *path);
void display_jpeg_espjpeg(const char *path);

#endif