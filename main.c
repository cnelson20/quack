#include <peekpoke.h>
#include <cbm.h>
#include <cx16.h>
#include <stdlib.h>
#include "main.h"
#include "routines.h"

#define NO_TILE 0
#define BLUE_TILE 1
#define RED_TILE 2
#define YELLOW_TILE 3

#define PILL_ANIM_1 0x11
#define PILL_ANIM_2 0x12

#define DIFF_EASY 0
#define DIFF_MED 1
#define DIFF_HARD 2

#define KEY_REPEAT_FRAMES 6
#define MENU_REPEAT_FRAMES 8

#define DOWN_PRESS_SPEED 7

#define CASCADE_FALL_FRAMES 24
#define VIRUS_DRAW_FRAMES 6
#define FRAMES_PER_SECOND 60

unsigned char joystick_num;

unsigned char grid[BOARD_HEIGHT][BOARD_WIDTH];
unsigned char fall_grid[BOARD_HEIGHT][BOARD_WIDTH];

unsigned char frames_fall_start_indexes[] = {15, 25, 31};
unsigned char frames_fall_table[] = {
        70, 68, 66, 64, 62, 60, 58, 56, 54, 52, 50, 48, 46, 44, 42,
        40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5,
        4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1};

unsigned char up_cool;
unsigned char down_cool;
unsigned char right_cool;
unsigned char left_cool;

unsigned char next_pill_colors[2];

unsigned char pill_is_falling = 0;
unsigned char pill_colors[2];
unsigned char pill_x;
unsigned char pill_y;
unsigned char pill_rot;
unsigned char last_chance;

unsigned char level;
unsigned char difficulty = DIFF_EASY;
unsigned char music_on = 1;

unsigned char pill_num;
unsigned char num_viruses_alive;
unsigned char num_speed_ups;
unsigned char frames_fall_index;

unsigned char frame_count;
unsigned char frame_tick;

#define TIME_MINUTES 2
#define TIME_SECONDS 1
#define TIME_JIFFIES 0
unsigned char game_time_units[3];

unsigned char game_paused;

unsigned char game_has_started = 0;
unsigned char game_is_over;
unsigned char player_won;

void setup_rand();

int main() {
    ROM_BANK = 0;
    joystick_num = 0;

    load_graphics();
    setup_display();

    level = 0;
    difficulty = DIFF_EASY;
    game_is_over = 0;

    setup_rand();

    while (1) {
        menu();
        game_loop();
        results_screen();
    }
    __asm__ ("lda #$80");
    __asm__ ("sta $9F25");
    __asm__ ("stz $01");
    __asm__ ("jmp ($FFFC)");
    return 0;
}

void game_setup() {
    static unsigned char j,i;

    game_has_started = 0;
    game_is_over = 0;
    pill_is_falling = 0;
    game_paused = 0;

    frame_tick = 0;
    frames_fall_index = frames_fall_start_indexes[difficulty];
    num_speed_ups = 0;

    for (i = 0; i < 3; ++i) {
        game_time_units[i] = 0;
    }

    last_chance = 0;

    left_cool = 0;
    right_cool = 0;
    up_cool = 0;

    for (j = 0; j < BOARD_HEIGHT; ++j) {
        for (i = 0; i < BOARD_WIDTH; ++i) {
            grid[j][i] = NO_TILE;
        }
    }
    draw_playfield();

    spawn_viruses();
    gen_pill_colors();
}



void menu() {
    static unsigned char start_game;

    load_title_background();
    do {
        // First time, go to logo
        if (!game_is_over) {
            clear_layer1();
            setup_logo();
            write_string_screen(14, 20, 0, "press  start", 12);

            // wait for no key presses
            while ((joystick_get(joystick_num) & 0xff) != 0xff) {
                move_logo();
                waitforjiffy();
            }
            start_game = 0xFF;
            do {
                static unsigned char i;
                for (i = 0; i < 5; ++i) {
                    start_game = start_game & joystick_get(joystick_num);
                    move_logo();
                    waitforjiffy();
                }
            } while (start_game & ST_PRESSED);
            disable_logo();
        }
        clear_layer1();

        game_is_over = 0;
        start_game = settings_menu();
        disable_logo();
    } while (start_game == 0);

}

#define M40(c) (c - 0x40)
#define LEVEL_STRING_LENGTH 5
char level_string[] = {'l', M40('e'), M40('v'), M40('e'), M40('l')};

