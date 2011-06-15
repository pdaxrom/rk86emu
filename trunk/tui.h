#ifndef _TUI_H_
#define _TUI_H_

void draw_string(uns16 x, uns16 y, char *str);

void draw_box(uns16 x, uns16 y, uns16 w, uns16 h, uns8 c);

typedef struct {} tui_menu;

tui_menu *tui_menu_new(uns16 w, uns16 h);

int tui_menu_add_item(tui_menu *m, char *text);

void tui_menu_free(tui_menu *m);

void tui_menu_dump(tui_menu *m);


#endif
