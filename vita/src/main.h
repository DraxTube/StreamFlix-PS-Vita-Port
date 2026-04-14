#ifndef MAIN_H
#define MAIN_H

#include <vita2d.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/sysmodule.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/apputil.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define SCREEN_W 960
#define SCREEN_H 544
#define APP_DATA_PATH "ux0:data/streamflix/"

/* Colors - dark theme with red accent */
#define COL_BG           RGBA8(15, 15, 20, 255)
#define COL_BG_CARD      RGBA8(28, 28, 38, 255)
#define COL_BG_CARD_SEL  RGBA8(45, 45, 65, 255)
#define COL_BG_HEADER    RGBA8(20, 20, 30, 240)
#define COL_PRIMARY      RGBA8(229, 57, 53, 255)
#define COL_PRIMARY_DIM  RGBA8(183, 28, 28, 255)
#define COL_ACCENT       RGBA8(255, 87, 34, 255)
#define COL_TEXT_WHITE    RGBA8(255, 255, 255, 255)
#define COL_TEXT_LIGHT    RGBA8(220, 220, 220, 255)
#define COL_TEXT_GRAY     RGBA8(158, 158, 158, 255)
#define COL_TEXT_DIM      RGBA8(100, 100, 100, 255)
#define COL_GOLD          RGBA8(255, 193, 7, 255)
#define COL_GREEN         RGBA8(76, 175, 80, 255)
#define COL_DIVIDER       RGBA8(48, 48, 58, 255)

typedef enum {
    STATE_INIT, STATE_MAIN_MENU, STATE_SEARCH, STATE_SEARCH_RESULTS,
    STATE_DETAIL, STATE_SEASONS, STATE_EPISODES, STATE_SERVERS,
    STATE_PLAYING, STATE_ERROR, STATE_LOADING, STATE_EXIT
} AppState;

typedef enum { CONTENT_MOVIE = 1, CONTENT_TVSHOW = 2 } ContentType;

typedef struct {
    int id;
    int box_type;
    char title[256];
    char overview[1024];
    char poster[512];
    char banner[512];
    char quality[32];
    char year[16];
    char rating[16];
    int runtime;
    char genres[256];
    char cast[512];
    char director[256];
} ShowItem;

typedef struct {
    char name[128];
    ShowItem *items;
    int count;
} Category;

typedef struct { int number; char id[64]; } Season;

typedef struct {
    int id; int number;
    char title[256]; char released[64]; char poster[512];
} Episode;

typedef struct {
    char url[1024]; char quality[32]; char size[64]; int fid;
} VideoLink;

typedef struct {
    AppState state, prev_state;
    SceCtrlData pad, pad_old;
    unsigned int pressed;
    Category *categories; int category_count;
    ShowItem *search_results; int search_result_count;
    char search_query[256];
    ShowItem selected_item;
    Season *seasons; int season_count; int selected_season_idx;
    Episode *episodes; int episode_count; int selected_episode_idx;
    VideoLink *links; int link_count; int selected_link_idx;
    int cursor_x, cursor_y, scroll_offset, menu_selection;
    char loading_msg[256]; char error_msg[512];
    vita2d_pgf *pgf;
    float anim_timer;
    bool net_initialized, running;
} AppContext;

extern AppContext g_app;

void app_init(void);
void app_cleanup(void);
void app_set_state(AppState new_state);
void app_load_home(void);
void app_load_search(void);
void app_load_detail(void);
void app_load_episodes(void);
void app_load_links(void);

#endif
