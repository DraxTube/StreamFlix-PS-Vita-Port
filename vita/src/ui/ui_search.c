/* Search UI with Vita IME */
#include "ui_common.h"
#include <psp2/ime_dialog.h>
#include <psp2/common_dialog.h>

static SceWChar16 ime_title[32], ime_input[256], ime_init[256];
static char ime_result[256];
static bool ime_active=false;

static void u16to8(const SceWChar16 *s, char *d, int ds) {
    int i=0,j=0;
    while(s[i]&&j<ds-1){
        if(s[i]<0x80) d[j++]=(char)s[i];
        else if(s[i]<0x800){if(j+2>ds-1)break;d[j++]=0xC0|(s[i]>>6);d[j++]=0x80|(s[i]&0x3F);}
        else{if(j+3>ds-1)break;d[j++]=0xE0|(s[i]>>12);d[j++]=0x80|((s[i]>>6)&0x3F);d[j++]=0x80|(s[i]&0x3F);}
        i++;
    } d[j]=0;
}
static void u8to16(const char *s, SceWChar16 *d, int ds) {
    int i=0,j=0;
    while(s[i]&&j<ds-1){unsigned char c=(unsigned char)s[i];
        if(c<0x80){d[j++]=c;i++;}
        else if((c&0xE0)==0xC0){d[j++]=((c&0x1F)<<6)|(s[i+1]&0x3F);i+=2;}
        else if((c&0xF0)==0xE0){d[j++]=((c&0x0F)<<12)|((s[i+1]&0x3F)<<6)|(s[i+2]&0x3F);i+=3;}
        else i++;
    } d[j]=0;
}

static void open_ime(void) {
    if(ime_active) return;
    u8to16("Search",ime_title,32);
    memset(ime_input,0,sizeof(ime_input)); memset(ime_init,0,sizeof(ime_init));
    if(g_app.search_query[0]) u8to16(g_app.search_query,ime_init,256);
    SceImeDialogParam p; sceImeDialogParamInit(&p);
    p.supportedLanguages=0; p.languagesForced=SCE_FALSE;
    p.type=SCE_IME_TYPE_DEFAULT; p.option=0;
    p.textBoxMode=SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT;
    p.title=ime_title; p.maxTextLength=128;
    p.initialText=ime_init; p.inputTextBuffer=ime_input;
    sceImeDialogInit(&p); ime_active=true;
}

void ui_draw_search(void) {
    ui_draw_header("Search","");
    int cy=SCREEN_H/2-40;
    vita2d_draw_fill_circle(SCREEN_W/2,cy-60,30,COL_PRIMARY_DIM);
    vita2d_draw_fill_circle(SCREEN_W/2,cy-60,26,COL_BG);
    ui_draw_text_center(cy,"Press X to open keyboard",COL_TEXT_LIGHT,1.0f);
    ui_draw_text_center(cy+30,"Search movies and TV shows",COL_TEXT_GRAY,0.8f);
    if(g_app.search_query[0]){char b[300];snprintf(b,300,"Last: \"%s\"",g_app.search_query);
        ui_draw_text_center(cy+70,b,COL_TEXT_DIM,0.75f);}
    ui_draw_footer("X: Keyboard | O: Back");
}

void ui_update_search(void) {
    if(ime_active){
        SceCommonDialogStatus st=sceImeDialogGetStatus();
        if(st==SCE_COMMON_DIALOG_STATUS_FINISHED){
            SceImeDialogResult r; memset(&r,0,sizeof(r)); sceImeDialogGetResult(&r);
            if(r.button==SCE_IME_DIALOG_BUTTON_ENTER){
                u16to8(ime_input,ime_result,256);
                if(ime_result[0]){strncpy(g_app.search_query,ime_result,255);
                    sceImeDialogTerm();ime_active=false;app_load_search();return;}
            }
            sceImeDialogTerm();ime_active=false;
        }
        return;
    }
    if(g_app.pressed&SCE_CTRL_CROSS) open_ime();
    if(g_app.pressed&SCE_CTRL_CIRCLE) app_set_state(STATE_MAIN_MENU);
}

/* Search Results */
void ui_draw_search_results(void) {
    char hdr[256]; snprintf(hdr,256,"\"%s\"",g_app.search_query);
    ui_draw_header("Results",hdr);
    if(!g_app.search_results||g_app.search_result_count<=0){
        ui_draw_text_center(SCREEN_H/2,"No results found",COL_TEXT_GRAY,1.0f);
        ui_draw_footer("O: Back"); return;
    }
    int sy=58,ih=50,vi=(SCREEN_H-sy-40)/ih,ss=0;
    if(g_app.cursor_y>=vi) ss=g_app.cursor_y-vi+1;
    char cs[32]; snprintf(cs,32,"%d results",g_app.search_result_count);
    ui_draw_text_right(SCREEN_W-15,sy-5,cs,COL_TEXT_DIM,0.65f);
    for(int i=ss;i<g_app.search_result_count;i++){
        int y=sy+(i-ss)*ih; if(y+ih>SCREEN_H-40) break;
        ShowItem *it=&g_app.search_results[i]; bool sel=(g_app.cursor_y==i);
        const char *tp=it->box_type==CONTENT_MOVIE?"MOVIE":"SHOW";
        char det[128]=""; if(it->year[0]){strcat(det,it->year);strcat(det," ");}
        strcat(det,tp); if(it->rating[0]){strcat(det," | ");strcat(det,it->rating);}
        ui_draw_list_item(10,y,SCREEN_W-20,it->title,det,sel);
    }
    /* Scrollbar */
    if(g_app.search_result_count>vi){int bh=SCREEN_H-sy-40;
        int th=(vi*bh)/g_app.search_result_count; if(th<20)th=20;
        int ty=sy+(ss*bh)/g_app.search_result_count;
        ui_draw_rect_filled(SCREEN_W-4,sy,3,bh,COL_DIVIDER);
        ui_draw_rect_filled(SCREEN_W-4,ty,3,th,COL_PRIMARY);}
    ui_draw_footer("X: Select | O: Back | START: New Search");
}

void ui_update_search_results(void) {
    if(!g_app.search_results) return;
    if(g_app.pressed&SCE_CTRL_DOWN){if(g_app.cursor_y<g_app.search_result_count-1)g_app.cursor_y++;}
    if(g_app.pressed&SCE_CTRL_UP){if(g_app.cursor_y>0)g_app.cursor_y--;}
    if(g_app.pressed&SCE_CTRL_CROSS){
        if(g_app.cursor_y<g_app.search_result_count){
            memcpy(&g_app.selected_item,&g_app.search_results[g_app.cursor_y],sizeof(ShowItem));
            app_load_detail();
        }
    }
    if(g_app.pressed&SCE_CTRL_CIRCLE) app_set_state(STATE_SEARCH);
    if(g_app.pressed&SCE_CTRL_START) app_set_state(STATE_SEARCH);
}
