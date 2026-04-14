/* Detail + Seasons + Episodes + Servers UI */
#include "ui_common.h"

void ui_draw_detail(void) {
    ShowItem *it = &g_app.selected_item;
    /* Background color strip */
    unsigned int hash = 0;
    for (const char *p = it->title; *p; p++) hash = hash * 31 + *p;
    ui_draw_rect_filled(0, 0, SCREEN_W, 160, RGBA8(20+(hash%30), 15+((hash>>8)%25), 30+((hash>>16)%40), 255));
    ui_draw_rect_filled(0, 140, SCREEN_W, 20, COL_BG); /* fade */

    /* Title */
    ui_draw_text(20, 45, it->title, COL_TEXT_WHITE, 1.3f);

    /* Info line */
    char info[256] = "";
    if (it->year[0]) { strcat(info, it->year); strcat(info, "  "); }
    if (it->rating[0]) { strcat(info, "Rating: "); strcat(info, it->rating); strcat(info, "  "); }
    if (it->runtime > 0) { char rt[32]; snprintf(rt, 32, "%dmin", it->runtime); strcat(info, rt); }
    ui_draw_text(20, 72, info, COL_TEXT_LIGHT, 0.8f);

    /* Quality badge */
    if (it->quality[0]) ui_draw_badge(20, 82, it->quality, COL_PRIMARY);

    /* Type badge */
    const char *type_str = it->box_type == CONTENT_MOVIE ? "MOVIE" : "TV SHOW";
    ui_draw_badge(20 + (it->quality[0] ? ui_text_width(it->quality, 0.65f) + 24 : 0), 82, type_str, COL_ACCENT);

    /* Genres */
    if (it->genres[0]) {
        ui_draw_text(20, 120, it->genres, COL_TEXT_GRAY, 0.7f);
    }

    /* Overview - word wrap simple */
    int oy = 170, ox = 20, max_w = SCREEN_W - 40;
    ui_draw_text(ox, oy - 10, "Overview", COL_PRIMARY, 0.85f);
    ui_draw_rect_filled(ox, oy - 2, 60, 2, COL_PRIMARY);

    if (it->overview[0]) {
        /* Simple line splitting */
        char buf[1024]; strncpy(buf, it->overview, 1023); buf[1023] = 0;
        char *word = strtok(buf, " ");
        char line[256] = ""; int ly = oy + 18;
        while (word && ly < SCREEN_H - 120) {
            char test[256]; snprintf(test, 256, "%s%s%s", line, line[0] ? " " : "", word);
            if (ui_text_width(test, 0.7f) > max_w) {
                ui_draw_text(ox, ly, line, COL_TEXT_LIGHT, 0.7f);
                ly += 18; strcpy(line, word);
            } else { strcpy(line, test); }
            word = strtok(NULL, " ");
        }
        if (line[0]) ui_draw_text(ox, ly, line, COL_TEXT_LIGHT, 0.7f);
    }

    /* Cast */
    if (it->cast[0]) {
        int cy = SCREEN_H - 110;
        ui_draw_text(20, cy, "Cast", COL_PRIMARY, 0.75f);
        char cb[256]; strncpy(cb, it->cast, 80); cb[80] = 0;
        if (strlen(it->cast) > 80) strcat(cb, "...");
        ui_draw_text(20, cy + 18, cb, COL_TEXT_GRAY, 0.65f);
    }

    /* Action buttons area */
    int by = SCREEN_H - 65;
    ui_draw_rect_filled(0, by - 5, SCREEN_W, 1, COL_DIVIDER);

    if (it->box_type == CONTENT_MOVIE) {
        bool sel = (g_app.cursor_y == 0);
        ui_draw_rect_filled(20, by, 200, 35, sel ? COL_PRIMARY : COL_BG_CARD);
        ui_draw_text(50, by + 24, "Play Movie", sel ? COL_TEXT_WHITE : COL_TEXT_LIGHT, 0.9f);
    } else {
        /* Show seasons */
        if (g_app.seasons && g_app.season_count > 0) {
            for (int i = 0; i < g_app.season_count && i < 8; i++) {
                int bx = 20 + i * 110;
                bool sel = (g_app.cursor_x == i);
                ui_draw_rect_filled(bx, by, 100, 35, sel ? COL_PRIMARY : COL_BG_CARD);
                char sn[32]; snprintf(sn, 32, "Season %d", g_app.seasons[i].number);
                ui_draw_text(bx + 10, by + 24, sn, sel ? COL_TEXT_WHITE : COL_TEXT_LIGHT, 0.75f);
            }
        }
    }

    ui_draw_footer("X: Select | O: Back");
}

void ui_update_detail(void) {
    ShowItem *it = &g_app.selected_item;

    if (it->box_type == CONTENT_MOVIE) {
        if (g_app.pressed & SCE_CTRL_CROSS) {
            app_load_links();
        }
    } else {
        /* Navigate seasons */
        if (g_app.pressed & SCE_CTRL_RIGHT) {
            if (g_app.cursor_x < g_app.season_count - 1) g_app.cursor_x++;
        }
        if (g_app.pressed & SCE_CTRL_LEFT) {
            if (g_app.cursor_x > 0) g_app.cursor_x--;
        }
        if (g_app.pressed & SCE_CTRL_CROSS) {
            if (g_app.season_count > 0) {
                g_app.selected_season_idx = g_app.cursor_x;
                app_load_episodes();
            }
        }
    }
    if (g_app.pressed & SCE_CTRL_CIRCLE) app_set_state(STATE_MAIN_MENU);
}

