#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H
#include <jansson.h>
const char *json_get_string(json_t *obj, const char *key);
int json_get_int(json_t *obj, const char *key, int def);
double json_get_double(json_t *obj, const char *key, double def);
json_t *json_get_array(json_t *obj, const char *key);
json_t *json_get_object(json_t *obj, const char *key);
#endif