#define DIFF_STRING_LENGTH 10
char difficulty_string[] = {'d', M40('i'), M40('f'), M40('f'), M40('i'), M40('c'), M40('u'), M40('l'), M40('t'), M40('y')};

#define MUSIC_STRING_LENGTH 5
char music_string[] = {'m', M40('u'), M40('s'), M40('i'), M40('c')};

char music_on_off_strings[][4] = {
        "off",
        "on ",
};
char difficulty_strings[][8] = {
        "  easy ",
        " medium",
        "  hard ",
};
unsigned char menu_row_maxs[3] = {20, DIFF_HARD, 0};
unsigned char menu_row_mins[3] = {0, DIFF_EASY, 1};

void setup_settings_background();

void display_settings() {
    write_num_screen(26, 7, 0, level);
    write_string_screen(23, 15, 0, difficulty_strings[difficulty], 7);
    write_string_screen(26, 23, 0, music_on_off_strings[music_on], 3);
}

unsigned char settings_menu() {
    static unsigned char joystick_input;
    static unsigned char menu_row;

    setup_settings_background();

    menu_row = 0;

    left_cool = 0;
    right_cool = 0;
    up_cool = 0;
    down_cool = 0;

    write_string_screen(11, 7, 0, level_string, LEVEL_STRING_LENGTH);
    write_string_screen(6, 15, 0, difficulty_string, DIFF_STRING_LENGTH);
    write_string_screen(11, 23, 0, music_string, MUSIC_STRING_LENGTH);
    display_settings();
    while ((joystick_get(joystick_num) & 0xff) != 0xff);
    while (1) {
        static unsigned char temp, i;
        display_settings();

        temp = (menu_row) ? (menu_row == 1 ? difficulty : music_on) : level;
        for (i = 0; i < 3; ++i) {
            if (i == menu_row && menu_row_mins[menu_row] != temp) {
                write_string_screen(22, 7 + (menu_row << 3), 0, "<", 1);
            } else {
                POKE(0x9F20, 22 << 1);
                POKE(0x9F21, 7 + (i << 3));
                POKE(0x9F23, 0xA0);
            }
            if (i == menu_row && menu_row_maxs[menu_row] != temp) {
                write_string_screen(31, 7 + (menu_row << 3), 0, ">", 1);
            } else {
                POKE(0x9F20, 31 << 1);
                POKE(0x9F21, 7 + (i << 3));
                POKE(0x9F23, 0xA0);
            }
        }

        waitforjiffy();

        joystick_input = joystick_get(joystick_num);
        if (!right_cool && !(joystick_input & RT_PRESSED)) {
            if (menu_row == 0 && level < 20) {
                ++level;
            } else if (menu_row == 1 && difficulty < DIFF_HARD) {
                ++difficulty;
            } else if (menu_row == 2 && music_on) {
                music_on = 0;
            }
            right_cool = MENU_REPEAT_FRAMES;
        } else if (right_cool) {
            --right_cool;
        }

        if (!left_cool && !(joystick_input & LT_PRESSED)) {
            if (menu_row == 0 && level > 0) {
                --level;
            } else if (menu_row == 1 && difficulty > DIFF_EASY) {
                --difficulty;
            } else if (menu_row == 2 && !music_on) {
                music_on = 1;
            }
            left_cool = MENU_REPEAT_FRAMES;
        } else if (left_cool) {
            --left_cool;
        }

        if (!up_cool && !(joystick_input & UP_PRESSED)) {
            if (menu_row) {
                --menu_row;
            } else {
                menu_row = 2;
            }
            up_cool = MENU_REPEAT_FRAMES;
        } else if (up_cool) {
            --up_cool;
        }

        if (!down_cool && !(joystick_input & DOWN_PRESSED)) {
           if (++menu_row >= 3) {
               menu_row = 0;
           }
           down_cool = MENU_REPEAT_FRAMES;
        } else if (down_cool) {
            --down_cool;
        }
        if (!(joystick_input & B_PRESSED)) {
            return 0;
        }
        if (!(joystick_input & ST_PRESSED)) {
            return 1;
        }
    }
}

