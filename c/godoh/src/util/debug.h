#ifndef GODOH_DEBUG_H
#define GODOH_DEBUG_H

#include <stdio.h>

// https://stackoverflow.com/a/27351464
#if defined(DEBUG) && DEBUG > 0
#define Dprintf(fmt, args...) fprintf(stderr, "DEBUG: ln=%d fn=%s(): " fmt, \
    __LINE__, __func__, ##args)
#else
#define Dprintf(fmt, args...) /* Don't do anything in release builds */
#endif

#endif //GODOH_DEBUG_H
