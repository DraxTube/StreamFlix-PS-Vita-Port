#include "ui_common.h"
#include <math.h>
#include <string.h>

void ui_draw_rect_filled(int x, int y, int w, int h, unsigned int c) { vita2d_draw_rectangle(x,y,w,h,c); }

void ui_draw_text(int x, int y, const char *t, unsigned int c, float s) {
    if(g_app.pgf&&t) vita2d_pgf_draw_text(g_app.pgf,x,y,c,s,t);
}
void ui_draw_text_center(int y, const char *t, unsigned int c, float s) {
    if(!g_app.pgf||!t) return;
    int w=vita2d_pgf_text_width(g_app.pgf,s,t); vita2d_pgf_draw_text(g_app.pgf,(SCREEN_W-w)/2,y,c,s,t);
}
void ui_draw_text_right(int x, int y, const char *t, unsigned int c, float s) {
    if(!g_app.pgf||!t) return;
    int w=vita2d_pgf_text_width(g_app.pgf,s,t); vita2d_pgf_draw_text(g_app.pgf,x-w,y,c,s,t);
}
int ui_text_width(const char *t, float s) { return (g_app.pgf&&t)?vita2d_pgf_text_width(g_app.pgf,s,t):0; }

void ui_draw_header(const char *title, const char *sub) {
    ui_draw_rect_filled(0,0,SCREEN_W,50,COL_BG_HEADER);
    ui_draw_rect_filled(0,48,SCREEN_W,2,COL_PRIMARY);
    ui_draw_text(15,32,title,COL_PRIMARY,1.2f);
    if(sub&&sub[0]) ui_draw_text_right(SCREEN_W-15,32,sub,COL_TEXT_GRAY,0.8f);
}
void ui_draw_footer(const char *h) {
    ui_draw_rect_filled(0,SCREEN_H-35,SCREEN_W,35,COL_BG_HEADER);
    ui_draw_rect_filled(0,SCREEN_H-35,SCREEN_W,1,COL_DIVIDER);
    if(h) ui_draw_text_center(SCREEN_H-12,h,COL_TEXT_GRAY,0.75f);
}
void ui_draw_spinner(int cx, int cy, float timer) {
    for(int i=0;i<8;i++){
        float a=(float)i/8.0f*6.2832f+timer*3.0f;
        int dx=cx+(int)(cosf(a)*20); int dy=cy+(int)(sinf(a)*20);
        unsigned int al=(unsigned int)((1.0f-(float)i/8.0f)*255);
        vita2d_draw_fill_circle(dx,dy,4,RGBA8(229,57,53,al));
    }
}
void ui_draw_badge(int x, int y, const char *t, unsigned int bg) {
    if(!t||!t[0]) return;
    int tw=ui_text_width(t,0.65f);
    ui_draw_rect_filled(x,y,tw+12,20,bg);
    ui_draw_text(x+6,y+15,t,COL_TEXT_WHITE,0.65f);
}
void ui_draw_show_card(int x, int y, int w, int h, const ShowItem *it, bool sel) {
    if(!it) return;
    ui_draw_rect_filled(x,y,w,h,sel?COL_BG_CARD_SEL:COL_BG_CARD);
    if(sel){ui_draw_rect_filled(x,y,w,2,COL_PRIMARY);ui_draw_rect_filled(x,y,2,h,COL_PRIMARY);
        ui_draw_rect_filled(x+w-2,y,2,h,COL_PRIMARY);ui_draw_rect_filled(x,y+h-2,w,2,COL_PRIMARY);}
    int ph=h-45; if(ph<20)ph=20;
    unsigned int hash=0; for(const char *p=it->title;*p;p++) hash=hash*31+*p;
    ui_draw_rect_filled(x+4,y+4,w-8,ph,RGBA8(40+(hash%60),30+((hash>>8)%60),50+((hash>>16)%80),255));
    if(it->title[0]){char ini[2]={it->title[0],0};
        ui_draw_text(x+(w-ui_text_width(ini,2.0f))/2,y+ph/2+15,ini,COL_TEXT_WHITE,2.0f);}
    if(it->quality[0]) ui_draw_badge(x+6,y+6,it->quality,COL_PRIMARY);
    char tb[40]; strncpy(tb,it->title,30); tb[30]=0;
    if(strlen(it->title)>30) strcat(tb,"...");
    ui_draw_text(x+6,y+ph+16,tb,COL_TEXT_WHITE,0.7f);
    char info[64]=""; if(it->year[0]){strcat(info,it->year);}
    if(it->rating[0]){if(info[0])strcat(info," | ");strcat(info,it->rating);}
    if(info[0]) ui_draw_text(x+6,y+ph+32,info,COL_TEXT_GRAY,0.6f);
}
void ui_draw_list_item(int x, int y, int w, const char *t, const char *d, bool sel) {
    ui_draw_rect_filled(x,y,w,44,sel?COL_BG_CARD_SEL:COL_BG_CARD);
    if(sel) ui_draw_rect_filled(x,y,3,44,COL_PRIMARY);
    if(t) ui_draw_text(x+15,y+28,t,sel?COL_TEXT_WHITE:COL_TEXT_LIGHT,0.85f);
    if(d) ui_draw_text_right(x+w-15,y+28,d,COL_TEXT_GRAY,0.7f);
    ui_draw_rect_filled(x+10,y+43,w-20,1,COL_DIVIDER);
}
void ui_draw_loading(void) {
    ui_draw_rect_filled(0,0,SCREEN_W,SCREEN_H,COL_BG);
    ui_draw_spinner(SCREEN_W/2,SCREEN_H/2-20,g_app.anim_timer);
    ui_draw_text_center(SCREEN_H/2+30,g_app.loading_msg,COL_TEXT_LIGHT,0.9f);
    /* Animated dots */
    int dots=(int)(g_app.anim_timer*2)%4;
    char db[8]=""; for(int i=0;i<dots;i++) strcat(db,".");
    ui_draw_text(SCREEN_W/2+ui_text_width(g_app.loading_msg,0.9f)/2+2,SCREEN_H/2+30,db,COL_TEXT_LIGHT,0.9f);
}
void ui_draw_error(void) {
    ui_draw_rect_filled(0,0,SCREEN_W,SCREEN_H,COL_BG);
    vita2d_draw_fill_circle(SCREEN_W/2,SCREEN_H/2-60,35,COL_PRIMARY_DIM);
    ui_draw_text(SCREEN_W/2-8,SCREEN_H/2-48,"!",COL_TEXT_WHITE,2.0f);
    /* Split error message by newlines */
    char buf[512]; strncpy(buf,g_app.error_msg,511); buf[511]=0;
    int y=SCREEN_H/2; char *line=strtok(buf,"\n");
    while(line){ui_draw_text_center(y,line,COL_TEXT_LIGHT,0.85f); y+=22; line=strtok(NULL,"\n");}
}
int ui_clamp(int v, int mn, int mx) { return v<mn?mn:(v>mx?mx:v); }
