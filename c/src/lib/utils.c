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
    int i = rand_range(1, MAX_SLEEP);
    Dprintf("[d] sleeping for %ds\n", i);
    sleep(i);
}

int str_to_hex_str(char *in, char *out)
{
    int w = 0;

    // https://stackoverflow.com/a/46210658
    int len = strlen(in);

    for (int i = 0, j = 0; i < len; ++i, j += 2)
    {
        sprintf(out + j, "%02x", in[i] & 0xff);
        w++;
    }

    return w;
}

int hex_str_to_char(char *in, char *out)
{
    int w = 0;
    // https://stackoverflow.com/a/46210658
    int len = strlen(in);

    for (int i = 0, j = 0; j < len; ++i, j += 2)
    {
        int val[1];
        sscanf(in + j, "%2x", val);
        out[i] = val[0];
        out[i + 1] = '\0';

        w++;
    }

    return w;
}

void hex_dump(const char *desc, const void *addr, const int len)
{
    // https://stackoverflow.com/a/7776146

    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *)addr;

    if (desc != NULL)
        printf("%s:\n", desc);

    if (len == 0)
    {
        printf("  ZERO LENGTH\n");
        return;
    }
    else if (len < 0)
    {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        if ((i % 16) == 0)
        {
            if (i != 0)
                printf("  %s\n", buff);

            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And buffer a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
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
