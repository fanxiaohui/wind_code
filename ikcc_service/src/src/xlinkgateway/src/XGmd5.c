/*
 * md5.c
 *
 *  Created on: 2015年1月9日
 *      Author: john
 */

#include <string.h>
#include <stdio.h>
#include "XGmd5.h"

static void XlinkMD5Init(XGMD5_CTX *ctx);
static void XlinkMD5Update(XGMD5_CTX *ctx, unsigned char *buf, unsigned int len);
static void XlinkMD5Final(unsigned char *digest, XGMD5_CTX *ctx);
static void XlinkMD5Transform(unsigned int *buf, unsigned int * in);

#ifdef REVERSED
//#ifdef sgiXXX
#define HIGHFIRST
#endif

#ifdef sunXXX
#define HIGHFIRST
#endif

#ifndef HIGHFIRST
#define XLINKbyteReverse(buf, len)    /* Nothing */
#else
/*
 * Note: this code is harmless on little-endian machines.
 */
void XLINKbyteReverse(buf, longs)
unsigned char *buf; unsigned longs;
{
	unsigned int t;
	do {
		t = ( unsigned int ) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
		((unsigned) buf[1] << 8 | buf[0]);
		*( unsigned int *) buf = t;
		buf += 4;
	}while (--longs);
}
#endif

