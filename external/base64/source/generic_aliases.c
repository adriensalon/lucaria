#include "codecs.h"

extern struct codec* base64_stream_encode_plain(void);
extern struct codec* base64_stream_decode_plain(void);

struct codec* base64_stream_encode_avx512(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_avx512(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_avx2(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_avx2(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_avx(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_avx(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_ssse3(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_ssse3(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_sse41(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_sse41(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_sse42(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_sse42(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_neon32(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_neon32(void) { return base64_stream_decode_plain(); }

struct codec* base64_stream_encode_neon64(void) { return base64_stream_encode_plain(); }
struct codec* base64_stream_decode_neon64(void) { return base64_stream_decode_plain(); }