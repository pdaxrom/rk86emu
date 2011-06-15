#include <stdio.h>
#include "i8080.h"

extern uns8 memory[];
extern uns8 memory_mon[];

/* VV55 keyboard */
uns8 keyboard_mod;
uns8 keyboard_state[8];

/* VG75 and screen */

#define VM_SIZE_X	64

int vg75_mode_cmd;
int vg75_sx, vg75_sy;
int vg75_param_cmd;
int vg75_cursor_cmd;
int vg75_cursor_x, vg75_cursor_y;
unsigned int vg75_base;
unsigned int vg75_user_size;
unsigned int video_mem_base;
unsigned int video_mem_size;

int screen_cols;
int screen_rows;
int screen_base_ofs;

void draw_char(uns16 x, uns16 y, uns8 c);

static void update_screen();

void perifer_init(void)
{
    int i;
    for (i = 0; i < 8; i++)
	keyboard_state[i] = 0xff;
    keyboard_mod = 0xff;
}

static void restart_cursor()
{
//    set_cursor_position(vg75_cursor_x, vg75_cursor_y);
}

static void restart_vm()
{
    if (vg75_sy > vg75_user_size / vg75_sx)
	vg75_sy = vg75_user_size / vg75_sx;

#if 0
    if (vg75_sy <= 30 && vg75_size_y > 30)
	init_vidmode_80x30();
    else if (vg75_sy > 30 && vg75_sy <= 43 && (vg75_size_y <= 30 || vg75_size_y > 43))
	init_vidmode_80x43();
    else if (vg75_sy > 43 && vg75_size_y <= 43)
	init_vidmode_80x50();
#endif

    screen_cols = vg75_sx;
    screen_rows = vg75_sy;
    video_mem_size = screen_cols * screen_rows;
    screen_base_ofs = screen_cols * 3 + 8;
    video_mem_base = vg75_base;

//    fprintf(stderr, "VIDEO: %04X %d %d\n", vg75_base, vg75_sx, vg75_sy);

    update_screen();
}

static void update_screen()
{
    int i, j;

    for(i = 0; i < screen_rows; i++) {
	for(j = 0; j < screen_cols; j++)
	    draw_char(j - 8, i - 3, memory[video_mem_base + i * screen_cols + j]);
    }
}

inline uns8 RD_BYTE(uns16 addr)
{
    if (addr >= 0xF800)
	return memory_mon[addr & 0x7ff];

    if (addr == 0xc001)
	return 0xff;

    if (addr == 0x8002) {
//	fprintf(stderr, "read KBD MODE\n");
	return keyboard_mod;
    }

    if (addr == 0x8001) {
//	fprintf(stderr, "read KBD\n");
	int i;
	uns8 dat = 0xff;
	for (i = 0; i < 8; i++)
	    if (1 << i & ~memory[0x8000])
		dat &= keyboard_state[i];
	return dat;
    }

    return memory[addr];
}

inline void WR_BYTE(uns16 addr, uns8 dat)
{
    memory[addr] = dat;

    /*
     *
     */
    if (addr == 0x8000) {
//	fprintf(stderr, "WRITE KBD 0x8000: %02X\n", dat);
    }

    if (addr == 0x8003) {
//	fprintf(stderr, "WRITE KBD 0x8003: %02X\n", dat);
    }

    /*
     * catch cursor command
     */
    if ((addr & 0xefff) == 0xc001 && dat == 0x80) {
	vg75_cursor_cmd = 1;
	return;
    }

    if ((addr & 0xefff) == 0xc000 && vg75_cursor_cmd == 1) {
	vg75_cursor_cmd++;
	vg75_cursor_x = dat + 1;
	return;
    }

    if ((addr & 0xefff) == 0xc000 && vg75_cursor_cmd == 2) {
	vg75_cursor_y = dat + 1;
	restart_cursor();
	vg75_cursor_cmd = 0;
	return;
    }

    /*
     * catch screen format command
     */
    if ((addr & 0xefff) == 0xc001 && dat == 0) {
	vg75_mode_cmd = 1;
	return;
    }

    if ((addr & 0xefff) == 0xc000 && vg75_mode_cmd == 1) {
	vg75_sx = (dat & 0x7f) + 1;
	vg75_mode_cmd++;
	return;
    }

    if ((addr & 0xefff) == 0xc000 && vg75_mode_cmd == 2) {
	vg75_sy = (dat & 0x3f) + 1;
	vg75_mode_cmd = 0;
    }

    if ((addr & 0xefff) == 0xe008 && dat == 0x80) {
	vg75_param_cmd = 1;
	return;
    }

    if ((addr & 0xefff) == 0xe004 && vg75_param_cmd == 1) {
	vg75_base = dat;
	vg75_param_cmd++;
	return;
    }

    if ((addr & 0xefff) == 0xe004 && vg75_param_cmd == 2) {
	vg75_base |= dat << 8;
	vg75_param_cmd++;
	return;
    }

    if ((addr & 0xefff) == 0xe005 && vg75_param_cmd == 3) {
	vg75_user_size = dat;
	vg75_param_cmd++;
	return;
    }

    if ((addr & 0xefff) == 0xe005 && vg75_param_cmd == 4) {
	vg75_user_size = ((vg75_user_size | dat << 8) & 0x3fff) + 1;
	vg75_param_cmd=0;
    }

    /*
     * Settings of video memory bounds and screen format
     * make sence only after the DMA command 0xA4 --
     * start channel
     */
    if ((addr & 0xefff) == 0xe008 && dat == 0xa4) {
	if (vg75_sx && vg75_sy) {
	    restart_vm();
	    return;
	}
    }

    if (addr >= video_mem_base && addr < video_mem_base + video_mem_size) {
	addr -= video_mem_base;
	draw_char(addr % screen_cols - 8, addr / screen_cols - 3, dat);
    }
}

inline uns16 RD_WORD(uns16 addr)
{
    uns16 temp = RD_BYTE(addr++);
    temp |= (RD_BYTE(addr) << 8);

    return temp;
}

inline void WR_WORD(uns16 addr, uns16 dat)
{
    WR_BYTE(addr++, dat & 0xFF);
    WR_BYTE(addr, dat >> 8);
}

inline uns8 in_byte(uns8 port)
{
//    fprintf(stderr, "\nRead port %02X\r", port);
    return 0;
}

inline void out_byte(uns8 port, uns8 data)
{
//    fprintf(stderr, "\nWrite port %02X = %02X\r", port, data);
}

inline void sound_on(void)
{
}

inline void sound_off(void)
{
}
