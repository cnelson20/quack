#ifndef _MAIN_H
#define _MAIN_H

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 16
#define BOARD_HALF_WIDTH 4

extern unsigned char grid[BOARD_HEIGHT][BOARD_WIDTH];
extern unsigned char fall_grid[BOARD_HEIGHT][BOARD_WIDTH];

int main();

void menu();
unsigned char settings_menu();

void results_screen();

void game_loop();
void gen_pill_colors();

void spawn_viruses();

void inc_pill_rot();

unsigned char check_collision(unsigned char x, unsigned char y, unsigned char rot);

void setup_display();
void clear_layer1();
void draw_pillbottle();
void setup_playfield();

void draw_playfield();

void wait_frames(unsigned short);

void write_num_screen(unsigned char x, unsigned char y, unsigned char palette, unsigned char num);
void write_string_screen(unsigned char x, unsigned char y, unsigned char palette, char *string, unsigned char str_length);

void pills_fall();

void clear_pillbottle_interior();

void setup_logo();
void move_logo();
void disable_logo();

void load_graphics();
void load_game_background();
void load_title_background();

void load_bitmap_into_vram(unsigned char startbank);
void load_ram_banks_vram(unsigned char startbank, unsigned char endbank, unsigned short lastaddr);

#endif
