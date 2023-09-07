.include "zsmkit/zsmkit.inc"

.import popa, popax

; void __fastcall__ zsm_init_engine(unsigned char i);
.export _zsm_init_engine
_zsm_init_engine := zsm_init_engine

; void __fastcall__ zcm_setmem(unsigned char slot, unsigned addr, unsigned char bank);
.export _zcm_setmem
_zcm_setmem:
	ldx $00
	phx
	sta $00
	
	jsr popax
	pha
	phx
	
	jsr popa
	tax
	ply
	pla
	
	jsr zcm_setmem
	pla
	sta $00
	rts

;void __fastcall__ zcm_play(unsigned char slot, unsigned char volume);
.export _zcm_play
_zcm_play:
	pha
	jsr popa
	tax
	pla
	jmp zcm_play

; void __fastcall__ zcm_stop();
.export _zcm_stop
_zcm_stop := zcm_stop

; void __fastcall__ zcm_tick(unsigned char setting);	
.export _zsm_tick
_zsm_tick := zsm_tick
	