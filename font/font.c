#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <linux/kd.h>

#include "font.h"

#define ST7735S_WIDTH	160
#define ST7735S_HEIGHT	128

typedef unsigned short 	u16;


static u16 display_buf[ST7735S_HEIGHT * ST7735S_WIDTH];


struct lcd_display_info {
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize;
	u16 *fb_mem;
} lcd;


// helper function to 'plot' a pixel in given color
void put_pixel(int x, int y, u16 color){
    if ( x > ST7735S_WIDTH || y > ST7735S_HEIGHT ) {
		return -1;
	}
	/* Set color */
	display_buf[x + (y * ST7735S_WIDTH)] = color;
	return 0;
}
void put_char(int x, int y, char ch, u16 color){
    char *symb_bmp = font8x8_basic[ch]; 
    for (int row = 0; row < FONTH; ++row) {
        for(int col = 0; col < FONTW; ++col) {
            //char pixel = symb_bmp[];
            if((symb_bmp[row] >> col) & 1) {
                put_pixel(x + col, y + row, color);
            }
            else {
                //
            }
        }
    }
}


void put_string(char *str, u16 color){
    int x = 5; int y = 5;
    int length = strlen(str);
    int start_x = x;
    int offset = 0;
    for(int i = 0; i < length; ++i){
        char ch = str[i];
        if(ch == '\n'){
            x = start_x;
            y += 8;
            offset = 0;
        } 
        else{
            put_char(x + FONTW * offset, y, ch, color);
            offset += 1;
        }
    }
}


void clearscreen(void){
	memset(lcd.fb_mem, 0, lcd.screensize * 2);
}
void setbackground(u16 color){
    for(int i = 0; i < ST7735S_HEIGHT * ST7735S_WIDTH; ++i)
        display_buf[i] = color;
}
void update()
{
	for(int i = 0; i < lcd.screensize ; ++i){
		lcd.fb_mem[i] = display_buf[i];
	}
}


int fb;
int fb_init(char *device) {
	// get current settings (which we have to restore)
	if (-1 == (fb = open(device, O_RDWR))) {
		fprintf(stderr, "Open %s error\n", device);
		return 0;
	}
	if (-1 == ioctl(fb, FBIOGET_VSCREENINFO, &lcd.vinfo)) {
		fprintf(stderr, "Ioctl FBIOGET_VSCREENINFO error.\n");
		return 0;
	}
	if (-1 == ioctl(fb, FBIOGET_FSCREENINFO, &lcd.finfo)) {
		fprintf(stderr, "Ioctl FBIOGET_FSCREENINFO error.\n");
		return 0;
	}
	lcd.screensize = lcd.finfo.smem_len / 2;
	//printf("lcd.finfo.smem_len is %d\n", lcd.finfo.smem_len);
	lcd.fb_mem = (u16*)mmap(NULL, lcd.finfo.smem_len, PROT_WRITE, MAP_SHARED, fb, 0);
	if (-1 == (int)lcd.fb_mem) {
		fprintf(stderr, "Mmap error.\n");
		goto err;
	}

	//printf("lcd.width %d\n", lcd.width);
	//printf("lcd.height %d\n", lcd.height);

	return 1;

err:
	if (-1 == ioctl(fb, FBIOPUT_VSCREENINFO, &lcd.vinfo))
		fprintf(stderr, "Ioctl FBIOPUT_VSCREENINFO error.\n");
	if (-1 == ioctl(fb, FBIOGET_FSCREENINFO, &lcd.finfo))
		fprintf(stderr, "Ioctl FBIOGET_FSCREENINFO.\n");
	return 0;
}



// application entry point
int main(int argc, char* argv[])
{
    if (!fb_init("/dev/fb0"))
		exit(1);

    setbackground(rand());
    put_string(argv[1], rand());
    update(); 
    //Graphic_ClearScreen();
    return 0;
  
}

