#ifndef GODOH_CRYPT_H
#define GODOH_CRYPT_H

#include <sys/types.h>

char *encrypt(char *cleartext, int clearlen, int *e_len);
size_t decrypt(char *ciphertext, int cipherlen, char *output);

#endif //GODOH_CRYPT_H
