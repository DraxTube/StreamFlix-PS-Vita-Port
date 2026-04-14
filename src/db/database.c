/* Simple file-based history storage (no SQLite dependency for simplicity) */
#include "database.h"
#include "../main.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdio.h>
#include <string.h>

#define HISTORY_FILE APP_DATA_PATH "history.dat"
#define MAX_HISTORY 50

typedef struct { int id; int box_type; char title[128]; } HistoryEntry;

static HistoryEntry history[MAX_HISTORY];
static int history_count = 0;

void db_init(void) {
    sceIoMkdir(APP_DATA_PATH, 0777);
    SceUID fd = sceIoOpen(HISTORY_FILE, SCE_O_RDONLY, 0);
    if (fd >= 0) {
        sceIoRead(fd, &history_count, sizeof(int));
        if (history_count > MAX_HISTORY) history_count = MAX_HISTORY;
        sceIoRead(fd, history, sizeof(HistoryEntry) * history_count);
        sceIoClose(fd);
    }
}

static void db_save(void) {
    SceUID fd = sceIoOpen(HISTORY_FILE, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, &history_count, sizeof(int));
        sceIoWrite(fd, history, sizeof(HistoryEntry) * history_count);
        sceIoClose(fd);
    }
}

void db_close(void) { db_save(); }

void db_add_history(int id, int box_type, const char *title) {
    /* Check if already exists */
    for (int i = 0; i < history_count; i++) {
        if (history[i].id == id && history[i].box_type == box_type) {
            /* Move to front */
            HistoryEntry tmp = history[i];
            memmove(&history[1], &history[0], sizeof(HistoryEntry) * i);
            history[0] = tmp;
            db_save(); return;
        }
    }
    /* Add new */
    if (history_count >= MAX_HISTORY) history_count = MAX_HISTORY - 1;
    memmove(&history[1], &history[0], sizeof(HistoryEntry) * history_count);
    history[0].id = id; history[0].box_type = box_type;
    strncpy(history[0].title, title, 127); history[0].title[127] = 0;
    history_count++; db_save();
}

int db_get_history_count(void) { return history_count; }
