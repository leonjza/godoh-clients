#include <sys/types.h>

int rand_range(int min, int max);
void jittered_sleep();
int str_to_hex_str(char *in, char *out);
int hex_str_to_char(char *in, char *out);
void hex_dump(const char *desc, const void *addr, const int len);
void rand_str(char *dest, size_t length);
