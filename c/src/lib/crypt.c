#include <string.h>
#include <sys/types.h>

#include "tinyaes/aes.h"
#include "tinyaes/pkcs7.h"
#include "options.h"
#include "utils.h"

#define KEY_LEN 16
#define IV_LEN 16

size_t decrypt(char *ciphertext, int cipherlen, char *output)
{
    // get key byte array
    char hex_key[KEY_LEN];
    hex_str_to_char(KEY, hex_key);

    // set the cipher byte array as uint8_t
    uint8_t cipher_array[cipherlen];
    memset(cipher_array, 0, cipherlen);
    for (int i = 0; i < cipherlen; i++)
    {
        cipher_array[i] = (uint8_t)ciphertext[i];
    }
    // hex_dump("cipher_array", cipher_array, cipherlen);

    // set the key byte array as uint8_t
    uint8_t key_array[KEY_LEN];
    memset(key_array, 0, KEY_LEN);
    for (int i = 0; i < KEY_LEN; i++)
    {
        key_array[i] = (uint8_t)hex_key[i];
    }
    // hex_dump("key_array", &key_array, KEY_LEN);

    // get the iv. its the first 16 bytes from the ciphertext
    uint8_t iv_array[IV_LEN];
    memset(iv_array, 0, IV_LEN);
    for (int i = 0; i < IV_LEN; i++)
    {
        iv_array[i] = (uint8_t)ciphertext[i];
    }
    // hex_dump("iv_array", &iv_array, IV_LEN);

    // decryption
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key_array, iv_array);
    AES_CBC_decrypt_buffer(&ctx, cipher_array, cipherlen);

    uint8_t *clear = cipher_array + IV_LEN;
    size_t dec_len = pkcs7_padding_data_length(clear, cipherlen - IV_LEN, AES_KEYLEN);
    // hex_dump("cipher_text_decrypted", clear, cipherlen - IV_LEN);

    // fill the output
    for (int i = 0; i < (int)dec_len; i++) {
        output[i] = (char)clear[i];
    }
    output[dec_len+1] = '\0';

    return dec_len;
}
