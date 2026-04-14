#ifndef DATABASE_H
#define DATABASE_H
/* Simple file-based favorites/history storage */
void db_init(void);
void db_close(void);
void db_add_history(int id, int box_type, const char *title);
int db_get_history_count(void);
#endif