void results_screen() {
    clear_pillbottle_interior();
    if (player_won) {
        // win
        write_string_screen(18, 12, 0, "level", 5);
        write_string_screen(17, 13, 0, "clear!", 6);

        write_string_screen(18, 15, 0, "time", 4);
        write_string_screen(19, 16, 0, ":", 1);
        write_num_screen(17, 16, 0, game_time_units[2]);
        if (game_time_units[2] < 10) {
            POKE(0x9F20, 17 << 1);
            POKE(0x9F21, 16);
            POKE(0x9F22, 0x00);
            POKE(0x9F23, 0xA0);
        }
        write_num_screen(20, 16, 0, game_time_units[1]);


        ++level;
    } else {
        write_string_screen(18, 12, 0, "game", 4);
        write_string_screen(18, 13, 0, "over", 4);
        // game over
    }
    write_string_screen(17, 20, 0, "press", 5);
    write_string_screen(17, 21, 0, "start", 5);
    while (joystick_get(joystick_num) & ST_PRESSED) { waitforjiffy(); }
    while (!(joystick_get(joystick_num) & ST_PRESSED)) { waitforjiffy(); }
}

void game_loop() {
    static unsigned char temp;
    static unsigned char down_pressed;

    setup_playfield();
    game_setup();
    frame_count = frames_fall_table[frames_fall_index];

    game_has_started = 1;
    while (!game_is_over) {
        if (game_paused) {
            clear_pillbottle_interior();
            write_string_screen(17, 15, 0, "paused", 6);
            while (!(joystick_get(joystick_num) & ST_PRESSED)) {
                waitforjiffy();
            }
            while (joystick_get(joystick_num) & ST_PRESSED) {
                waitforjiffy();
            }
            while (!(joystick_get(joystick_num) & ST_PRESSED)) {
                waitforjiffy();
            }
            waitforjiffy();
            draw_playfield();
            game_paused = 0;
        }
        if (!pill_is_falling) {
            if (grid[0][BOARD_HALF_WIDTH] || grid[0][BOARD_HALF_WIDTH - 1]) {
                player_won = 0;
                game_is_over = 1;
                break;
            }
            gen_pill_colors();
            pill_x = BOARD_HALF_WIDTH - 1;
            pill_y = 0;
            pill_rot = 0;
            pill_is_falling = 1;

            ++pill_num;
            if (pill_num >= 10) {
                pill_num = 0;
                if (num_speed_ups < 49) {
                    ++num_speed_ups;
                    ++frames_fall_index;
                }
            }
        } else {
            static unsigned char joystick_input;
            joystick_input = joystick_get(joystick_num);
            POKEW(0x0a, joystick_input);

            if (!right_cool && !(joystick_input & RT_PRESSED)) {
                temp = pill_x + ((pill_rot % 2) ^ 1) + 1; // temp = pill_x + pill_width
                if (temp < BOARD_WIDTH && !grid[pill_y][temp] && (!(pill_rot % 2) || !grid[pill_y + 1][temp])) {
                    pill_x++;
                    right_cool = KEY_REPEAT_FRAMES;
                }
            } else if (right_cool) {
                --right_cool;
            }

            if (!left_cool && !(joystick_input & LT_PRESSED) && pill_x > 0) {
                temp = pill_x - 1;
                if (!grid[pill_y][temp] && (!(pill_rot % 2) || !grid[pill_y + 1][temp])) {
                    pill_x--;
                    left_cool = KEY_REPEAT_FRAMES;
                }
            } else if (left_cool) {
                --left_cool;
            }

            if (!up_cool && (!(joystick_input & UP_PRESSED) || !(joystick_input & A_PRESSED)) && !check_collision(pill_x, pill_y, (pill_rot + 1) % 4)) {
                if ((pill_rot % 2) && pill_x + 1 >= BOARD_WIDTH) {
                    if (!check_collision(pill_x - 1, pill_y, (pill_rot + 1) % 4)) {
                        inc_pill_rot();
                        --pill_x;
                        up_cool = 1;
                    }
                } else {
                    inc_pill_rot();
                    up_cool = 1;
                }
            } else if ((joystick_input & (UP_PRESSED | A_PRESSED)) == (UP_PRESSED | A_PRESSED)) { // up not pressed
                up_cool = 0;
            }
            down_pressed = !(joystick_input & DOWN_PRESSED) ? 1 : 0;

            if (!(joystick_input & ST_PRESSED)) {
                game_paused = 1;
            }

            if (frame_count == 0) {
                frame_count = frames_fall_table[frames_fall_index];
                if (down_pressed && frame_count > DOWN_PRESS_SPEED) {
                    frame_count = DOWN_PRESS_SPEED;
                }
                if (last_chance && check_collision(pill_x, pill_y + 1, pill_rot)) {
                    pill_is_falling = 0;
                    grid[pill_y][pill_x] = pill_colors[0];
                    grid[pill_y + (pill_rot & 1)][pill_x + ((pill_rot & 1) ^ 1)] = pill_colors[1];

                    check_matches();
                    if (num_viruses_alive == 0 || num_viruses_alive >= 128) {
                        player_won = 1;
                        game_is_over = 1;
                        break;
                    }
                } else {
                    last_chance = 0;
                    ++pill_y;
                }
            }
            if (check_collision(pill_x, pill_y + 1, pill_rot)) {
                last_chance = 1;
            } else {
                last_chance = 0;
            }
            draw_playfield();
        }
        waitforjiffy();
        --frame_count;
    }
    wait_frames(frame_count);
    draw_playfield();
    wait_frames(FRAMES_PER_SECOND);
}

