unsigned char rand_byte();


#define LT_PRESSED 0x02
#define RT_PRESSED 0x01
#define UP_PRESSED 0x08
#define DOWN_PRESSED 0x04
#define A_PRESSED 0x80
#define B_PRESSED 0x40
#define SL_PRESSED 0x20
#define ST_PRESSED 0x10
unsigned short joystick_get(unsigned char joynum);

void waitforjiffy();

void check_matches();
void setup_calc_pills_fall();
void calc_pills_fall();
void calc_falling_pieces();
void make_pieces_fall();

unsigned char find_piece_support(unsigned char x, unsigned char y, unsigned char x2, unsigned char y2);

#define DISPLAY_CURRENT 0
#define DISPLAY_TOP 1
void __fastcall__ display_score(unsigned char);