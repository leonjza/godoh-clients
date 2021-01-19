/*
    This file is mostly from:
        https://zlib.net/zpipe.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#include "debug.h"
#include "utils.h"

char *zdepress(char *input, u_int ilen, u_int *olen)
{

    char *ostream = malloc(ilen);
    int ret = uncompress((Bytef *)ostream, (uLongf *)olen, (Bytef *)input, ilen + 1);

    switch (ret)
    {
    case Z_BUF_ERROR:
        Dprintf(" ! failed to decompress: Z_BUF_ERROR\n");
    case Z_DATA_ERROR:
        Dprintf(" ! failed to decompress: Z_DATA_ERROR\n");
    case Z_MEM_ERROR:
        Dprintf(" ! failed to decompress: Z_MEM_ERROR\n");
    }

    return ostream;
}

char *zpress(char *input, u_int ilen, u_int *olen)
{

    *olen = (u_int)compressBound(ilen);
    char *ostream = malloc(*olen);

    int res = compress2((Bytef *)ostream, (uLongf *)olen, (Bytef *)input, ilen + 1, Z_BEST_COMPRESSION);
    if (res == Z_BUF_ERROR)
    {
        printf("Buffer was too small!\n");
        return ostream;
    }
    if (res == Z_MEM_ERROR)
    {
        printf("Not enough memory for compression!\n");
        return ostream;
    }

    return ostream;
}
