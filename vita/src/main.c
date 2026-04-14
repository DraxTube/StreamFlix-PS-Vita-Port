/*
 * StreamFlix Vita - Main Entry Point
 */
#include "main.h"
#include "net/http_client.h"
#include "providers/superstream.h"
#include "ui/ui_common.h"
#include "db/database.h"
#include <psp2/kernel/threadmgr.h>

AppContext g_app;
static char net_memory[1 * 1024 * 1024];

static void init_network(void) {
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    SceNetInitParam p; p.memory = net_memory; p.size = sizeof(net_memory); p.flags = 0;
    sceNetInit(&p); sceNetCtlInit();
    g_app.net_initialized = true;
}

static void cleanup_network(void) {
    if (!g_app.net_initialized) return;
    sceNetCtlTerm(); sceNetTerm();
}

void app_init(void) {
    memset(&g_app, 0, sizeof(AppContext));
    g_app.running = true; g_app.state = STATE_INIT;
    vita2d_init(); vita2d_set_clear_color(COL_BG);
    g_app.pgf = vita2d_load_default_pgf();
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
    SceAppUtilInitParam ip; SceAppUtilBootParam bp;
    memset(&ip,0,sizeof(ip)); memset(&bp,0,sizeof(bp));
    sceAppUtilInit(&ip,&bp);
    init_network(); http_client_init();
    sceIoMkdir(APP_DATA_PATH, 0777);
    db_init();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
}

void app_cleanup(void) {
    db_close(); http_client_cleanup(); cleanup_network();
    if (g_app.pgf) vita2d_free_pgf(g_app.pgf);
    if (g_app.categories) {
        for (int i=0;i<g_app.category_count;i++) free(g_app.categories[i].items);
        free(g_app.categories);
    }
    free(g_app.search_results); free(g_app.seasons);
    free(g_app.episodes); free(g_app.links);
    vita2d_fini();
}

void app_set_state(AppState s) {
    g_app.prev_state=g_app.state; g_app.state=s;
    g_app.cursor_x=0; g_app.cursor_y=0; g_app.scroll_offset=0; g_app.anim_timer=0;
}

static void update_input(void) {
    g_app.pad_old=g_app.pad;
    sceCtrlPeekBufferPositive(0,&g_app.pad,1);
    g_app.pressed=g_app.pad.buttons & ~g_app.pad_old.buttons;
}

/* Loading threads */
static int load_home_thread(SceSize a, void *b) {
    (void)a;(void)b;
    if (ss_get_home(&g_app.categories,&g_app.category_count)==0 && g_app.category_count>0)
        app_set_state(STATE_MAIN_MENU);
    else { strcpy(g_app.error_msg,"Failed to load.\nX: Retry  O: Exit"); app_set_state(STATE_ERROR); }
    return 0;
}
static int load_search_thread(SceSize a, void *b) {
    (void)a;(void)b;
    if (ss_search(g_app.search_query,1,&g_app.search_results,&g_app.search_result_count)==0)
        app_set_state(STATE_SEARCH_RESULTS);
    else { strcpy(g_app.error_msg,"Search failed.\nO: Back"); app_set_state(STATE_ERROR); }
    return 0;
}
static int load_detail_thread(SceSize a, void *b) {
    (void)a;(void)b;
    ShowItem *it=&g_app.selected_item;
    int ret;
    if (it->box_type==CONTENT_MOVIE) ret=ss_get_movie_detail(it->id,it);
    else ret=ss_get_tvshow_detail(it->id,it,&g_app.seasons,&g_app.season_count);
    if (ret==0) app_set_state(STATE_DETAIL);
    else { strcpy(g_app.error_msg,"Load failed.\nO: Back"); app_set_state(STATE_ERROR); }
    return 0;
}
static int load_episodes_thread(SceSize a, void *b) {
    (void)a;(void)b;
    if (ss_get_episodes(g_app.selected_item.id,g_app.seasons[g_app.selected_season_idx].number,
        &g_app.episodes,&g_app.episode_count)==0) app_set_state(STATE_EPISODES);
    else { strcpy(g_app.error_msg,"Load failed.\nO: Back"); app_set_state(STATE_ERROR); }
    return 0;
}
static int load_links_thread(SceSize a, void *b) {
    (void)a;(void)b;
    int ret;
    if (g_app.selected_item.box_type==CONTENT_MOVIE)
        ret=ss_get_movie_links(g_app.selected_item.id,&g_app.links,&g_app.link_count);
    else ret=ss_get_episode_links(g_app.selected_item.id,
        g_app.seasons[g_app.selected_season_idx].number,
        g_app.episodes[g_app.selected_episode_idx].number,&g_app.links,&g_app.link_count);
    if (ret==0 && g_app.link_count>0) app_set_state(STATE_SERVERS);
    else { strcpy(g_app.error_msg,"No sources found.\nO: Back"); app_set_state(STATE_ERROR); }
    return 0;
}