void inc_pill_rot() {
    static unsigned char temp;
    ++pill_rot;
    if ((pill_rot & 1) == 0) {
        temp = pill_colors[0];
        pill_colors[0] = pill_colors[1];
        pill_colors[1] = temp;
    }
}

void gen_pill_colors() {
    pill_colors[0] = next_pill_colors[0];
    pill_colors[1] = next_pill_colors[1];

	function_start:
    next_pill_colors[0] = (rand_byte() + 1) & 0x3;
	if (!next_pill_colors[0]) { goto function_start; }
    next_pill_colors[1] = (rand_byte() + 1) & 0x3;
	if (!next_pill_colors[1]) { goto function_start; }
}

unsigned char virus_min_y[] = {
        10, 10, 10, 10, 10,
        10, 10, 10, 10, 10,
        10, 10, 10, 10, 10,
        11, 11, 12, 12, 13};
unsigned char virus_color_tables[] = {
        2, 1, 0, 0, 1, 2, 2, 1,
        0, 0, 1, 2, 2, 1, 0, 1};

void spawn_viruses() {
    static unsigned char num_viruses;
    num_viruses = (level + 1) << 2;
    if (level == 20) { num_viruses -= 4; }
    num_viruses_alive = num_viruses;

    while (num_viruses > 0) {
        static unsigned char x,y;
        static unsigned char min_row;
        static unsigned char virus_color;
        static unsigned char colors_found[3];

        spawn_viruses_start:

        if (level >= 20) {
            min_row = virus_min_y[19]; // levels >= 20 use # from lvl 19
        } else {
            min_row = virus_min_y[level]; // highest row viruses can appear on based on lvl
        }
        y = rand_byte() & 0xf;
        if (y < 16 - min_row) {
            goto spawn_viruses_start;
        }
        x = rand_byte() & 0x7;
        while (grid[y][x]) {
            step_7_1:
            ++x;
            if (x >= BOARD_WIDTH) {
                ++y;
                if (y >= BOARD_HEIGHT) {
                    goto spawn_viruses_start;
                }
            }
        }
        virus_color = num_viruses & 0x3;
        if (virus_color == 3) {
            virus_color = virus_color_tables[rand_byte() & 0xf];
        }
        // Step 7
        colors_found[0] = 0;
        colors_found[1] = 0;
        colors_found[2] = 0;
        if (x < (BOARD_WIDTH - 2) && grid[y][x + 2]) {
            colors_found[grid[y][x + 2] & 0xf] = 1;
        }
        if (x >= 2 && grid[y][x - 2]) {
            colors_found[grid[y][x - 2] & 0xf] = 1;
        }
        if (y < (BOARD_WIDTH - 2) && grid[y + 2][x]) {
            colors_found[grid[y + 2][x] & 0xf] = 1;
        }
        if (y >= 2 && grid[y - 2][x]) {
            colors_found[grid[y][x - 2] & 0xf] = 1;
        }

        if (colors_found[0] && colors_found[1] && colors_found[2]) {
            goto step_7_1;
        }
        while (colors_found[virus_color]) {
            ++virus_color;
            if (virus_color == 3) { virus_color = 0; }
        }

        grid[y][x] = 0x10 | (virus_color + 1);
        --num_viruses;
        draw_playfield();
        wait_frames(VIRUS_DRAW_FRAMES);

    }

}

