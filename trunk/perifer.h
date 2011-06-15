#ifndef _PERIFER_H_
#define _PERIFER_H_

enum {
    SS = 0x20,
    US = 0x40,
    RL = 0x80
};

extern uns8 keyboard_mod;
extern uns8 keyboard_state[];

void perifer_init(void);

uns8 RD_BYTE(uns16 addr);
void WR_BYTE(uns16 addr, uns8 dat);

uns16 RD_WORD(uns16 addr);
void WR_WORD(uns16 addr, uns16 dat);

uns8 in_byte(uns8 port);
void out_byte(uns8 port, uns8 data);

void sound_on(void);
void sound_off(void);

#endif
