#ifndef _COMPRESS_H
#define _COMPRESS_H

#include "types.h"

int range(int size, int *src, int ws, char *dst);
int compress(u32 size, u8 *src, u8 mode, u8 *dst);
u32 decompress(u8 *src, u8 *dest);

#endif // _COMPRESS_H