unsigned char check_collision(unsigned char x, unsigned char y, unsigned char rot) {
    static unsigned char pill_height;
    static unsigned char pill_length_m1;
    pill_height = (rot % 2);
    pill_length_m1 = pill_height ^ 1;

    return (y + pill_height) >= BOARD_HEIGHT || grid[y + pill_height][x] || (pill_length_m1 && grid[y + pill_height][x + 1]);
}

void setup_calc_pills_fall() {
    __asm__ ("ldx #$7F");
    setup_calc_loop:
    __asm__ ("stz %v, X", fall_grid);
    __asm__ ("dex");
    __asm__ ("bpl %g", setup_calc_loop);
}

/*void calc_pills_fall(unsigned char x, unsigned char y) {
    if (x > 0 && grid[y][x - 1] && grid[y][x - 1] < 0x10) {
        fall_grid[y][x - 1] = 1;
    }
    if (y > 0 && grid[y - 1][x] && grid[y - 1][x] < 0x10) {
        fall_grid[y - 1][x] = 1;
    }
    if (x < BOARD_WIDTH - 1 && grid[y][x + 1] && grid[y][x + 1] < 0x10) {
        fall_grid[y][x + 1] = 1;
    }

}*/

unsigned char pieces_moved;
extern unsigned char piece_hit_something;

void pills_fall(unsigned char first_time) {
    static unsigned char j;
    static unsigned char first_iter;

    piece_hit_something = 0;
    pieces_moved = 0;
    first_iter = 2;

    calc_falling_pieces();

    if (first_time) for (j = 0; j < CASCADE_FALL_FRAMES; ++j) {
        waitforjiffy();
        draw_playfield();
    }
    if (pieces_moved) do {
        pieces_moved = 0;
        make_pieces_fall();
        for (j = 0; j < CASCADE_FALL_FRAMES; ++j) {
            waitforjiffy();
            draw_playfield();
        }
        if (first_iter)  {
            --first_iter;
        }
    } while (pieces_moved && !piece_hit_something);
    if (!first_iter) for (j = 0; j < CASCADE_FALL_FRAMES; ++j) {
        waitforjiffy();
        draw_playfield();
    }
}

void wait_frames(unsigned short num_frames) {
    while (num_frames > 0) {
        waitforjiffy();
        --num_frames;
    }
}

#define FUNCTION_FALL_FRAMES (10)

/*
void wait_cascade_fall_frames() {
    static unsigned char i;

    draw_pill_anyway = PILL_ANIM_1;
    for (i = FUNCTION_FALL_FRAMES; i; --i) {
        waitforjiffy();
        draw_playfield();
    }
    draw_pill_anyway = PILL_ANIM_2;
    for (i = FUNCTION_FALL_FRAMES; i; --i) {
        waitforjiffy();
        draw_playfield();
    }
    draw_pill_anyway = 0;
}
*/


void setup_display() {
    VERA.control = 0;

    VERA.display.video = 0x70 | (0xf & VERA.display.video);
    VERA.display.hscale = 64;
    VERA.display.vscale = 64;

    VERA.layer1.config = 0x62;
    VERA.layer1.mapbase = 0x00;
    VERA.layer1.tilebase = 0xD8;
    VERA.layer1.hscroll = 0x00;
    VERA.layer1.vscroll = 0x00;

    VERA.layer0.config = 0x6;
    VERA.layer0.tilebase = 0x10;
}

void clear_layer1() {
    POKE(0x9F20, 0);
    POKE(0x9F21, 0);
    POKE(0x9F22, 0x10);
    while (PEEK(0x9F21) < 0x1e) {
        POKE(0x9F23, 0);
    }
}

void setup_playfield() {
    load_game_background();
    clear_layer1();
    draw_pillbottle();
}

