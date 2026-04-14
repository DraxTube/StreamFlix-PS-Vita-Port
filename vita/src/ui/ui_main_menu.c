/* Main Menu UI */
#include "ui_common.h"
#define CARD_W 140
#define CARD_H 210
#define CARD_PAD 10
#define ROW_H (CARD_H+40)
#define CAT_LBL_H 30

void ui_draw_main_menu(void) {
    ui_draw_header("StreamFlix","PS Vita");
    if(!g_app.categories||g_app.category_count<=0){
        ui_draw_text_center(SCREEN_H/2,"No content",COL_TEXT_GRAY,1.0f);
        ui_draw_footer("START: Search | SELECT: Exit"); return;
    }
    int yo=55-g_app.scroll_offset;
    for(int c=0;c<g_app.category_count;c++){
        Category *cat=&g_app.categories[c];
        int cy=yo+c*(ROW_H+CAT_LBL_H);
        if(cy+ROW_H+CAT_LBL_H<50) continue; if(cy>SCREEN_H) break;
        bool cs=(g_app.cursor_y==c);
        ui_draw_text(15,cy+20,cat->name,cs?COL_PRIMARY:COL_TEXT_LIGHT,0.9f);
        if(cs) ui_draw_rect_filled(5,cy+7,4,18,COL_PRIMARY);
        int cdy=cy+CAT_LBL_H;
        int rs=0; if(cs&&g_app.cursor_x>4) rs=(g_app.cursor_x-4)*(CARD_W+CARD_PAD);
        for(int i=0;i<cat->count;i++){
            int cx=15+i*(CARD_W+CARD_PAD)-rs;
            if(cx+CARD_W<0) continue; if(cx>SCREEN_W) break;
            ui_draw_show_card(cx,cdy,CARD_W,CARD_H,&cat->items[i],cs&&g_app.cursor_x==i);
        }
    }
    ui_draw_footer("X: Select | START: Search | SELECT: Exit");
}

void ui_update_main_menu(void) {
    if(!g_app.categories||g_app.category_count<=0) return;
    if(g_app.pressed&SCE_CTRL_DOWN){
        if(g_app.cursor_y<g_app.category_count-1){g_app.cursor_y++;g_app.cursor_x=0;
            int ty=g_app.cursor_y*(ROW_H+CAT_LBL_H);
            int vh=SCREEN_H-130;
            if(ty>g_app.scroll_offset+vh) g_app.scroll_offset=ty-vh+ROW_H;}
    }
    if(g_app.pressed&SCE_CTRL_UP){
        if(g_app.cursor_y>0){g_app.cursor_y--;g_app.cursor_x=0;
            int ty=g_app.cursor_y*(ROW_H+CAT_LBL_H);
            if(ty<g_app.scroll_offset) g_app.scroll_offset=ty;}
    }
    Category *cur=&g_app.categories[g_app.cursor_y];
    if(g_app.pressed&SCE_CTRL_RIGHT){if(g_app.cursor_x<cur->count-1)g_app.cursor_x++;}
    if(g_app.pressed&SCE_CTRL_LEFT){if(g_app.cursor_x>0)g_app.cursor_x--;}
    if(g_app.pressed&SCE_CTRL_CROSS){
        if(cur->count>0&&g_app.cursor_x<cur->count){
            memcpy(&g_app.selected_item,&cur->items[g_app.cursor_x],sizeof(ShowItem));
            app_load_detail();
        }
    }
}
