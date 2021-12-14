#pragma once

#include <sodium.h>

// Taken from : https://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MASTERPASSWORD_FILE "masterPassword"
#define FOLDER_NAME "data"
#define PW_SIZE 50

#define NB_ALLOWED_TRIES 3

// Hashing
#define hash_OPSLIMIT crypto_pwhash_OPSLIMIT_SENSITIVE
#define hash_MEMLIMIT crypto_pwhash_MEMLIMIT_SENSITIVE

// Key Derivation Function
#define KDF_OPSLIMIT crypto_pwhash_OPSLIMIT_SENSITIVE
#define KDF_MEMLIMIT crypto_pwhash_MEMLIMIT_SENSITIVE
#define KDF_ALG crypto_pwhash_ALG_DEFAULT

// Encryption stuff
#define KEY_LEN crypto_box_SEEDBYTES
#define SALT_LEN crypto_pwhash_SALTBYTES
#define NONCE_LEN crypto_aead_xchacha20poly1305_ietf_NPUBBYTES
#define ENC_PW_LEN PW_SIZE + crypto_aead_xchacha20poly1305_ietf_ABYTES

#define ADDITIONAL_DATA (const unsigned char *) NULL
#define ADDITIONAL_DATA_LEN 0

// TODO: remove debug annotations
#define DEBUG 0