void draw_playfield() {
    static unsigned char j, i;
    static unsigned char temp;
    static unsigned char curr_virus_offset;

    curr_virus_offset = (frame_tick & 0x10) ? 0x15 : 0x12;

    POKE(0x9F21, 8);
    POKE(0x9F22, 0x10);
    for (j = 0; j < BOARD_HEIGHT; ++j) {
        POKE(0x9F20, 32 /* 16 << 1 */);
        for (i = 0; i < BOARD_WIDTH; ++i) {
            temp = grid[j][i];
            POKE(0x9F23, temp ? ((temp & 0xf0) ? (curr_virus_offset + (temp & 0x3)) : 0x10) : 0x0f);
            POKE(0x9F23, temp << 4);
        }
        __asm__ ("inc $9F21");
    }

    POKEW(0x0c, pill_rot);
    if (pill_is_falling) {
        POKE(0x9F20, 32 + (pill_x << 1));
        POKE(0x9F21, 8 + pill_y);
        POKE(0x9F23, 0xb + ((pill_rot & 1) << 1));
        POKE(0x9F23, pill_colors[0] << 4);
        if (pill_rot % 2) {
            POKE(0x9F20, PEEK(0x9F20) - 2);
            __asm__ ("inc $9F21");
        }
        POKE(0x9F23, 0xc + ((pill_rot & 1) << 1));
        POKE(0x9F23, pill_colors[1] << 4);
    }
    if (game_has_started) {
        POKE(0x9F20, 0x26);
        POKE(0x9F21, 4);

        POKE(0x9F23, 0xb);
        POKE(0x9F23, next_pill_colors[0] << 4);
        POKE(0x9F23, 0xc);
        POKE(0x9F23, next_pill_colors[1] << 4);
    }
}

void write_num_screen(unsigned char x, unsigned char y, unsigned char palette, unsigned char num) {
    palette = palette << 4;

    POKE(0x9F20, x << 1);
    POKE(0x9F21, y);
    POKE(0x9F22, 0x10);
    POKE(0x9F23, 0xb0 | (num / 10));
    POKE(0x9F23, palette);
    POKE(0x9F23, 0xb0 | (num % 10));
    POKE(0x9F23, palette);
}

void write_string_screen(unsigned char x, unsigned char y, unsigned char palette, char *string, unsigned char str_length) {
    static unsigned char p;
    static char *s;
    static unsigned char l;

    p = palette << 4;
    s = string;
    l = str_length;

    POKE(0x9F20, x << 1);
    POKE(0x9F21, y);
    POKE(0x9F22, 0x10);

    __asm__ ("lda %v", s);
    __asm__ ("sta ptr1");
    __asm__ ("lda %v + 1", s);
    __asm__ ("sta ptr1 + 1");
    __asm__ ("ldy #0");
    loop_write_string_screen:
    __asm__ ("lda (ptr1), Y");
    __asm__ ("ora #$80");
    __asm__ ("sta $9F23");
    __asm__ ("iny");
    __asm__ ("lda %v", p);
    __asm__ ("sta $9F23");
    __asm__ ("dec %v", l);
    __asm__ ("bne %g", loop_write_string_screen);
}

#define BACKGROUND_WIDTH_64 4
#define BACKGROUND_HEIGHT_32 3

unsigned char palette_offsets_background[3] = {0xB1, 0xB0, 0xB3};

void setup_settings_background() {
    static unsigned char i,j;
    static unsigned short temp;

    POKEW(0x9F20, 0xFD08);
    POKE(0x9F22, 0x11);
    for (j = 0; j < BACKGROUND_HEIGHT_32; ++j) {
        for (i = 0; i < BACKGROUND_WIDTH_64; ++i) {
            POKE(0x9F23, 0x11A00 >> 5);
            POKE(0x9F23, 0x11A00 >> 13);
            temp = 32 + (i << 6);
            __asm__ ("lda %v", temp);
            __asm__ ("sta $9F23");
            __asm__ ("lda %v + 1", temp);
            __asm__ ("sta $9F23");
            temp = 44 + (j << 6);
            __asm__ ("lda %v", temp);
            __asm__ ("sta $9F23");
            __asm__ ("lda %v + 1", temp);
            __asm__ ("sta $9F23");
            POKE(0x9F23, 0x08);
            POKE(0x9F23, palette_offsets_background[j]);
        }
    }
}

#define NUM_LOGO_SPRITES 26

