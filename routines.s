r0 = $02
r1 = $04
r2 = $06

BOARD_WIDTH = 8
BOARD_HEIGHT = 16

.importzp tmp1, tmp2, tmp3

.import _srand, _rand
.import popa, popax

.import _pills_fall, _setup_calc_pills_fall

.import _fall_grid, _grid
.import _pieces_moved
.import _frame_tick
.import _game_time_units
.import _game_paused
.import _num_viruses_alive

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
.export _calc_pills_fall
_calc_pills_fall:
@x = tmp3
@y = tmp2
@ind = tmp1
    lda $03
    sta @y

    asl
    asl
    asl
    ora $02 ; x
    sta @ind

    lda $02
    cmp #0
    beq @x_zero ; if x == 0, branch ahead
    ; Check if grid[y][x - 1] == 0

    ldy @ind
    dey
    lda _grid, Y
    beq :+
    cmp #$10
    bcs :+
    sta _fall_grid, Y
    :
    lda $02
@x_zero:
    cmp #(BOARD_WIDTH - 1)
    bcs @x_board_width_m1 ; if x >= board_width - 1, branch ahead
    ; Check for grid[y][x + 1] == 0
    ldy @ind
    iny
    lda _grid, Y
    beq :+
    cmp #$10
    bcs :+
    sta _fall_grid, Y
    :
@x_board_width_m1:
    lda @y
    beq @y_zero ; if y == 0, branch ahead
    ; Check grid[y - 1][x]
    lda @ind
    sec
    sbc #8
    tay
    lda _grid, Y
    beq :+
    cmp #$10
    bcs :+
    sta _fall_grid, Y
    :
@y_zero:
    rts


;
; void call_falling_pieces();
;
.export _calc_falling_pieces
_calc_falling_pieces:
    ldy #BOARD_WIDTH * BOARD_HEIGHT - 1
@loop:
    lda _fall_grid, Y
    beq @fail_outer_if
    sta _pieces_moved ; non-zero

    tya
    sec
    sbc #8
    sta tmp1

    tya
    and #7
    bne :+
    ldx tmp1
    dex
    lda _grid, X
    beq :+
    cmp #$10
    bcs :+
    ; .A is non-zero
    sta _fall_grid, X
    :
    ldx tmp1
    lda _grid, X
    beq :+
    cmp #$10
    bcs :+
    ; .A is non-zero
    sta _fall_grid, X
    :
    tya
    and #7
    cmp #7
    beq :+
    ldx tmp1
    inx
    lda _grid, X
    beq :+
    cmp #$10
    bcs :+
    sta _fall_grid, X
    :
@fail_outer_if:
    dey
    cpy #8
    bcs @loop
    rts

.export _piece_hit_something
_piece_hit_something:
    .byte 0
;
; void make_pieces_fall();
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
    bcs :++
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
@second_fail_case:
    lda #0
    sta _fall_grid, Y
@next_iter:
    dey
    bpl @loop
    rts

fall_anyway:
    .byte 0
;
; void check_matches();
;
.export _check_matches
_check_matches:
    stz fall_anyway
    jsr _setup_calc_pills_fall
@check_matches_reentry:
    stp
    stz @match_found

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

    ldy #BOARD_WIDTH * BOARD_HEIGHT - 4
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
    phy ; push index ;

    tya
    and #7
    sta $02 ; x = index & %111 (& 7)
    tya
    lsr
    lsr
    lsr
    sta $03 ; y = index >> 3 (div 8)
    ldx #0
    :
    phx
    jsr _calc_pills_fall
    inc $02
    plx
    inx
    cpx @size_row
    bcc :-

    ply ; pull index ;
    phy ; push again ;
    ldx #0
    lda #0
    :
    sta _grid, Y
    iny
    inx
    cpx @size_row
    bcc :-
    ply ; pull one last time ;
@not_horiz_match:
    dey
    bpl @check_horiz_loop

@end:
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
.export _spare_grid
_spare_grid := $9000
    .res (BOARD_WIDTH * BOARD_HEIGHT)