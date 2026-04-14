/*
 * SuperStream Provider - Ported from Android Kotlin to C
 */
#include "superstream.h"
#include "../net/http_client.h"
#include "../net/json_helpers.h"
#include "../net/ss_crypto.h"
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *API_URL = "https://showbox.shegu.net/api/api_client/index/";
static const char *API_URL2 = "https://mbpapi.shegu.net/api/api_client/index/";
static const char *APPID1 = "com.tdo.showbox";
static const char *APPID2 = "com.movieboxpro.android";
static const char *APPVER = "14.7";
static const char *APPVER_OLD = "11.5";

static char *build_params(const char *pairs[][2], int n) {
    json_t *o = json_object();
    for (int i = 0; i < n; i++) json_object_set_new(o, pairs[i][0], json_string(pairs[i][1]));
    char *r = json_dumps(o, JSON_COMPACT); json_decref(o); return r;
}

static json_t *api_call(const char *url, const char *jp) {
    char *pd = ss_build_query(jp); if (!pd) return NULL;
    HttpResponse resp; int ret = http_post_form(url, pd, &resp); free(pd);
    if (ret != 0 || !resp.data) { http_response_free(&resp); return NULL; }
    json_error_t err; json_t *root = json_loads(resp.data, 0, &err);
    http_response_free(&resp);
    if (!root) return NULL;
    if (json_get_int(root, "code", -1) != 200) { json_decref(root); return NULL; }
    return root;
}

static void parse_item(json_t *o, ShowItem *it) {
    if (!o || !it) return; memset(it, 0, sizeof(ShowItem));
    it->id = json_get_int(o, "id", 0);
    it->box_type = json_get_int(o, "box_type", 1);
    strncpy(it->title, json_get_string(o, "title"), 255);
    strncpy(it->overview, json_get_string(o, "description"), 1023);
    const char *p = json_get_string(o, "poster");
    if (!p[0]) p = json_get_string(o, "poster_org");
    strncpy(it->poster, p, 511);
    strncpy(it->banner, json_get_string(o, "banner_mini"), 511);
    strncpy(it->quality, json_get_string(o, "quality_tag"), 31);
    int yr = json_get_int(o, "year", 0);
    if (yr > 0) snprintf(it->year, 15, "%d", yr);
    strncpy(it->rating, json_get_string(o, "imdb_rating"), 15);
    it->runtime = json_get_int(o, "runtime", 0);
    strncpy(it->genres, json_get_string(o, "cats"), 255);
    strncpy(it->cast, json_get_string(o, "actors"), 511);
    strncpy(it->director, json_get_string(o, "director"), 255);
}

int ss_get_home(Category **cats, int *cnt) {
    if (!cats || !cnt) return -1;
    char exp[32]; snprintf(exp, 32, "%lld", ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"app_version",APPVER},{"appid",APPID2},
        {"module","Home_list_type_v2"},{"channel","Website"},{"page","0"},{"lang","en"},
        {"type","all"},{"pagelimit","10"},{"expired_date",exp},{"platform","android"}};
    char *jp = build_params(p, 11);
    json_t *root = api_call(API_URL2, jp); free(jp); if (!root) return -1;
    json_t *data = json_object_get(root, "data");
    if (!data || !json_is_array(data)) { json_decref(root); return -1; }
    int nc = (int)json_array_size(data); if (nc <= 0) { json_decref(root); return -1; }
    if (nc > 10) nc = 10;
    *cats = (Category *)calloc(nc, sizeof(Category)); if (!*cats) { json_decref(root); return -1; }
    int vc = 0;
    for (int i = 0; i < nc; i++) {
        json_t *co = json_array_get(data, i); if (!co) continue;
        const char *nm = json_get_string(co, "name"); if (!nm[0]) nm = "Featured";
        json_t *lst = json_get_array(co, "list"); if (!lst) continue;
        int ls = (int)json_array_size(lst); if (ls <= 0) continue; if (ls > 20) ls = 20;
        Category *c = &(*cats)[vc]; strncpy(c->name, nm, 127);
        c->items = (ShowItem *)calloc(ls, sizeof(ShowItem)); if (!c->items) continue;
        int vi = 0;
        for (int j = 0; j < ls; j++) {
            json_t *io = json_array_get(lst, j); if (!io) continue;
            int bt = json_get_int(io, "box_type", 0); if (bt != 1 && bt != 2) continue;
            parse_item(io, &c->items[vi]); vi++;
        }
        c->count = vi; if (vi > 0) vc++;
    }
    *cnt = vc; json_decref(root); return 0;
}

