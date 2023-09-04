r0 = $02
r1 = $04
r2 = $06

BOARD_WIDTH = 8
BOARD_HEIGHT = 16

NO_SUPPORT = 0
SUPPORT_SELF = 1
SUPPORT_LEFT = 2
SUPPORT_UNDER = 3
SUPPORT_RIGHT = 4

.importzp tmp1, tmp2, tmp3

; Imported functions from cc65
.import _srand, _rand
.import popa, popax
.import pusha

; Imported functions from main.c
.import _pills_fall, _setup_calc_pills_fall
.import _wait_cascade_fall_frames
.import _animate_viruses

; sound play functions
.import _play_kill_sfx

; Zsound functions
;.import _pcm_play, _zsm_play
;.import _pcm_trigger_digi
.import _zcm_play, _zsm_tick

; Imported variables & arrays
.import _pill_rot
; .import _fall_grid
; .import _grid
.import _pieces_moved
.import _frame_tick
.import _game_time_units
.import _game_paused
.import _num_viruses_alive
.import _alive_virus_colors
.import _difficulty

RDTIM = $FFDE
entropy_get := $FECF

.export _setup_rand
_setup_rand:
    lda #0
    jsr entropy_get
    stx tmp1
    eor tmp1
    phy
    plx
    jsr _srand


;
; char __fastcall__ rand_byte();
;
.export _rand_byte
_rand_byte:
    jsr _rand
    stx tmp1
    eor tmp1
    rts

	;jsr entropy_get
	;stx tmp1
	;eor tmp1
	;sty tmp1
	;eor tmp1
	;rts

.export _joystick_get
_joystick_get := $FF56

.export _waitforjiffy
_waitforjiffy:
jsr RDTIM
sta tmp1
:
jsr RDTIM
cmp tmp1
beq :-

lda #0
jsr _zsm_tick
jsr _animate_viruses

lda _game_paused
beq @incrementGameTimers
rts
@incrementGameTimers:
inc _frame_tick
bne :+
inc _frame_tick + 1
:

ldx #0
:
lda _game_time_units, X
inc A
cmp #60
bcc :+
stz _game_time_units, X
inx
cpx #3
bcc :-
dex
lda #0 ; 1 hour of gameplay, reset clock to 0
:
sta _game_time_units, X


rts

;
; void __fastcall__ calc_pills_fall();
;
; This function finds out which pills should fall due to gravity after a match.
;
; fall_grid[y][x] = 1 --> piece should fall
; fall_grid[y][x] = 0 --> piece stays put
;
.export _calc_pills_fall
_calc_pills_fall:
    jsr _setup_calc_pills_fall
	
	ldy #BOARD_HEIGHT * BOARD_WIDTH - 1 ; $7F
@check_loop:	
	lda _grid, Y
	beq :+
	lda _support_grid, Y	
	cmp #NO_SUPPORT	
	bne @next_iter ; if tile stil there and tile has support, do nothing
	; if tile has no support and is there, should be falling
	lda #1
	sta _fall_grid, Y
	sta _pieces_moved
	lda #0
	sta _support_grid, Y
	:
	
	tya
	sec
	sbc #8
	tax
	
	lda _grid, X
	beq :+
	lda _support_grid, X
	cmp #SUPPORT_UNDER ; if above tile doesnt depend on this tile, do nothing
	beq @support_under_fall
	:
	tya
	tax
	bra @check_side_tiles

@support_under_fall:	
	; do things
	lda #1
	sta _fall_grid, X
	stz _support_grid, X

@check_side_tiles:	
	; if tile to left has SUPPORT_RIGHT, make it fall
	; do similar thing for right tile
	tya
	and #$7
	phx
	beq :+
	dex
	lda _grid, X
	beq :+ ; if no tile, skip ahead
	lda _support_grid, X
	cmp #SUPPORT_RIGHT
	bne :+
	lda #1
	sta _fall_grid, X
	sta _pieces_moved
	stz _support_grid, X
	:
	plx
	tya
	and #$7
	cmp #$7
	; if on rightmost column skip check
	phx
	beq :+
	inx
	lda _grid, X
	beq :+ ; if no tile, skip ahead
	lda _support_grid, X
	cmp #SUPPORT_LEFT
	bne :+
	lda #1
	sta _fall_grid, X
	sta _pieces_moved
	stz _support_grid, X
	:
	plx	
	bra @next_iter
	
@next_iter:	
	dey
	cpy #8
	bcs @check_loop
	
	rts

.export _piece_hit_something
_piece_hit_something:
    .byte 0
