#include <sys/types.h>

char* base64( const void* binaryData, int len, int *flen );
unsigned char* unbase64( const char* ascii, int len, int *flen );