int ss_search(const char *q, int pg, ShowItem **res, int *cnt) {
    if (!q || !res || !cnt) return -1;
    char ps[16], exp[32]; snprintf(ps, 16, "%d", pg); snprintf(exp, 32, "%lld", ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"app_version",APPVER_OLD},{"appid",APPID1},
        {"module","Search3"},{"channel","Website"},{"page",ps},{"lang","en"},{"type","all"},
        {"keyword",q},{"pagelimit","20"},{"expired_date",exp},{"platform","android"}};
    char *jp = build_params(p, 12);
    json_t *root = api_call(API_URL, jp); free(jp); if (!root) return -1;
    json_t *data = json_object_get(root, "data");
    if (!data || !json_is_array(data)) { json_decref(root); return -1; }
    int n = (int)json_array_size(data); if (n <= 0) { *res = NULL; *cnt = 0; json_decref(root); return 0; }
    if (n > 30) n = 30;
    *res = (ShowItem *)calloc(n, sizeof(ShowItem)); if (!*res) { json_decref(root); return -1; }
    int v = 0;
    for (int i = 0; i < n; i++) {
        json_t *io = json_array_get(data, i); if (!io) continue;
        int bt = json_get_int(io, "box_type", 0); if (bt != 1 && bt != 2) continue;
        parse_item(io, &(*res)[v]); v++;
    }
    *cnt = v; json_decref(root); return 0;
}

int ss_get_movie_detail(int mid, ShowItem *out) {
    if (!out) return -1;
    char ms[32], exp[32]; snprintf(ms, 32, "%d", mid); snprintf(exp, 32, "%lld", ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"uid",""},{"app_version",APPVER_OLD},{"appid",APPID1},
        {"module","Movie_detail"},{"channel","Website"},{"mid",ms},{"lang","en"},
        {"expired_date",exp},{"platform","android"},{"oss",""},{"group",""}};
    char *jp = build_params(p, 12);
    json_t *root = api_call(API_URL, jp); free(jp); if (!root) return -1;
    json_t *data = json_object_get(root, "data");
    if (!data || !json_is_object(data)) { json_decref(root); return -1; }
    parse_item(data, out);
    const char *rel = json_get_string(data, "released");
    if (rel[0] && !out->year[0]) strncpy(out->year, rel, 4);
    json_decref(root); return 0;
}

int ss_get_tvshow_detail(int sid, ShowItem *out, Season **seasons, int *scnt) {
    if (!out || !seasons || !scnt) return -1;
    char ts[32], exp[32]; snprintf(ts, 32, "%d", sid); snprintf(exp, 32, "%lld", ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"uid",""},{"app_version",APPVER_OLD},{"appid",APPID1},
        {"module","TV_detail_1"},{"display_all","1"},{"channel","Website"},{"lang","en"},
        {"expired_date",exp},{"platform","android"},{"tid",ts}};
    char *jp = build_params(p, 11);
    json_t *root = api_call(API_URL, jp); free(jp); if (!root) return -1;
    json_t *data = json_object_get(root, "data");
    if (!data || !json_is_object(data)) { json_decref(root); return -1; }
    parse_item(data, out); out->box_type = CONTENT_TVSHOW;
    json_t *sa = json_get_array(data, "season");
    if (sa) {
        int n = (int)json_array_size(sa);
        if (n > 0) { *seasons = (Season *)calloc(n, sizeof(Season));
            if (*seasons) { for (int i = 0; i < n; i++) {
                json_t *s = json_array_get(sa, i);
                if (json_is_integer(s)) (*seasons)[i].number = (int)json_integer_value(s);
                snprintf((*seasons)[i].id, 63, "%d-%d", sid, (*seasons)[i].number);
            } *scnt = n; }
        }
    }
    json_decref(root); return 0;
}

