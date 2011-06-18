#ifndef _TUI_H_
#define _TUI_H_

enum {
    TUI_UP = 1,
    TUI_DOWN
};

typedef struct {
    char	*text;
    void	*next;
} menu_item;

typedef struct {
    uns16	w;
    uns16	h;
    uns16	vis_start;
    short	cur_pos;
    uns16	num_items;
    char	*text;
    menu_item	*item;
} tui_menu;

void draw_string(uns16 x, uns16 y, char *str);

void draw_box(uns16 x, uns16 y, uns16 w, uns16 h, uns8 c);

tui_menu *tui_menu_new(uns16 w, uns16 h, char *name);

int tui_menu_add_item(tui_menu *m, char *text);

void tui_menu_free(tui_menu *m);

void tui_menu_dump(tui_menu *m);

void tui_menu_draw(tui_menu *m, uns16 x, uns16 y);

void tui_menu_key(tui_menu *m, uns16 key);

char *tui_menu_get_item(tui_menu *m);

#endif
