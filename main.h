#ifndef _MAIN_H
#define _MAIN_H

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 16
#define BOARD_HALF_WIDTH 4

extern unsigned char grid[BOARD_HEIGHT][BOARD_WIDTH];
extern unsigned char fall_grid[BOARD_HEIGHT][BOARD_WIDTH];

extern unsigned short duck_animation_offsets[];
extern unsigned short virus_animation_offsets[];

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
void setup_playfield();

void draw_playfield();

void wait_frames(unsigned short);

void write_num_screen(unsigned char x, unsigned char y, unsigned char palette, unsigned char num);
void write_string_screen(unsigned char x, unsigned char y, unsigned char palette, char *string, unsigned char str_length);

void __fastcall__ pills_fall(unsigned char first_one);

void clear_pillbottle_interior();

void setup_logo();
void disable_sprites();

void set_duck_throw();
void setup_game_sprites();
void animate_viruses();

void update_virus_count();
void print_stats();

extern unsigned char pill_toss_timer;
unsigned char animate_pill_toss();

extern unsigned short duck_animation_offsets[];

void load_graphics();
void load_game_background();
void setup_title_background();
void load_title_background();
void animate_menu_background();

void load_ram_banks_vram(unsigned char startbank, unsigned char endbank, unsigned short lastaddr);

#endif
