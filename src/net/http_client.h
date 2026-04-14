#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <stddef.h>
typedef struct { char *data; size_t size; int status_code; } HttpResponse;
void http_client_init(void);
void http_client_cleanup(void);
int http_get(const char *url, HttpResponse *response);
int http_post_form(const char *url, const char *post_fields, HttpResponse *response);
void http_response_free(HttpResponse *response);
#endif
