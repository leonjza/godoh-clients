#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../options.h"
#include <unistd.h>

int rand_range(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

void jitter_sleep() {
    int i = rand_range(1, MAX_SLEEP);
    sleep(i);
}

#ifdef DEBUG

void hex_dump(const char *desc, const void *addr, const int len) {
    // https://stackoverflow.com/a/7776146

    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *) addr;

    if (desc != NULL)
        printf("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    } else if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
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
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}

void hex_str_dump(const void *addr, const int len) {

    const unsigned char *pc = (const unsigned char *) addr;

    printf("hex str | ");
    for (int i = 0; i < len; i++) {
        printf("%02x", pc[i]);
    }
    printf("\n");
}


#endif

char *bin_str_to_hex_str(const char *in, int in_len) {
    char *out = calloc(in_len, sizeof(char *));

    for (int i = 0, j = 0; i < in_len; ++i, j += 2)
        sprintf(out + j, "%02x", in[i] & 0xff);

    return out;
}

char *hex_str_to_bin_char(const char *in, int *res_len) {

    int in_len = strlen(in);
    int out_len = in_len / 2;

    char *out = calloc(out_len, sizeof(char *));

    for (int i = 0, j = 0; j < in_len; ++i, j += 2) {
        int val[1];
        sscanf(in + j, "%2x", val);
        out[i] = val[0];
    }

    if (res_len != NULL) {
        *res_len = out_len;
    }

    return out;
}


char *rand_str(size_t length) {

    char *out = calloc(length + 1, sizeof(char *));

    // https://stackoverflow.com/a/15768317
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        out[length] = charset[index];
    }

    return out;
}

