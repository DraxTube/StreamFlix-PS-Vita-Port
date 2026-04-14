/* Video Player using SceAvPlayer */
#include "ui_common.h"
#include <psp2/avplayer.h>
#include <psp2/audiodec.h>
#include <psp2/kernel/threadmgr.h>

static SceAvPlayerHandle player = 0;
static bool player_init = false;
static bool player_paused = false;
static char now_playing_title[256] = "";
static char now_playing_url[1024] = "";

static void *av_alloc(void *arg, uint32_t align, uint32_t size) {
    (void)arg;
    return memalign(align, size);
}

static void av_free(void *arg, void *ptr) {
    (void)arg;
    free(ptr);
}

static void *av_alloc_gpu(void *arg, uint32_t align, uint32_t size) {
    (void)arg;
    void *ptr = NULL;
    SceUID uid = sceKernelAllocMemBlock("avplayer_gpu",
        SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, (size + 0x3FFFF) & ~0x3FFFF, NULL);
    if (uid >= 0) sceKernelGetMemBlockBase(uid, &ptr);
    return ptr;
}

static void av_free_gpu(void *arg, void *ptr) {
    (void)arg;
    if (ptr) {
        SceUID uid = sceKernelFindMemBlockByAddr(ptr, 0);
        if (uid >= 0) sceKernelFreeMemBlock(uid);
    }
}

static int start_player(const char *url) {
    if (player_init) {
        sceAvPlayerStop(player);
        sceAvPlayerClose(player);
        player_init = false;
    }

    SceAvPlayerInitData init;
    memset(&init, 0, sizeof(SceAvPlayerInitData));
    init.memoryReplacement.allocate = av_alloc;
    init.memoryReplacement.deallocate = av_free;
    init.memoryReplacement.allocateTexture = av_alloc_gpu;
    init.memoryReplacement.deallocateTexture = av_free_gpu;
    init.basePriority = 0xA0;
    init.numOutputVideoFrameBuffers = 2;
    init.autoStart = SCE_TRUE;

    player = sceAvPlayerInit(&init);
    if (player < 0) return -1;

    int ret = sceAvPlayerAddSource(player, url);
    if (ret < 0) {
        sceAvPlayerClose(player);
        return -1;
    }

    player_init = true;
    player_paused = false;
    return 0;
}

static void stop_player(void) {
    if (player_init) {
        sceAvPlayerStop(player);
        sceAvPlayerClose(player);
        player_init = false;
    }
}

void ui_draw_player(void) {
    /* Black background */
    ui_draw_rect_filled(0, 0, SCREEN_W, SCREEN_H, RGBA8(0, 0, 0, 255));

    if (!player_init) {
        /* Starting player */
        VideoLink *lk = &g_app.links[g_app.selected_link_idx];
        strncpy(now_playing_url, lk->url, 1023);
        strncpy(now_playing_title, g_app.selected_item.title, 255);

        ui_draw_text_center(SCREEN_H / 2 - 20, "Starting player...", COL_TEXT_LIGHT, 1.0f);
        ui_draw_text_center(SCREEN_H / 2 + 10, now_playing_title, COL_TEXT_GRAY, 0.8f);
        ui_draw_text_center(SCREEN_H / 2 + 35, lk->quality, COL_PRIMARY, 0.75f);

        if (start_player(now_playing_url) != 0) {
            strcpy(g_app.error_msg, "Failed to start player.\nO: Back");
            app_set_state(STATE_ERROR);
            return;
        }
        return;
    }

    if (player_init) {
        /* Get video frame */
        SceAvPlayerFrameInfo frame;
        memset(&frame, 0, sizeof(SceAvPlayerFrameInfo));

        if (sceAvPlayerGetVideoData(player, &frame) == 0 && frame.pData) {
            /* Create texture from frame data */
            vita2d_texture *tex = vita2d_create_empty_texture_format(
                frame.details.video.width,
                frame.details.video.height,
                SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1);

            if (tex) {
                void *tex_data = vita2d_texture_get_datap(tex);
                memcpy(tex_data, frame.pData,
                    frame.details.video.width * frame.details.video.height * 3 / 2);

                /* Scale to screen */
                float sx = (float)SCREEN_W / frame.details.video.width;
                float sy = (float)SCREEN_H / frame.details.video.height;
                float scale = sx < sy ? sx : sy;
                int dx = (SCREEN_W - (int)(frame.details.video.width * scale)) / 2;
                int dy = (SCREEN_H - (int)(frame.details.video.height * scale)) / 2;

                vita2d_draw_texture_scale(tex, dx, dy, scale, scale);
                vita2d_free_texture(tex);
            }
        }

        /* Audio */
        SceAvPlayerFrameInfo audio_frame;
        memset(&audio_frame, 0, sizeof(SceAvPlayerFrameInfo));
        sceAvPlayerGetAudioData(player, &audio_frame);

        /* Check if playback ended */
        if (sceAvPlayerIsActive(player) == SCE_FALSE) {
            stop_player();
            app_set_state(STATE_SERVERS);
            return;
        }
    }

    /* HUD overlay (always visible for simplicity) */
    /* Top bar */
    ui_draw_rect_filled(0, 0, SCREEN_W, 40, RGBA8(0, 0, 0, 150));
    ui_draw_text(15, 28, now_playing_title, COL_TEXT_WHITE, 0.8f);

    /* Bottom controls hint */
    ui_draw_rect_filled(0, SCREEN_H - 30, SCREEN_W, 30, RGBA8(0, 0, 0, 150));
    const char *status = player_paused ? "PAUSED" : "PLAYING";
    ui_draw_text(15, SCREEN_H - 10, status, player_paused ? COL_GOLD : COL_GREEN, 0.7f);
    ui_draw_text_right(SCREEN_W - 15, SCREEN_H - 10, "X: Pause | O: Stop", COL_TEXT_GRAY, 0.65f);
}

void ui_update_player(void) {
    /* Pause/Resume */
    if (g_app.pressed & SCE_CTRL_CROSS) {
        if (player_init) {
            if (player_paused) {
                sceAvPlayerResume(player);
                player_paused = false;
            } else {
                sceAvPlayerPause(player);
                player_paused = true;
            }
        }
    }

    /* Stop and go back */
    if (g_app.pressed & SCE_CTRL_CIRCLE) {
        stop_player();
        app_set_state(STATE_SERVERS);
    }
}
