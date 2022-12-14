#include <string.h>
#include "sec_cust_struct.h"
#include "alg_sha1.h"

/**************************************************************************
 *  GLOBAL VARIABLE
 **************************************************************************/
unsigned char sha1sum[HASH_LEN];

/**************************************************************************
 *  INTERNAL DEFINIITION
 **************************************************************************/
#define MASK 0x3F
#define K1 0x5A827999
#define K2 0x6ED9EBA1
#define K3 0x8F1BBCDC
#define K4 0xCA62C1D6

static const unsigned char padding[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/**************************************************************************
 *  INTERNAL FUNCTIONS
 **************************************************************************/
void hash_starts(sha1_ctx *ctx);

void hash_update(sha1_ctx *ctx, const unsigned char *input, int ilen);

void hash_finish(sha1_ctx *ctx, unsigned char output[20]);

/**************************************************************************
 *  MACRO
 **************************************************************************/
unsigned long get_ul(const unsigned char *b, unsigned long i) {
    return ((unsigned long) (b)[(i)] << 24) | ((unsigned long) (b)[(i) + 1] << 16) |
           ((unsigned long) (b)[(i) + 2] << 8) | ((unsigned long) (b)[(i) + 3]);
}

void set_ul(unsigned long n, unsigned char *b, unsigned long i) {
    (b)[(i)] = (unsigned char) ((n) >> 24);
    (b)[(i) + 1] = (unsigned char) ((n) >> 16);
    (b)[(i) + 2] = (unsigned char) ((n) >> 8);
    (b)[(i) + 3] = (unsigned char) ((n));
}

unsigned long cal_S(unsigned long x, unsigned long n) {
    return ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)));
}

unsigned long cal_A2(unsigned long b) {
    return cal_S(b, 30);
}

unsigned long
cal_P(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long x, unsigned int i) {
    if ((20 > i) && (i >= 0)) {
        return (cal_S(a, 5) + ((d ^ (b & (c ^ d)))) + K1 + x);
    } else if ((40 > i) && (i >= 20)) {
        return (cal_S(a, 5) + ((b ^ c ^ d)) + K2 + x);
    } else if ((60 > i) && (i >= 40)) {
        return (cal_S(a, 5) + (((b & c) | (d & (b | c)))) + K3 + x);
    } else {
        return (cal_S(a, 5) + ((b ^ c ^ d)) + K4 + x);
    }
}

/**************************************************************************
 *  FUNCTIONS
 **************************************************************************/
void hash_starts(sha1_ctx *ctx) {
    ctx->to[0] = 0;
    ctx->to[1] = 0;

    ctx->st[0] = 0x67452301;
    ctx->st[1] = 0xEFCDAB89;
    ctx->st[2] = 0x98BADCFE;
    ctx->st[3] = 0x10325476;
    ctx->st[4] = 0xC3D2E1F0;
}

static void hash_process(sha1_ctx *ctx, const unsigned char data[64]) {

    unsigned int i = 0;
    unsigned long temp, V[16], A1, A2, A3, A4, A5;

    for (i = 0; i < 16; i++) {
        V[i] = get_ul(data, i * 4);
    }

#define R(t)                                                \
    (                                                       \
        temp = V[(t -  3) & 0x0F] ^ V[(t - 8) & 0x0F] ^     \
        V[(t - 14) & 0x0F] ^ V[t & 0x0F],                   \
        ( V[t & 0x0F] = cal_S(temp,1) )                     \
    )

    A1 = ctx->st[0];
    A2 = ctx->st[1];
    A3 = ctx->st[2];
    A4 = ctx->st[3];
    A5 = ctx->st[4];

    for (i = 0; i < 16; i++) {
        /* processing */
        switch (i % 5) {
            case 0:
                A5 += cal_P(A1, A2, A3, A4, V[i], i);
                A2 = cal_A2(A2);
                break;
            case 1:
                A4 += cal_P(A5, A1, A2, A3, V[i], i);
                A1 = cal_A2(A1);
                break;
            case 2:
                A3 += cal_P(A4, A5, A1, A2, V[i], i);
                A5 = cal_A2(A5);
                break;
            case 3:
                A2 += cal_P(A3, A4, A5, A1, V[i], i);
                A4 = cal_A2(A4);
                break;
            case 4:
                A1 += cal_P(A2, A3, A4, A5, V[i], i);
                A3 = cal_A2(A3);
                break;
        }
    }

    for (i = 16; i < 80; i++) {
        /* processing */
        switch (i % 5) {
            case 0:
                A5 += cal_P(A1, A2, A3, A4, R(i), i);
                A2 = cal_A2(A2);
                break;
            case 1:
                A4 += cal_P(A5, A1, A2, A3, R(i), i);
                A1 = cal_A2(A1);
                break;
            case 2:
                A3 += cal_P(A4, A5, A1, A2, R(i), i);
                A5 = cal_A2(A5);
                break;
            case 3:
                A2 += cal_P(A3, A4, A5, A1, R(i), i);
                A4 = cal_A2(A4);
                break;
            case 4:
                A1 += cal_P(A2, A3, A4, A5, R(i), i);
                A3 = cal_A2(A3);
                break;
        }
    }

    ctx->st[0] += A1;
    ctx->st[1] += A2;
    ctx->st[2] += A3;
    ctx->st[3] += A4;
    ctx->st[4] += A5;
}

void hash_update(sha1_ctx *ctx, const unsigned char *input, int ilen) {
    int fill;
    unsigned long le;

    if (ilen <= 0) {
        return;
    }

    le = ctx->to[0] & MASK;
    fill = 64 - le;

    ctx->to[0] += ilen;
    ctx->to[0] &= 0xFFFFFFFF;

    if (ctx->to[0] < (unsigned long) ilen) {
        ctx->to[1]++;
    }

    if (le && ilen >= fill) {
        memcpy((void *) (ctx->buf + le), (void *) input, fill);
        hash_process(ctx, ctx->buf);
        ilen -= fill;
        input += fill;
        le = 0;
    }

    while (ilen >= 64) {
        hash_process(ctx, input);
        ilen -= 64;
        input += 64;
    }

    if (ilen > 0) {
        memcpy((void *) (ctx->buf + le), (void *) input, ilen);
    }
}

void hash_finish(sha1_ctx *ctx, unsigned char output[20]) {
    unsigned int i = 0;
    unsigned long last, padn;
    unsigned long hi, lo;
    unsigned char msglen[8];

    hi = (ctx->to[0] >> 29) | (ctx->to[1] << 3);
    lo = (ctx->to[0] << 3);

    set_ul(hi, msglen, 0);
    set_ul(lo, msglen, 4);

    last = ctx->to[0] & MASK;

    if (last < 56) {
        padn = 56 - last;
    } else {
        padn = 120 - last;
    }

    hash_update(ctx, (unsigned char *) padding, padn);
    hash_update(ctx, msglen, 8);

    for (i = 0; i < 5; i++) {
        set_ul(ctx->st[i], output, i * 4);
    }
}

void sha1(const unsigned char *input, int ilen, unsigned char output[20]) {
    sha1_ctx ctx;

    /* initialize variable */
    hash_starts(&ctx);
    /* block processing */
    hash_update(&ctx, input, ilen);
    /* complete */
    hash_finish(&ctx, output);

    memset(&ctx, 0, sizeof(sha1_ctx));
}
