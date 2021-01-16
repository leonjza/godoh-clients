/*
    This file mostly from:
        https://zlib.net/zpipe.c
*/

#include <string.h>
#include <assert.h>
#include <zlib.h>

#include "utils.h"

#define CHUNK 1024

int zdepress(char *input, u_int ilen, u_int *olen, char *output)
{

    int ret;
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = ilen,
    strm.next_in = (Bytef *)input;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* 
        We're not looping like in zpipe as the expectation is to have really small
        chunks of data to decompress here.
    */

    strm.avail_out = CHUNK;
    strm.next_out = (Bytef *) output;
    ret = inflate(&strm, Z_NO_FLUSH);

    switch (ret)
    {
    case Z_NEED_DICT:
        ret = Z_DATA_ERROR; /* and fall through */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return ret;
    }

    *olen = strm.total_out; // write # of decompressed bytes

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