;
; void make_pieces_fall();
;
; This function does the work of moving pieces down as they fall due to gravity
;
.export _make_pieces_fall
_make_pieces_fall:
    ldy #BOARD_WIDTH * (BOARD_HEIGHT - 1) - 1
	
	@loop:
    lda _fall_grid, Y
    beq @next_iter
    lda _grid, Y
    beq @next_iter

    tya
    clc
    adc #8
    tax
    adc #8
    sta tmp1
    lda _grid, X
    bne @second_fail_case

    lda #1
    sta _pieces_moved
    sta _fall_grid, X

    cpy #(BOARD_HEIGHT - 1) * BOARD_WIDTH
    bcs :++ ; if .Y represents piece with y = 15, skip past
    phx
    ldx tmp1
    lda _grid, X
    beq :+
    lda _fall_grid, X
    bne :+
    lda #1
    sta _piece_hit_something
    :
    plx
    :

    lda _grid, Y
    sta _grid, X
    lda #0
    sta _grid, Y
	
	lda #NO_SUPPORT
	sta _support_grid, Y
	sta _support_grid, X
	bra @second_fail_case_tt
@second_fail_case:
	lda #SUPPORT_UNDER
	sta _support_grid, Y
@second_fail_case_tt:
    lda #0
    sta _fall_grid, Y

@next_iter:
    dey
    bpl @loop
    rts

fall_anyway:
    .byte 0
viruses_killed:
    .byte 0
;
; void check_matches();
;
; This function goes through the game grid to find matches >= 4 and r
; remove those pieces from the grid, changing virus #'s if necessary
;
.export _check_matches
_check_matches:
    stz fall_anyway
@check_matches_reentry:
    stz @match_found
    stz viruses_killed

    ldy #BOARD_HEIGHT * BOARD_WIDTH - 1
@horiz_loop:
    tya
    and #7
    cmp #7
    beq @is_new

    lda _grid, Y
    and #$0f
    cmp @to_match
    bne @is_new_ent

    inx
    txa
    sta _spare_grid, Y
    bra @end_horiz_loop
@is_new:
    lda _grid, Y
    and #$0f
@is_new_ent:
    sta @to_match
    lda #1
    ldx #1
    sta _spare_grid, Y
@end_horiz_loop:
    dey
    bpl @horiz_loop

    ldy #0
@check_horiz_loop:
    lda _grid, Y
    beq @not_horiz_match
    lda _spare_grid, Y
    cmp #4
    bcc @not_horiz_match

; Found a match
	sta @size_row
    lda #1
    sta @match_found
	
	phx
	phy 
	jsr _play_kill_sfx
	ply
	plx
	
    phy ; push index ;

    ldx @size_row

    :
    lda _grid, Y
    cmp #$10
    bcc :+
    dec _num_viruses_alive
    inc viruses_killed
    phx
    and #$3
    tax
    dec _alive_virus_colors, X
    plx
    :
    lda #0
    sta _grid, Y
    sta _spare_grid, Y
	sta _support_grid, Y
    iny
    dex
    bne :--
    ply ; pull one last time ;
    lda _num_viruses_alive
    bne @not_horiz_match
    jsr @calc_score_add
    rts
@not_horiz_match:
    iny
    cpy #BOARD_HEIGHT * BOARD_WIDTH
    bcc @check_horiz_loop

    ldy #BOARD_HEIGHT * BOARD_WIDTH - 1
    bra @is_new_vert
@vert_loop:
    lda _grid, Y
    and #$0f
    cmp @to_match
    bne @is_new_vert_ent

    inx
    txa
    sta _spare_grid, Y
    bra @end_vert_loop
@is_new_vert:
    lda _grid, Y
    and #$0f
@is_new_vert_ent:
    sta @to_match
    lda #1
    ldx #1
    sta _spare_grid, Y
@end_vert_loop:
    tya
    sec
    sbc #8
    bcs :+
    cpy #0
    beq @vert_loop_done
    tya
    dec A
    ora #$78
    pha
    ldx #0
    tay
    lda _grid, Y
    sta @to_match
    pla
    :
    tay
    jmp @vert_loop
@vert_loop_done:

    ldy #0
@vert_check_loop:
    lda _grid, Y
    beq @not_vert_match
    lda _spare_grid, Y
    cmp #4
    bcc @not_vert_match

; Found a match
    sta @size_row
    lda #1
    sta @match_found
	
	phx
	phy 
	jsr _play_kill_sfx
	ply
	plx
	
    phy ; pull and push
    ldx @size_row
    :
    lda _grid, Y
    cmp #$10
    bcc :+
    dec _num_viruses_alive
    inc viruses_killed
    phx
    and #$3
    tax
    dec _alive_virus_colors, X
    plx
    :
    lda #0
    sta _grid, Y
    sta _spare_grid, Y
	sta _support_grid, Y
    tya
    clc
    adc #8
    tay
    dex
    bne :--
    ply
    lda _num_viruses_alive
    bne @not_vert_match
    jsr @calc_score_add
    rts
