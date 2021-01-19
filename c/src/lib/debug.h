#include <stdio.h>

// https://stackoverflow.com/a/1941331
#ifdef DEBUG
#define Dprintf(fmt, args...) printf(fmt, ##args)
#else
#define Dprintf(fmt, args...)
#endif
