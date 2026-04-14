#ifndef UI_COMMON_H
#define UI_COMMON_H
#include "../main.h"
void ui_draw_rect_filled(int x, int y, int w, int h, unsigned int color);
void ui_draw_text(int x, int y, const char *text, unsigned int color, float scale);
void ui_draw_text_center(int y, const char *text, unsigned int color, float scale);
void ui_draw_text_right(int x, int y, const char *text, unsigned int color, float scale);
int ui_text_width(const char *text, float scale);
void ui_draw_header(const char *title, const char *subtitle);
void ui_draw_footer(const char *hints);
void ui_draw_spinner(int cx, int cy, float timer);
void ui_draw_badge(int x, int y, const char *text, unsigned int bg_color);
void ui_draw_show_card(int x, int y, int w, int h, const ShowItem *item, bool selected);
void ui_draw_list_item(int x, int y, int w, const char *text, const char *detail, bool selected);
void ui_draw_loading(void);
void ui_draw_error(void);
int ui_clamp(int val, int mn, int mx);
#endif