@not_vert_match:
    iny
    cpy #BOARD_WIDTH * BOARD_HEIGHT
    bcc @vert_check_loop

@end:
    jsr @calc_score_add

@no_viruses_killed:

    lda @match_found
    bne :+
    lda fall_anyway
    stz fall_anyway
    cmp #0
    beq :++
    lda #0
    tax
    jsr _pills_fall
    jmp @check_matches_reentry
    :
    lda #1
    ldx #0
    jsr _pills_fall
    lda #1
    sta fall_anyway
    jmp @check_matches_reentry
    :
    rts

;
; Score is bugged
;
@calc_score_add:
    lda viruses_killed
    asl
    asl
    ora _difficulty
    tax
    lda _score_to_add, X
	beq :+
    sed
    clc
    adc _score
    sta _score
    lda _score + 1
    adc #0
    sta _score + 1
    lda _score + 2
    adc #0
    sta _score + 2
    cld
	:
    rts

@match_found:
    .byte 0
@size_row:
    .byte 0
@to_match:
    .byte 0
@ind:
    .byte 0
@j:
    .byte 0
@i:
    .byte 0

; Define addresses for grid
.export _fall_grid
_fall_grid:
	.res (BOARD_WIDTH * BOARD_HEIGHT)
.export _grid
_grid:
	.res (BOARD_WIDTH * BOARD_HEIGHT)

.export _spare_grid
_spare_grid:
    .res (BOARD_WIDTH * BOARD_HEIGHT)

.export _support_grid	
_support_grid:
	.res (BOARD_WIDTH * BOARD_HEIGHT)

.export _score_to_add
_score_to_add:
    .byte $00, $00, $00, $FF
    .byte $01, $02, $03, $FF
    .byte $02, $04, $06, $FF
    .byte $04, $08, $12, $FF
    .byte $08, $16, $24, $FF
    .byte $16, $32, $48, $FF
    .byte $32, $64, $96, $FF

.export _score
_score:
    .res 4
.export _top_score
_top_score:
    .res 4

SCORE_UPDATE_X = 10
SCORE_UPDATE_Y = 7

DISPLAY_CURRENT = 0
DISPLAY_TOP = 1

.export _display_score
_display_score:
    cmp #DISPLAY_CURRENT
    beq :+
    ; top score
    ldy #(_top_score - _score) + 2
    lda #SCORE_UPDATE_Y
    bra :++
    :
    ; current score
    ldy #2
    lda #(SCORE_UPDATE_Y + 3)
    :
    sta $9F21

    lda #$10
    sta $9F22
    lda #SCORE_UPDATE_X
    sta $9F20

    ldx #2
    :
    lda _score, Y
    lsr
    lsr
    lsr
    lsr
	clc
    adc #$90
    sta $9F23
    lda #$a0
    sta $9F23

    lda _score, Y
	and #$0F
    clc
    adc #$90
    sta $9F23
    lda #$a0
    sta $9F23
    dey
    dex
    bpl :-
    rts

.export _find_piece_support
_find_piece_support:
	sta @y2
	jsr popa
	sta @x2
	
	jsr popa 
	sta @y
	jsr popa
	sta @x
	
	lda @y
	cmp #(BOARD_HEIGHT - 1)
	beq :+
	lda @y2
	cmp #(BOARD_HEIGHT - 1)
	bne @not_support_self	
	:
	; if y = 15 or vertical and y = 14, ground supports it
	lda #SUPPORT_SELF
	rts
	
@not_support_self:
	lda @y
	asl
	asl
	asl
	ora @x
	tax
	
	clc
	adc #BOARD_WIDTH
	tay
	; index of piece below in .Y
	lda _grid, Y
	beq :+
	; There is a piece below this one
	; =
	; =
	lda #SUPPORT_UNDER
	rts
	:
	lda @x
	cmp @x2
	beq @no_support ; if pill was placed vertically and nothing is below position, there is no support
	bcc :+ ; if x < x2, skip ahead 
	
	dey
	lda _grid, Y
	iny
	cmp #0
	beq @no_support
	lda #SUPPORT_LEFT ; since x > x2, this tile is supported by the piece under x2
	; x is supported by x2, to its left
	; ==
	; =
	rts
	:
	;bcs :+ ; if x > x2, skip ahead
	iny
	lda _grid, Y
	beq :+
	lda #SUPPORT_RIGHT
	; x is supported to x2, to its right
	; ==
	;  =
	rts
	:
@no_support:
	lda $BEEF
	lda #NO_SUPPORT
	rts
	
@x:
	.byte 0
@y:
	.byte 0
@x2:
	.byte 0
@y2:
	.byte 0
