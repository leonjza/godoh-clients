#ifndef GODOH_UTILS_H
#define GODOH_UTILS_H

#include <sys/types.h>

int rand_range(int min, int max);

void jitter_sleep();

char *bin_str_to_hex_str(const char *in, int in_len);

char *hex_str_to_bin_char(const char *in, int *res_len);

void hex_dump(const char *desc, const void *addr, int len);

char *rand_str(size_t length);

void hex_str_dump(const void *addr, int len);

#endif //GODOH_UTILS_H
