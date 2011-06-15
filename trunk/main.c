#include <stdio.h>
#include <unistd.h>
#include <SDL.h>
#include "i8080.h"
#include "perifer.h"
#include "tui.h"

int vscr_width = 6 * 64;
int vscr_height = 9 * 25;

uns8 memory[65536];
uns8 memory_mon[2048];
uns8 memory_font[2048];

static SDL_Surface *screen;

static int fExit = 0;

//
static volatile uint64_t one_takt_delay = 0;
static volatile uint64_t one_takt_calib = 0;
static volatile uint64_t one_takt_one_percent = 0;

#if defined(__PPC__)
#warning "PPC32 code"

#define TIMEBASE 79800000

#define READ_TIMESTAMP(val) \
{				\
    unsigned int tmp;		\
    do {			\
	__asm__ __volatile__ (	\
	"mftbu %0 \n\t"		\
	"mftb  %1 \n\t"		\
	"mftbu %2 \n\t"		\
	: "=r"(((long*)&val)[0]), "=r"(((long*)&val)[1]), "=r"(tmp)	\
	:			\
	);			\
    } while (tmp != (((long*)&val)[0]));	\
}

#elif defined(__i386__)

#define READ_TIMESTAMP(val) \
    __asm__ __volatile__("rdtsc" : "=A" (val))

#elif defined(__x86_64__)

#define READ_TIMESTAMP(val) \
    __asm__ __volatile__("rdtsc" : "=a" (((int*)&val)[0]), "=d" (((int*)&val)[1]));

#else

#define READ_TIMESTAMP(var) readTSC(&var)

static void readTSC(volatile uint64_t *v)
{
    struct timespec tp;
    clock_gettime (CLOCK_REALTIME, &tp);
    *v = (uint64_t)(tp.tv_sec * (uint64_t)1000000000) + (uint64_t)tp.tv_nsec;
}

#endif

static void exitRequested(void)
{
    fExit = 1;
}

static void keyMatrix(int row, uns8 mask, int stat)
{
    if (stat)
	keyboard_state[row] |= mask;
    else
	keyboard_state[row] &= ~mask;
}

static void keyMod(uns8 mask, int stat)
{
    if (stat)
	keyboard_mod |= mask;
    else
	keyboard_mod &= ~mask;
}