void setup_logo() {
    static unsigned char i,j;
    static unsigned char index;
    static unsigned short temp;

    index = 0;

    POKEW(0x9F20, 0xFC08);
    POKE(0x9F22, 0x11);
    for (j = 0; j < 2; ++j) {
        for (i = 0; i < 13; ++i) {
            temp = index << 8;
            POKE(0x9F23, temp >> 5);
            POKE(0x9F23, 0x08 | (temp >> 13));
            temp = 56 + (i << 4);
            __asm__ ("lda %v", temp);
            __asm__ ("sta $9F23");
            __asm__ ("lda %v + 1", temp);
            __asm__ ("sta $9F23");
            temp = 24 + (j << 5);
            __asm__ ("lda %v", temp);
            __asm__ ("sta $9F23");
            __asm__ ("lda %v + 1", temp);
            __asm__ ("sta $9F23");
            POKE(0x9F23, 0x0C);
            POKE(0x9F23, 0x90); // 32 x 16 sprites

            ++index;
        }
    }
}

void disable_logo() {
    static unsigned char i;

    POKEW(0x9F20, 0xFC0E);
    POKE(0x9F22, 0x41);
    for (i = 1; i; ++i) {
        POKE(0x9F23, 0);
    }
}

#define LOGO_TABLE_LENGTH 30
unsigned char logo_mvmt_table[LOGO_TABLE_LENGTH] = {
        0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0,
};

void move_logo() {
    static unsigned char i;
    static unsigned char table_index = LOGO_TABLE_LENGTH >> 1;
    static unsigned char move_direction = 0;
    static unsigned char current_move_amt;

    current_move_amt = logo_mvmt_table[table_index];

    POKE(0x9F22, 0x01);
    POKEW(0x9F20, 0xFC0C);
    if (!move_direction) {
        for (i = 0; i < NUM_LOGO_SPRITES; ++i) {
            __asm__ ("lda $9F23");
            __asm__ ("clc");
            __asm__ ("adc %v", current_move_amt);
            __asm__ ("sta $9F23");
            __asm__ ("inc $9F20");

            __asm__ ("lda $9F23");
            __asm__ ("adc #0");
            __asm__ ("sta $9F23");

            POKEW(0x9F20, PEEKW(0x9F20) + 7);
        }
    } else {
        for (i = 0; i < 26; ++i) {
            __asm__ ("lda $9F23");
            __asm__ ("sec");
            __asm__ ("sbc %v", current_move_amt);
            __asm__ ("sta $9F23");
            __asm__ ("inc $9F20");

            __asm__ ("lda $9F23");
            __asm__ ("sbc #0");
            __asm__ ("sta $9F23");

            POKEW(0x9F20, PEEKW(0x9F20) + 7);
        }
    }

    ++table_index;
    if (table_index >= LOGO_TABLE_LENGTH) {
        table_index = 0;
        move_direction = !move_direction;
    }
}

void clear_pillbottle_interior() {
    static unsigned char j, i;
    POKE(0x9F22, 0x20);
    POKE(0x9F21, 8);
    for (j = 0; j < 16; ++j) {
        POKE(0x9F20, 32);
        for (i = 0; i < 8; ++i) {
            POKE(0x9F23, 0);
        }
        __asm__ ("inc $9F21");
    }
}

#define BOTTLE_START_TILE_INDEX 0x19
#define BOTTLE_LAST_TILE_P1 0x67

unsigned char bottle_positions[][2] = {
{2, 0},
{3, 0},
{4, 0},
{5, 0},
{6, 0},
{7, 0},
{2, 1},
{3, 1},
{4, 1},
{5, 1},
{6, 1},
{7, 1},
{2, 2},
{3, 2},
{6, 2},
{7, 2},
{2, 3},
{3, 3},
{6, 3},
{7, 3},
{0, 4},
{1, 4},
{2, 4},
{3, 4},
{4, 4},
{5, 4},
{6, 4},
{7, 4},
{8, 4},
{9, 4},
{0, 5},
{1, 5},
{2, 5},
{3, 5},
{6, 5},
{7, 5},
{8, 5},
{9, 5},
{0, 6},
{9, 6},
{0, 7},
{9, 7},
{0, 8},
{9, 8},
{0, 9},
{9, 9},
{0, 10},
{9, 10},
{0, 11},
{9, 11},
{0, 12},
{9, 12},
{0, 13},
{9, 13},
{0, 14},
{9, 14},
{0, 15},
{9, 15},
{0, 16},
{9, 16},
{0, 17},
{9, 17},
{0, 18},
{9, 18},
{0, 19},
{9, 19},
{0, 20},
{9, 20},
{0, 21},
{1, 21},
{2, 21},
{3, 21},
{4, 21},
{5, 21},
{6, 21},
{7, 21},
{8, 21},
{9, 21},
};

