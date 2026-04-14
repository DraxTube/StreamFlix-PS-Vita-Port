#ifndef SS_CRYPTO_H
#define SS_CRYPTO_H
#include <stddef.h>
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);
unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length);
char *md5_hex(const char *input);
char *md5_bytes_hex(const unsigned char *input, size_t len);
char *desede_cbc_encrypt(const char *plaintext, const char *key, const char *iv_str);
char *ss_build_query(const char *json_params);
void ss_generate_token(char *out, size_t len);
long long ss_get_expiry(void);
#endif
