#ifndef SUPERSTREAM_H
#define SUPERSTREAM_H
#include "../main.h"
int ss_get_home(Category **categories, int *count);
int ss_search(const char *query, int page, ShowItem **results, int *count);
int ss_get_movie_detail(int movie_id, ShowItem *out);
int ss_get_tvshow_detail(int show_id, ShowItem *out, Season **seasons, int *season_count);
int ss_get_episodes(int show_id, int season_num, Episode **episodes, int *count);
int ss_get_movie_links(int movie_id, VideoLink **links, int *count);
int ss_get_episode_links(int show_id, int season_num, int episode_num, VideoLink **links, int *count);
#endif