void draw_pillbottle() {
    static unsigned char tile_index;
    static unsigned char i;

    tile_index = BOTTLE_START_TILE_INDEX;
    i = 0;
    POKE(0x9F22, 0x10);
    while (tile_index < BOTTLE_LAST_TILE_P1) {
        POKE(0x9F20, (bottle_positions[i][0] << 1) + 30);
        POKE(0x9F21, bottle_positions[i][1] + 3);
        POKE(0x9F23, tile_index);
        POKE(0x9F23, 0x60);
        ++i;
        ++tile_index;
    }
}

#define DEVICE_NUM 8
#define DR_BIN_FILELEN 0x1000
#define LETTER_BIN_FILELEN 0x1000
#define SPRITE_BIN_FILELEN 0x2000
#define PALETTE_FILELEN 512
#define LOAD_ADDRESS 0xA000

#define TITLE_BACKGROUND_BANK 4
#define GAME_BACKGROUND_BANK 9

void load_graphics() {
	cbm_k_setnam("dr.bin");
	cbm_k_setlfs(0, DEVICE_NUM, 2);
	POKE(0x9F20, 0);
	POKE(0x9F21, 0xB0);
	POKE(0x9F22, 0x11);
    RAM_BANK = 1;
	cbm_k_load(0, 0xA000);
    POKEW(0x2, LOAD_ADDRESS);
    POKEW(0x4, 0x9F23);
    POKEW(0x6, DR_BIN_FILELEN);
    __asm__("jsr $FEE7");

    cbm_k_setnam("letter.bin");
    cbm_k_setlfs(0, DEVICE_NUM, 2);
    RAM_BANK = 1;
    cbm_k_load(0, 0xA000);
    POKEW(0x6, LETTER_BIN_FILELEN);
    __asm__("jsr $FEE7");

    cbm_k_setnam("palette.bin");
    cbm_k_setlfs(0, DEVICE_NUM, 2);
    POKE(0x9F20, 0);
    POKE(0x9F21, 0xFA);
    POKE(0x9F22, 0x11);
    RAM_BANK = 1;
    cbm_k_load(0, LOAD_ADDRESS);
    POKEW(0x6, PALETTE_FILELEN);
    __asm__("jsr $FEE7");

    cbm_k_setnam("sprites.bin");
    cbm_k_setlfs(0, DEVICE_NUM, 2);
    RAM_BANK = 1;
    cbm_k_load(0, 0xA000);
    POKE(0x9F20, 0x00);
    POKE(0x9F21, 0x00);
    POKE(0x9F22, 0x11);
    load_ram_banks_vram(1, 2, 0xc000);

    cbm_k_setnam("menu.bin");
    cbm_k_setlfs(0, DEVICE_NUM, 2);
    RAM_BANK = TITLE_BACKGROUND_BANK;
    cbm_k_load(0, LOAD_ADDRESS);

    cbm_k_setnam("office.bin");
    cbm_k_setlfs(0, DEVICE_NUM, 2);
    RAM_BANK = GAME_BACKGROUND_BANK;
    cbm_k_load(0, LOAD_ADDRESS);

}

void load_bitmap_into_vram(unsigned char startbank) {
    POKEW(0x9F20, 0x2000);
    POKE(0x9F22, 0x10);
    load_ram_banks_vram(startbank, startbank + 4, 0xb600);
}

void load_title_background() {
    VERA.layer0.hscroll = 0x0500;
    load_bitmap_into_vram(TITLE_BACKGROUND_BANK);
}

void load_game_background() {
    VERA.layer0.hscroll = 0x0400;
    load_bitmap_into_vram(GAME_BACKGROUND_BANK);
}

void load_ram_banks_vram(unsigned char startbank, unsigned char endbank, unsigned short lastaddr) {
    static unsigned char num_banks;
    num_banks = endbank - startbank;

    RAM_BANK = startbank;
    POKEW(0x2, LOAD_ADDRESS);
    POKEW(0x4, 0x9F23);
    POKEW(0x6, 0x2000);
    while (num_banks) {
        __asm__("jsr $FEE7");
        --num_banks;
        RAM_BANK = RAM_BANK + 1;
    }
    POKEW(0x6, lastaddr - 0xA000);
    __asm__("jsr $FEE7");
}