/* Episodes list */
void ui_draw_episodes(void) {
    char hdr[128]; snprintf(hdr, 128, "Season %d",
        g_app.seasons[g_app.selected_season_idx].number);
    ui_draw_header(g_app.selected_item.title, hdr);

    if (!g_app.episodes || g_app.episode_count <= 0) {
        ui_draw_text_center(SCREEN_H / 2, "No episodes", COL_TEXT_GRAY, 1.0f);
        ui_draw_footer("O: Back"); return;
    }

    int sy = 58, ih = 50, vi = (SCREEN_H - sy - 40) / ih, ss = 0;
    if (g_app.cursor_y >= vi) ss = g_app.cursor_y - vi + 1;

    for (int i = ss; i < g_app.episode_count; i++) {
        int y = sy + (i - ss) * ih;
        if (y + ih > SCREEN_H - 40) break;
        Episode *ep = &g_app.episodes[i];
        bool sel = (g_app.cursor_y == i);

        char nm[256];
        snprintf(nm, 256, "E%02d - %s", ep->number, ep->title[0] ? ep->title : "Episode");
        ui_draw_list_item(10, y, SCREEN_W - 20, nm, ep->released, sel);
    }

    if (g_app.episode_count > vi) {
        int bh = SCREEN_H - sy - 40;
        int th = (vi * bh) / g_app.episode_count; if (th < 20) th = 20;
        int ty = sy + (ss * bh) / g_app.episode_count;
        ui_draw_rect_filled(SCREEN_W - 4, sy, 3, bh, COL_DIVIDER);
        ui_draw_rect_filled(SCREEN_W - 4, ty, 3, th, COL_PRIMARY);
    }

    ui_draw_footer("X: Select | O: Back");
}

void ui_update_episodes(void) {
    if (!g_app.episodes) return;
    if (g_app.pressed & SCE_CTRL_DOWN) { if (g_app.cursor_y < g_app.episode_count - 1) g_app.cursor_y++; }
    if (g_app.pressed & SCE_CTRL_UP) { if (g_app.cursor_y > 0) g_app.cursor_y--; }
    if (g_app.pressed & SCE_CTRL_CROSS) {
        g_app.selected_episode_idx = g_app.cursor_y;
        app_load_links();
    }
    if (g_app.pressed & SCE_CTRL_CIRCLE) app_set_state(STATE_DETAIL);
}

/* Servers / Quality selection */
void ui_draw_servers(void) {
    ui_draw_header("Select Quality", "");

    if (!g_app.links || g_app.link_count <= 0) {
        ui_draw_text_center(SCREEN_H / 2, "No sources", COL_TEXT_GRAY, 1.0f);
        ui_draw_footer("O: Back"); return;
    }

    int sy = 70;
    ui_draw_text(20, sy - 8, "Available qualities:", COL_TEXT_GRAY, 0.75f);

    for (int i = 0; i < g_app.link_count; i++) {
        int y = sy + i * 55;
        if (y > SCREEN_H - 60) break;
        VideoLink *lk = &g_app.links[i];
        bool sel = (g_app.cursor_y == i);

        ui_draw_rect_filled(20, y, SCREEN_W - 40, 48, sel ? COL_BG_CARD_SEL : COL_BG_CARD);
        if (sel) {
            ui_draw_rect_filled(20, y, 4, 48, COL_PRIMARY);
            ui_draw_rect_filled(20, y, SCREEN_W - 40, 2, COL_PRIMARY);
        }

        /* Quality badge */
        ui_draw_badge(35, y + 8, lk->quality, COL_PRIMARY);

        /* Size */
        if (lk->size[0]) {
            ui_draw_text(35, y + 38, lk->size, COL_TEXT_GRAY, 0.7f);
        }

        /* Play icon on selected */
        if (sel) {
            ui_draw_text(SCREEN_W - 80, y + 30, ">>", COL_PRIMARY, 0.9f);
        }
    }

    ui_draw_footer("X: Play | O: Back");
}

void ui_update_servers(void) {
    if (!g_app.links) return;
    if (g_app.pressed & SCE_CTRL_DOWN) { if (g_app.cursor_y < g_app.link_count - 1) g_app.cursor_y++; }
    if (g_app.pressed & SCE_CTRL_UP) { if (g_app.cursor_y > 0) g_app.cursor_y--; }
    if (g_app.pressed & SCE_CTRL_CROSS) {
        g_app.selected_link_idx = g_app.cursor_y;
        app_set_state(STATE_PLAYING);
    }
    if (g_app.pressed & SCE_CTRL_CIRCLE) {
        if (g_app.selected_item.box_type == CONTENT_MOVIE) app_set_state(STATE_DETAIL);
        else app_set_state(STATE_EPISODES);
    }
}
