#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i8080.h"
//#include "tui.h"

typedef struct {
    char	*text;
    void	*next;
} menu_item;

typedef struct {
    uns16	w;
    uns16	h;
    uns16	vis_start;
    uns16	cur_pos;
    menu_item	*item;
} tui_menu;

void draw_char(uns16 x, uns16 y, uns8 c);

void draw_string(uns16 x, uns16 y, char *str)
{
    while (*str)
	draw_char(x++, y, *str++);
}

void draw_box(uns16 x, uns16 y, uns16 w, uns16 h, uns8 c)
{
    int i, j;
    for (j = 0; j < h; j++)
	for (i = 0; i < w; i++)
	    draw_char(x + i, y +j, c);
}

tui_menu *tui_menu_new(uns16 w, uns16 h)
{
    tui_menu *tmp = malloc(sizeof(tui_menu));
    if (!tmp)
	return NULL;

    tmp->w = w;
    tmp->h = h;
    tmp->vis_start = 0;
    tmp->cur_pos = 0;
    tmp->item = NULL;

    return tmp;
}

int tui_menu_add_item(tui_menu *m, char *text)
{
    menu_item *tmp = malloc(sizeof(menu_item));
    if (!tmp)
	return -1;

    tmp->text = strdup(text);
    tmp->next = NULL;

    if (!m->item)
	m->item = tmp;
    else {
	menu_item *item = m->item;
	while (item->next)
	    item = item->next;
	item->next = tmp;
    }

    return 0;
}

void tui_menu_free(tui_menu *m)
{
    menu_item *item = m->item;
    while (item) {
	menu_item *tmp = item->next;
	free(item->text);
	free(item);
	item = tmp;
    }
    free(m);
}

void tui_menu_dump(tui_menu *m)
{
    menu_item *item = m->item;
    while (item) {
	fprintf(stderr, "-> %s\n", item->text);
	item = item->next;
    }
}

void tui_menu_draw(uns16 x, uns16 y, tui_menu *m)
{
    int i, j;
    int w = m->w;
    int h = m->h;

    menu_item *item = m->item;
    draw_box(x, y, w, h, 'X');
    draw_box(x + 1, y + 1, w - 2, h - 2, ' ');
    for (i = 0; i != m->vis_start && item; item = item->next, i++) ;
fprintf(stderr, ">>>%d\n", i);
    if (item) {
	for (i = 0, j = y + 1; j < y + h - 1 && item; item = item->next, j++, i++) {
	    if (i == m->cur_pos)
		draw_char(x + 1, j, '>');
	    draw_string(x + 2, j, item->text);
	}
    }
}