/*
 * Start MD5 accumulation. Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void XlinkMD5Init(XGMD5_CTX *ctx) {
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;

	ctx->bits[0] = 0;
	ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void XlinkMD5Update(XGMD5_CTX *ctx, unsigned char *buf, unsigned int len) {
	unsigned int t;

	/* Update bitcount */

	t = ctx->bits[0];
	if ((ctx->bits[0] = t + ((unsigned int) len << 3)) < t)
		ctx->bits[1]++; /* Carry from low to high */
	ctx->bits[1] += len >> 29;

	t = (t >> 3) & 0x3f; /* Bytes already in shsInfo->data */

	/* Handle any leading odd-sized chunks */

	if (t) {
		unsigned char *p = (unsigned char *) ctx->in + t;

		t = 64 - t;
		if (len < t) {
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, t);
		XLINKbyteReverse(ctx->in, 16);
		XlinkMD5Transform(ctx->buf, (unsigned int *) ctx->in);
		buf += t;
		len -= t;
	}
	/* Process data in 64-byte chunks */

	while (len >= 64) {
		memcpy(ctx->in, buf, 64);
		XLINKbyteReverse(ctx->in, 16);
		XlinkMD5Transform(ctx->buf, (unsigned int *) ctx->in);
		buf += 64;
		len -= 64;
	}

	/* Handle any remaining bytes of data. */

	memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void XlinkMD5Final(unsigned char *digest, XGMD5_CTX *ctx) {
	unsigned count;
	unsigned char *p;

	/* Compute number of bytes mod 64 */
	count = (ctx->bits[0] >> 3) & 0x3F;

	/* Set the first char of padding to 0x80. This is safe since there is
	 always at least one byte free */
	p = ctx->in + count;
	*p++ = 0x80;

	/* Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/* Pad out to 56 mod 64 */
	if (count < 8) {
		/* Two lots of padding: Pad the first block to 64 bytes */
		memset(p, 0, count);
		XLINKbyteReverse(ctx->in, 16);
		XlinkMD5Transform(ctx->buf, (unsigned int *) ctx->in);

		/* Now fill the next block with 56 bytes */
		memset(ctx->in, 0, 56);
	} else {
		/* Pad block to 56 bytes */
		memset(p, 0, count - 8);
	}XLINKbyteReverse(ctx->in, 14);

	/* Append length in bits and transform */
	((unsigned int *) ctx->in)[14] = ctx->bits[0];
	((unsigned int *) ctx->in)[15] = ctx->bits[1];

	XlinkMD5Transform(ctx->buf, (unsigned int *) ctx->in);
	XLINKbyteReverse((unsigned char *) ctx->buf, 4);
	memcpy(digest, ctx->buf, 16);
	memset(ctx, 0, sizeof(XGMD5_CTX)); /* In case it's sensitive */
}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define XLINK_F1(x, y, z) (z ^ (x & (y ^ z)))
#define XLINK_F2(x, y, z) XLINK_F1(z, x, y)
#define XLINK_F3(x, y, z) (x ^ y ^ z)
#define XLINK_F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define XLINK_MD5STEP(f, w, x, y, z, data, s) \
    ( w += f(x, y, z) + data, w = w<<s | w>>(32-s), w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data. MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void XlinkMD5Transform(unsigned int *buf, unsigned int * in) {
	register unsigned int a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	XLINK_MD5STEP(XLINK_F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	XLINK_MD5STEP(XLINK_F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	XLINK_MD5STEP(XLINK_F1, c, d, a, b, in[2] + 0x242070db, 17);
	XLINK_MD5STEP(XLINK_F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	XLINK_MD5STEP(XLINK_F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	XLINK_MD5STEP(XLINK_F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	XLINK_MD5STEP(XLINK_F1, c, d, a, b, in[6] + 0xa8304613, 17);
	XLINK_MD5STEP(XLINK_F1, b, c, d, a, in[7] + 0xfd469501, 22);
	XLINK_MD5STEP(XLINK_F1, a, b, c, d, in[8] + 0x698098d8, 7);
	XLINK_MD5STEP(XLINK_F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	XLINK_MD5STEP(XLINK_F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	XLINK_MD5STEP(XLINK_F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	XLINK_MD5STEP(XLINK_F1, a, b, c, d, in[12] + 0x6b901122, 7);
	XLINK_MD5STEP(XLINK_F1, d, a, b, c, in[13] + 0xfd987193, 12);
	XLINK_MD5STEP(XLINK_F1, c, d, a, b, in[14] + 0xa679438e, 17);
	XLINK_MD5STEP(XLINK_F1, b, c, d, a, in[15] + 0x49b40821, 22);

	XLINK_MD5STEP(XLINK_F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	XLINK_MD5STEP(XLINK_F2, d, a, b, c, in[6] + 0xc040b340, 9);
	XLINK_MD5STEP(XLINK_F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	XLINK_MD5STEP(XLINK_F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	XLINK_MD5STEP(XLINK_F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	XLINK_MD5STEP(XLINK_F2, d, a, b, c, in[10] + 0x02441453, 9);
	XLINK_MD5STEP(XLINK_F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	XLINK_MD5STEP(XLINK_F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	XLINK_MD5STEP(XLINK_F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	XLINK_MD5STEP(XLINK_F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	XLINK_MD5STEP(XLINK_F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	XLINK_MD5STEP(XLINK_F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	XLINK_MD5STEP(XLINK_F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	XLINK_MD5STEP(XLINK_F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	XLINK_MD5STEP(XLINK_F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	XLINK_MD5STEP(XLINK_F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	XLINK_MD5STEP(XLINK_F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	XLINK_MD5STEP(XLINK_F3, d, a, b, c, in[8] + 0x8771f681, 11);
	XLINK_MD5STEP(XLINK_F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	XLINK_MD5STEP(XLINK_F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	XLINK_MD5STEP(XLINK_F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	XLINK_MD5STEP(XLINK_F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	XLINK_MD5STEP(XLINK_F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	XLINK_MD5STEP(XLINK_F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	XLINK_MD5STEP(XLINK_F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	XLINK_MD5STEP(XLINK_F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	XLINK_MD5STEP(XLINK_F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	XLINK_MD5STEP(XLINK_F3, b, c, d, a, in[6] + 0x04881d05, 23);
	XLINK_MD5STEP(XLINK_F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	XLINK_MD5STEP(XLINK_F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	XLINK_MD5STEP(XLINK_F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	XLINK_MD5STEP(XLINK_F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	XLINK_MD5STEP(XLINK_F4, a, b, c, d, in[0] + 0xf4292244, 6);
	XLINK_MD5STEP(XLINK_F4, d, a, b, c, in[7] + 0x432aff97, 10);
	XLINK_MD5STEP(XLINK_F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	XLINK_MD5STEP(XLINK_F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	XLINK_MD5STEP(XLINK_F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	XLINK_MD5STEP(XLINK_F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	XLINK_MD5STEP(XLINK_F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	XLINK_MD5STEP(XLINK_F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	XLINK_MD5STEP(XLINK_F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	XLINK_MD5STEP(XLINK_F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	XLINK_MD5STEP(XLINK_F4, c, d, a, b, in[6] + 0xa3014314, 15);
	XLINK_MD5STEP(XLINK_F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	XLINK_MD5STEP(XLINK_F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	XLINK_MD5STEP(XLINK_F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	XLINK_MD5STEP(XLINK_F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	XLINK_MD5STEP(XLINK_F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

void xlinkGetMd5(unsigned char *RetBuffer, unsigned char *data, unsigned int datalen) {
	struct XGMD5Context md5c;
	if (RetBuffer == NULL || data == NULL || datalen == 0) {
		return;
	}
	XlinkMD5Init(&md5c);
	XlinkMD5Update(&md5c, data, datalen);
	XlinkMD5Final(RetBuffer, &md5c);
}
void xlinkHttpMd5Init(XGMD5_CTX *ctx) {
	XlinkMD5Init(ctx);
}
void XlinkHttpMd5Update(XGMD5_CTX *ctx, unsigned char *data, unsigned int datalen) {
	XlinkMD5Update(ctx, data, datalen);
}

void XlinkHttpM5dFinal(XGMD5_CTX *ctx, unsigned char *RetBuffer) {
	XlinkMD5Final(RetBuffer, ctx);
}

void XlinkHttpM5dFinalString(XGMD5_CTX *ctx, char *RetBuffer) {
	unsigned char temp[16];
	int i = 0, index = 0;
	XlinkMD5Final(temp, ctx);
	for (i = 0; i < 16; i++) {
		index += sprintf(RetBuffer + index, "%02X", temp[i]);
	}
}