static void ChecKeyboard(void)
{
    SDLKey sdlkey;
    SDL_Event event;
    int keystat = 2;

    if(SDL_WaitEvent(&event) > 0){
	switch(event.type) {
	case SDL_QUIT:
	    exitRequested();
	    break;
	case SDL_KEYDOWN:
	    keystat--;
	case SDL_KEYUP:
	    keystat--;
	    sdlkey=event.key.keysym.sym;
	    //fprintf(stderr, "KEY %02X %02X\n", sdlkey, keystat);
	    switch(sdlkey) {
	    case SDLK_LSHIFT: keyMod(SS, keystat); break;
	    case SDLK_LCTRL: keyMod(US, keystat); break;
	    case SDLK_LALT: keyMod(RL, keystat); break;

	    case SDLK_HOME: keyMatrix(0, 0x01, keystat); break;
	    case SDLK_PAGEUP: keyMatrix(0, 0x02, keystat); break;
	    case SDLK_ESCAPE: keyMatrix(0, 0x04, keystat); break;
	    case SDLK_F1: keyMatrix(0, 0x08, keystat); break;
	    case SDLK_F2: keyMatrix(0, 0x10, keystat); break;
	    case SDLK_F3: keyMatrix(0, 0x20, keystat); break;
	    case SDLK_F4: keyMatrix(0, 0x40, keystat); break;
	    case SDLK_F5: keyMatrix(0, 0x80, keystat); break;

	    case SDLK_TAB: keyMatrix(1, 0x01, keystat); break;
	    case SDLK_INSERT: keyMatrix(1, 0x02, keystat); break;
	    case SDLK_RETURN: keyMatrix(1, 0x04, keystat); break;
	    case SDLK_BACKSPACE: keyMatrix(1, 0x08, keystat); break;
	    case SDLK_LEFT: keyMatrix(1, 0x10, keystat); break;
	    case SDLK_UP: keyMatrix(1, 0x20, keystat); break;
	    case SDLK_RIGHT: keyMatrix(1, 0x40, keystat); break;
	    case SDLK_DOWN: keyMatrix(1, 0x80, keystat); break;

	    case SDLK_0: keyMatrix(2, 0x01, keystat); break;
	    case SDLK_1: keyMatrix(2, 0x02, keystat); break;
	    case SDLK_2: keyMatrix(2, 0x04, keystat); break;
	    case SDLK_3: keyMatrix(2, 0x08, keystat); break;
	    case SDLK_4: keyMatrix(2, 0x10, keystat); break;
	    case SDLK_5: keyMatrix(2, 0x20, keystat); break;
	    case SDLK_6: keyMatrix(2, 0x40, keystat); break;
	    case SDLK_7: keyMatrix(2, 0x80, keystat); break;

	    case SDLK_8: keyMatrix(3, 0x01, keystat); break;
	    case SDLK_9: keyMatrix(3, 0x02, keystat); break;
	    case SDLK_QUOTE: keyMatrix(3, 0x04, keystat); break;
	    case SDLK_SEMICOLON: keyMatrix(3, 0x08, keystat); break;
	    case SDLK_COMMA: keyMatrix(3, 0x10, keystat); break;
	    case SDLK_MINUS: keyMatrix(3, 0x20, keystat); break;
	    case SDLK_PERIOD: keyMatrix(3, 0x40, keystat); break;
	    case SDLK_SLASH: keyMatrix(3, 0x80, keystat); break;

	    case SDLK_EQUALS: keyMatrix(4, 0x01, keystat); break;
	    case SDLK_a: keyMatrix(4, 0x02, keystat); break;
	    case SDLK_b: keyMatrix(4, 0x04, keystat); break;
	    case SDLK_c: keyMatrix(4, 0x08, keystat); break;
	    case SDLK_d: keyMatrix(4, 0x10, keystat); break;
	    case SDLK_e: keyMatrix(4, 0x20, keystat); break;
	    case SDLK_f: keyMatrix(4, 0x40, keystat); break;
	    case SDLK_g: keyMatrix(4, 0x80, keystat); break;

	    case SDLK_h: keyMatrix(5, 0x01, keystat); break;
	    case SDLK_i: keyMatrix(5, 0x02, keystat); break;
	    case SDLK_j: keyMatrix(5, 0x04, keystat); break;
	    case SDLK_k: keyMatrix(5, 0x08, keystat); break;
	    case SDLK_l: keyMatrix(5, 0x10, keystat); break;
	    case SDLK_m: keyMatrix(5, 0x20, keystat); break;
	    case SDLK_n: keyMatrix(5, 0x40, keystat); break;
	    case SDLK_o: keyMatrix(5, 0x80, keystat); break;

	    case SDLK_p: keyMatrix(6, 0x01, keystat); break;
	    case SDLK_q: keyMatrix(6, 0x02, keystat); break;
	    case SDLK_r: keyMatrix(6, 0x04, keystat); break;
	    case SDLK_s: keyMatrix(6, 0x08, keystat); break;
	    case SDLK_t: keyMatrix(6, 0x10, keystat); break;
	    case SDLK_u: keyMatrix(6, 0x20, keystat); break;
	    case SDLK_v: keyMatrix(6, 0x40, keystat); break;
	    case SDLK_w: keyMatrix(6, 0x80, keystat); break;

	    case SDLK_x: keyMatrix(7, 0x01, keystat); break;
	    case SDLK_y: keyMatrix(7, 0x02, keystat); break;
	    case SDLK_z: keyMatrix(7, 0x04, keystat); break;
	    case SDLK_LEFTBRACKET: keyMatrix(7, 0x08, keystat); break;
	    case SDLK_BACKSLASH: keyMatrix(7, 0x10, keystat); break;
	    case SDLK_RIGHTBRACKET: keyMatrix(7, 0x20, keystat); break;
	    case SDLK_BACKQUOTE: keyMatrix(7, 0x40, keystat); break;
	    case SDLK_SPACE: keyMatrix(7, 0x80, keystat); break;

	    default: keystat = -1;
	    }
	    break;
	}
    }
}

int SDLCALL HandleKeyboard(void *unused)
{
    while (!fExit)
	ChecKeyboard();

    return 0;
}

int SDLCALL HandleVideo(void *unused)
{
    while (!fExit) {
	SDL_Flip(screen);
	usleep(20000);
    }

    return 0;
}

void draw_char(uns16 x, uns16 y, uns8 c)
{
    int w, h;
    unsigned short *vmem = screen->pixels;

    if (x > 63 || y > 24)
	return;

    vmem += y * vscr_width * 9 + x * 6;

    for (h = 0; h < 8; h++) {
	unsigned char p = memory_font[(c << 3) + h];
	for (w = 0; w < 6; w++) {
	    vmem[w] = (p & 0x20)?0:0xFFFF;
	    p <<= 1;
	}
	vmem += vscr_width;
    }
}

int loadimage(char *name, uns8 *ptr, uns16 len)
{
    FILE *f = fopen(name, "rb");
    if (!f)
	return 0;

    int ret = fread(ptr, 1, len, f);

    fclose(f);

    return ret;
}

