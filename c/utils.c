#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// https://stackoverflow.com/a/1941331
#ifdef DEBUG
#define Dprintf(fmt, args...) printf(fmt, ##args)
#else
#define Dprintf(fmt, args...)
#endif

int rand_range(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void jittered_sleep()
{
    Dprintf("getting random numner\n");
    int i = rand_range(1, MAX_SLEEP);
    Dprintf("sleeping for %ds\n", i);
    sleep(i);
}

void str2hexstr(char *in, char *out)
{
    int len = strlen(in);

    // https://stackoverflow.com/a/46210658
    for (int i = 0, j = 0; i < len; ++i, j += 2)
        sprintf(out + j, "%02x", in[i] & 0xff);
}

void rand_str(char *dest, size_t length)
{
    // https://stackoverflow.com/a/15768317
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0)
    {
        size_t index = (double)rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }

    *dest = '\0';
}