static void start_thread(int(*f)(SceSize,void*), const char *msg) {
    strcpy(g_app.loading_msg,msg); g_app.state=STATE_LOADING;
    SceUID t=sceKernelCreateThread("ld",f,0x10000100,0x100000,0,0,NULL);
    if (t>=0) sceKernelStartThread(t,0,NULL);
}

void app_load_home(void)     { start_thread(load_home_thread,"Loading..."); }
void app_load_search(void)   { start_thread(load_search_thread,"Searching..."); }
void app_load_detail(void)   { start_thread(load_detail_thread,"Loading details..."); }
void app_load_episodes(void) { start_thread(load_episodes_thread,"Loading episodes..."); }
void app_load_links(void)    { start_thread(load_links_thread,"Loading sources..."); }

/* UI forward decls */
void ui_draw_main_menu(void); void ui_update_main_menu(void);
void ui_draw_search(void); void ui_update_search(void);
void ui_draw_search_results(void); void ui_update_search_results(void);
void ui_draw_detail(void); void ui_update_detail(void);
void ui_draw_episodes(void); void ui_update_episodes(void);
void ui_draw_servers(void); void ui_update_servers(void);
void ui_draw_loading(void); void ui_draw_error(void);
void ui_draw_player(void); void ui_update_player(void);

int main(void) {
    app_init();
    app_load_home();

    while (g_app.running) {
        update_input();
        g_app.anim_timer += 0.016f;

        switch (g_app.state) {
            case STATE_MAIN_MENU:    ui_update_main_menu(); break;
            case STATE_SEARCH:       ui_update_search(); break;
            case STATE_SEARCH_RESULTS: ui_update_search_results(); break;
            case STATE_DETAIL:       ui_update_detail(); break;
            case STATE_EPISODES:     ui_update_episodes(); break;
            case STATE_SERVERS:      ui_update_servers(); break;
            case STATE_PLAYING:      ui_update_player(); break;
            case STATE_ERROR:
                if (g_app.pressed & SCE_CTRL_CROSS) app_load_home();
                if (g_app.pressed & SCE_CTRL_CIRCLE) {
                    if (g_app.prev_state==STATE_INIT) g_app.running=false;
                    else app_set_state(g_app.prev_state);
                }
                break;
            default: break;
        }
        if (g_app.state==STATE_MAIN_MENU && (g_app.pressed & SCE_CTRL_START))
            app_set_state(STATE_SEARCH);
        if ((g_app.pressed & SCE_CTRL_SELECT) && g_app.state==STATE_MAIN_MENU)
            g_app.running=false;

        vita2d_start_drawing(); vita2d_clear_screen();
        switch (g_app.state) {
            case STATE_MAIN_MENU:      ui_draw_main_menu(); break;
            case STATE_SEARCH:         ui_draw_search(); break;
            case STATE_SEARCH_RESULTS: ui_draw_search_results(); break;
            case STATE_DETAIL:         ui_draw_detail(); break;
            case STATE_EPISODES:       ui_draw_episodes(); break;
            case STATE_SERVERS:        ui_draw_servers(); break;
            case STATE_PLAYING:        ui_draw_player(); break;
            case STATE_LOADING:        ui_draw_loading(); break;
            case STATE_ERROR:          ui_draw_error(); break;
            default: break;
        }
        vita2d_end_drawing(); vita2d_swap_buffers(); sceDisplayWaitVblankStart();
    }
    app_cleanup(); sceKernelExitProcess(0); return 0;
}