int ss_get_episodes(int sid, int sn, Episode **eps, int *cnt) {
    if (!eps || !cnt) return -1;
    char ts[32], ss[16], exp[32];
    snprintf(ts, 32, "%d", sid); snprintf(ss, 16, "%d", sn); snprintf(exp, 32, "%lld", ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"app_version",APPVER},{"year","0"},{"appid",APPID2},
        {"module","TV_episode"},{"display_all","1"},{"channel","Website"},{"season",ss},
        {"lang","en"},{"expired_date",exp},{"platform","android"},{"tid",ts}};
    char *jp = build_params(p, 12);
    json_t *root = api_call(API_URL2, jp); free(jp); if (!root) return -1;
    json_t *data = json_object_get(root, "data");
    if (!data || !json_is_array(data)) { json_decref(root); return -1; }
    int n = (int)json_array_size(data);
    if (n <= 0) { *eps = NULL; *cnt = 0; json_decref(root); return 0; }
    *eps = (Episode *)calloc(n, sizeof(Episode)); if (!*eps) { json_decref(root); return -1; }
    for (int i = 0; i < n; i++) {
        json_t *e = json_array_get(data, i); if (!e) continue;
        (*eps)[i].id = json_get_int(e, "id", 0);
        (*eps)[i].number = json_get_int(e, "episode", i + 1);
        strncpy((*eps)[i].title, json_get_string(e, "title"), 255);
        strncpy((*eps)[i].released, json_get_string(e, "released"), 63);
        strncpy((*eps)[i].poster, json_get_string(e, "thumbs"), 511);
    }
    *cnt = n; json_decref(root); return 0;
}

static int parse_links(json_t *root, VideoLink **lnk, int *cnt) {
    json_t *data = json_object_get(root, "data");
    if (!data || !json_is_object(data)) return -1;
    json_t *list = json_get_array(data, "list"); if (!list) return -1;
    int n = (int)json_array_size(list);
    if (n <= 0) { *lnk = NULL; *cnt = 0; return 0; }
    *lnk = (VideoLink *)calloc(n, sizeof(VideoLink)); if (!*lnk) return -1;
    int v = 0;
    for (int i = 0; i < n; i++) {
        json_t *l = json_array_get(list, i); if (!l) continue;
        const char *path = json_get_string(l, "path"); if (!path[0]) continue;
        strncpy((*lnk)[v].url, path, 1023);
        strncpy((*lnk)[v].quality, json_get_string(l, "quality"), 31);
        strncpy((*lnk)[v].size, json_get_string(l, "size"), 63);
        (*lnk)[v].fid = json_get_int(l, "fid", 0); v++;
    }
    *cnt = v; return 0;
}

int ss_get_movie_links(int mid, VideoLink **lnk, int *cnt) {
    if (!lnk || !cnt) return -1;
    char ms[32], exp[32]; snprintf(ms, 32, "%d", mid); snprintf(exp, 32, "%lld", ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"uid",""},{"app_version",APPVER_OLD},{"appid",APPID1},
        {"module","Movie_downloadurl_v3"},{"channel","Website"},{"mid",ms},{"lang",""},
        {"expired_date",exp},{"platform","android"},{"oss","1"},{"group",""}};
    char *jp = build_params(p, 12);
    json_t *root = api_call(API_URL, jp); free(jp); if (!root) return -1;
    int r = parse_links(root, lnk, cnt); json_decref(root); return r;
}

int ss_get_episode_links(int sid, int sn, int en, VideoLink **lnk, int *cnt) {
    if (!lnk || !cnt) return -1;
    char ts[32], ss[16], es[16], exp[32];
    snprintf(ts,32,"%d",sid); snprintf(ss,16,"%d",sn);
    snprintf(es,16,"%d",en); snprintf(exp,32,"%lld",ss_get_expiry());
    const char *p[][2] = {{"childmode","1"},{"app_version",APPVER_OLD},{"module","TV_downloadurl_v3"},
        {"channel","Website"},{"episode",es},{"expired_date",exp},{"platform","android"},
        {"tid",ts},{"oss","1"},{"uid",""},{"appid",APPID1},{"season",ss},{"lang","en"},{"group",""}};
    char *jp = build_params(p, 14);
    json_t *root = api_call(API_URL, jp); free(jp); if (!root) return -1;
    int r = parse_links(root, lnk, cnt); json_decref(root); return r;
}
