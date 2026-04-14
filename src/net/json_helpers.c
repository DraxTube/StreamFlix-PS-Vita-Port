#include "json_helpers.h"
#include <string.h>
#include <stdlib.h>

const char *json_get_string(json_t *o, const char *k) {
    if(!o||!k) return "";
    json_t *v=json_object_get(o,k);
    return (v&&json_is_string(v)) ? json_string_value(v) : "";
}
int json_get_int(json_t *o, const char *k, int d) {
    if(!o||!k) return d;
    json_t *v=json_object_get(o,k);
    if(v&&json_is_integer(v)) return (int)json_integer_value(v);
    if(v&&json_is_string(v)){const char*s=json_string_value(v); if(s&&s[0])return atoi(s);}
    return d;
}
double json_get_double(json_t *o, const char *k, double d) {
    if(!o||!k) return d;
    json_t *v=json_object_get(o,k);
    if(v&&json_is_real(v)) return json_real_value(v);
    if(v&&json_is_integer(v)) return (double)json_integer_value(v);
    if(v&&json_is_string(v)){const char*s=json_string_value(v); if(s&&s[0])return atof(s);}
    return d;
}
json_t *json_get_array(json_t *o, const char *k) {
    if(!o||!k) return NULL;
    json_t *v=json_object_get(o,k);
    return (v&&json_is_array(v)) ? v : NULL;
}
json_t *json_get_object(json_t *o, const char *k) {
    if(!o||!k) return NULL;
    json_t *v=json_object_get(o,k);
    return (v&&json_is_object(v)) ? v : NULL;
}
