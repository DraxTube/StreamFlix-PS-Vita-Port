/* Stub implementations for zstd functions required by libcurl.
 * We don't actually need zstd decompression for SuperStream API responses,
 * but libcurl was compiled with zstd support so it needs these symbols. */

#include <stddef.h>

typedef size_t ZSTD_DStream;

ZSTD_DStream* ZSTD_createDStream(void) { return NULL; }
size_t ZSTD_freeDStream(ZSTD_DStream* zds) { (void)zds; return 0; }
size_t ZSTD_decompressStream(ZSTD_DStream* zds, void* output, void* input) {
    (void)zds; (void)output; (void)input; return 0;
}
unsigned ZSTD_isError(size_t code) { (void)code; return 0; }
const char* ZSTD_getErrorName(size_t code) { (void)code; return "unsupported"; }
size_t ZSTD_initDStream(ZSTD_DStream* zds) { (void)zds; return 0; }
