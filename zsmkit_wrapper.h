void __fastcall__ zsm_init_engine(unsigned char bank);

void __fastcall__ zcm_setmem(unsigned char slot, unsigned addr, unsigned char bank);
void __fastcall__ zcm_play(unsigned char slot, unsigned char volume);
void __fastcall__ zcm_stop();

void __fastcall__ zsm_tick(unsigned char setting);