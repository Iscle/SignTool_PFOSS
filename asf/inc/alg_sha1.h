#ifndef _SHA1_H
#define _SHA1_H

#define SHA1_LEN 20

typedef struct {
    unsigned long to[2];
    unsigned long st[5];
    unsigned char buf[64];
} sha1_ctx;

/**************************************************************************
 *  EXPORT FUNCTIONS
 **************************************************************************/
void sha1(const unsigned char *input, int ilen, unsigned char output[20]);

#endif
