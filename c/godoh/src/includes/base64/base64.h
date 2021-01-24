#ifndef GODOH_BASE64_H
#define GODOH_BASE64_H

char* base64( const void* binaryData, int len, int *flen );
unsigned char* unbase64( const char* ascii, int len, int *flen );

#endif //GODOH_BASE64_H