int main(int argc, char *argv[])
{
    FILE *tape = NULL;
    Uint32 sys_flags;
    Uint32 vid_flags;
    SDL_Thread *video_thread;
    SDL_Thread *keybd_thread;

    sys_flags = 0;
#ifdef USE_JOYSTICK
    sys_flags |= SDL_INIT_JOYSTICK;
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | sys_flags) < 0) {
	fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
	exit(1);
    }

    fprintf(stderr, "RADIO-86RK emulator system\n");

    fprintf(stderr, "Loading Monitor ROM ... ");
    if (loadimage("mon32.rom", memory_mon, 2048) == 2048)
	fprintf(stderr, "Done\n");
    else
	fprintf(stderr, "Failed\n");

    fprintf(stderr, "Loading Font ROM    ... ");
    if (loadimage("font.rom", memory_font, 1024) == 1024)
	fprintf(stderr, "Done\n");
    else
	fprintf(stderr, "Failed\n");

    fprintf(stderr, "Detecting host cpu speed... ");
    {
	volatile uint64_t a;
	READ_TIMESTAMP(a);
	sleep(1);
	READ_TIMESTAMP(one_takt_delay);
#ifdef __PPC__
#warning "PPC PS3 calculation, fixme"
	one_takt_delay -= a;
	fprintf(stderr, "%lld MHz\n", one_takt_delay * 4 / 100000);
	one_takt_delay /= 1740000;
	one_takt_calib = one_takt_delay;
#else
	one_takt_delay -= a;
	one_takt_delay /= 1000000;
	fprintf(stderr, "%lld MHz\n", one_takt_delay);
	one_takt_delay /= 2;
	one_takt_calib = one_takt_delay;
#endif
    }

    vid_flags = SDL_HWSURFACE;
    vid_flags |= SDL_DOUBLEBUF;

    screen = SDL_SetVideoMode(vscr_width, vscr_height, 16, vid_flags);

    SDL_WM_SetCaption("RADIO-86RK Emulator", "RADIO86-RK");

    video_thread = SDL_CreateThread(HandleVideo, NULL);
    keybd_thread = SDL_CreateThread(HandleKeyboard, NULL);

    fExit = 0;

    i8080_init();
    i8080_reset();

    unsigned int old_Tstates = 0;
    volatile uint64_t ts1;
    READ_TIMESTAMP(ts1);
    while(!fExit) {

if (PC == 0xfb98) {
    fprintf(stderr, "input from tape [%02X] ->", A);
    if (!tape) {
	draw_box(5, 5, 64 - 10, 25 - 10, 'X');
	draw_box(6, 6, 64 - 12, 25 - 12, ' ');
	draw_string(8, 6, "HELLO WORLD!");
	tui_menu *menu = tui_menu_new(13,13);
	tui_menu_add_item(menu, "hell1");
	tui_menu_add_item(menu, "hell2");
	tui_menu_add_item(menu, "hell3");
	tui_menu_add_item(menu, "hell4");
	tui_menu_add_item(menu, "hell5");
	tui_menu_add_item(menu, "hell6");
	tui_menu_add_item(menu, "hell7");
	tui_menu_add_item(menu, "hell8");
	tui_menu_add_item(menu, "hell9");
	tui_menu_add_item(menu, "hell10");
	tui_menu_add_item(menu, "hell11");
	tui_menu_add_item(menu, "hell12");
	tui_menu_add_item(menu, "hell12");
	tui_menu_add_item(menu, "hell14");
	tui_menu_add_item(menu, "hell15");
	tui_menu_add_item(menu, "hell16");
	tui_menu_dump(menu);
	tui_menu_draw(3, 3, menu);
	tui_menu_free(menu);
	sleep(5);
	tape = fopen(argv[1], "rb");
    }
    if (tape) {
	int c = EOF;
	if (A == 0xff) {
	    do {
		c = fgetc(tape);
	    } while (c != EOF && c != 0xe6);
	    if (c == 0xe6)
		c = fgetc(tape);
	} else if (A == 0x08)
	    c = fgetc(tape);
	if (c == EOF) {
	    fclose(tape);
	    tape = NULL;
	}
	A = c;
    }
    fprintf(stderr, " [%02X]!\n", A);
    Tstates += 10;
    POP(PC);
}

	i8080_do_opcode();

	volatile uint64_t ts2;
	do {
	    READ_TIMESTAMP(ts2);
	} while ((ts2 - ts1) < (one_takt_delay * (Tstates - old_Tstates)));
	ts1 = ts2;
	old_Tstates = Tstates;
    }

    return 0;
}
