unsigned char rand_byte();


#define LT_PRESSED 0x02
#define RT_PRESSED 0x01
#define UP_PRESSED 0x08
#define DOWN_PRESSED 0x04
#define B_PRESSED 0x80
#define Y_PRESSED 0x40
#define SL_PRESSED 0x20
#define ST_PRESSED 0x10
unsigned short joystick_get(unsigned char joynum);

void waitforjiffy();

void calc_pills_fall();
