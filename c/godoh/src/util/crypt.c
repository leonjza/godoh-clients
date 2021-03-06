#include "crypt.h"

#include <stdlib.h>
#include <string.h>

#include "../includes/tinyaes/aes.h"
#include "../includes/tinyaes/pkcs7.h"

#include "../options.h"
#include "utils.h"

#define KEY_LEN 16
#define IV_LEN 16

char *encrypt(char *cleartext, int clearlen, int *e_len) {
    // get key byte array
    char *hex_key = hex_str_to_bin_char(KEY, NULL);

    // set the clear text byte array as uint8_t, resizing it
    // such that padding would fit in the array still
    int clearlen_pad = clearlen;
    if (clearlen_pad % 16)
        clearlen_pad += 16 - (clearlen_pad % 16);

    uint8_t clear_array[clearlen_pad];
    memset(clear_array, 0, clearlen_pad);
    for (int i = 0; i < clearlen; i++) {
        clear_array[i] = (uint8_t) cleartext[i];
    }

    // set the key byte array as uint8_t
    uint8_t key_array[KEY_LEN];
    memset(key_array, 0, KEY_LEN);
    for (int i = 0; i < KEY_LEN; i++) {
        key_array[i] = (uint8_t) hex_key[i];
    }

    // get the iv. its the first 16 bytes from the ciphertext
    uint8_t iv_array[IV_LEN];
    memset(iv_array, 0, IV_LEN);
    for (int i = 0; i < IV_LEN; i++) {
        iv_array[i] = rand();
    }

    // padding
    pkcs7_padding_pad_buffer(clear_array, clearlen, sizeof(clear_array), 16);

    // decryption
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key_array, iv_array);
    AES_CBC_encrypt_buffer(&ctx, clear_array, sizeof(clear_array));

    *e_len = sizeof(iv_array) + sizeof(clear_array);

    char *output = malloc(*e_len);
    memcpy(output, iv_array, sizeof(iv_array));
    memcpy(output + sizeof(iv_array), clear_array, sizeof(clear_array));

    return output;
}

size_t decrypt(char *ciphertext, int cipherlen, char *output) {

    // get key byte array
    char *hex_key = hex_str_to_bin_char(KEY, NULL);

    // set the cipher byte array as uint8_t
    uint8_t cipher_array[cipherlen];
    memset(cipher_array, 0, cipherlen);
    for (int i = 0; i < cipherlen; i++) {
        cipher_array[i] = (uint8_t) ciphertext[i];
    }

    // set the key byte array as uint8_t
    uint8_t key_array[KEY_LEN];
    memset(key_array, 0, KEY_LEN);
    for (int i = 0; i < KEY_LEN; i++) {
        key_array[i] = (uint8_t) hex_key[i];
    }

    // get the iv. its the first 16 bytes from the ciphertext
    uint8_t iv_array[IV_LEN];
    memset(iv_array, 0, IV_LEN);
    for (int i = 0; i < IV_LEN; i++) {
        iv_array[i] = (uint8_t) ciphertext[i];
    }

    // decryption
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key_array, iv_array);
    AES_CBC_decrypt_buffer(&ctx, cipher_array, cipherlen);

    uint8_t *clear = cipher_array + IV_LEN; // strip off the IV
    size_t dec_len = pkcs7_padding_data_length(clear, cipherlen - IV_LEN, AES_KEYLEN);

    // fill the output
    for (int i = 0; i < (int) dec_len; i++) {
        output[i] = (char) clear[i];
    }
    output[dec_len + 1] = '\0';

    return dec_len;
}
