#include "http_client.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

static CURL *g_curl = NULL;

static size_t write_cb(void *p, size_t s, size_t n, void *u) {
    size_t rs = s*n; HttpResponse *r = (HttpResponse*)u;
    char *ptr = realloc(r->data, r->size+rs+1);
    if (!ptr) return 0;
    r->data=ptr; memcpy(r->data+r->size, p, rs);
    r->size += rs; r->data[r->size]=0; return rs;
}

void http_client_init(void)    { curl_global_init(CURL_GLOBAL_ALL); g_curl=curl_easy_init(); }
void http_client_cleanup(void) { if(g_curl){curl_easy_cleanup(g_curl);g_curl=NULL;} curl_global_cleanup(); }

static void setup(CURL *c, const char *url, HttpResponse *r) {
    curl_easy_reset(c);
    curl_easy_setopt(c,CURLOPT_URL,url);
    curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,write_cb);
    curl_easy_setopt(c,CURLOPT_WRITEDATA,r);
    curl_easy_setopt(c,CURLOPT_TIMEOUT,30L);
    curl_easy_setopt(c,CURLOPT_CONNECTTIMEOUT,15L);
    curl_easy_setopt(c,CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(c,CURLOPT_SSL_VERIFYPEER,0L);
    curl_easy_setopt(c,CURLOPT_SSL_VERIFYHOST,0L);
    curl_easy_setopt(c,CURLOPT_USERAGENT,"Mozilla/5.0 (Linux; Android 14) AppleWebKit/537.36");
    struct curl_slist *h=NULL;
    h=curl_slist_append(h,"Accept: charset=utf-8");
    h=curl_slist_append(h,"Platform: android");
    curl_easy_setopt(c,CURLOPT_HTTPHEADER,h);
}

int http_get(const char *url, HttpResponse *r) {
    if(!g_curl||!url||!r) return -1;
    memset(r,0,sizeof(HttpResponse)); setup(g_curl,url,r);
    if(curl_easy_perform(g_curl)!=CURLE_OK) return -1;
    long code=0; curl_easy_getinfo(g_curl,CURLINFO_RESPONSE_CODE,&code);
    r->status_code=(int)code; return 0;
}

int http_post_form(const char *url, const char *pf, HttpResponse *r) {
    if(!g_curl||!url||!r) return -1;
    memset(r,0,sizeof(HttpResponse)); setup(g_curl,url,r);
    curl_easy_setopt(g_curl,CURLOPT_POST,1L);
    curl_easy_setopt(g_curl,CURLOPT_POSTFIELDS,pf);
    if(curl_easy_perform(g_curl)!=CURLE_OK) return -1;
    long code=0; curl_easy_getinfo(g_curl,CURLINFO_RESPONSE_CODE,&code);
    r->status_code=(int)code; return 0;
}

void http_response_free(HttpResponse *r) { if(r&&r->data){free(r->data);r->data=NULL;r->size=0;} }
