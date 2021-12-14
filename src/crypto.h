#pragma once

/*
 * Ask the user to enter its password, compute its hash and store it.
 * /!\ Returns the key in a sodium allocated memory zone
 */
char* createMasterPasswordFileAndKey();

/*
 * Ask the user to enter its password, compute the hash.
 * /!\ Returns the key in a sodium allocated memory zone
 */
char* askAndCheckMasterPassword();

/*
 * Change the master password. Will regenerate a new key.
 * This will re-encrypt the stored passwords with the new key
 * /!\ Returns the new key in sodium allocated memory zone
 */
char* changeMasterPassword(char* key);

/*
 * Allocate secure memory space
 */
char* sodiumAlloc(const size_t length, const size_t unitSize);

/*
 * Erase the memory allocated by libsodium and free the pointer with size PW_SIZE
 */
void sodiumFree(char* masterPassword);

/*
 * Erase the memory allocated by libsodium and free the pointer with size KEY_LEN
 */
void sodiumFreeKey(char* key);

/*
 * Encrypt a password with a key and return a string with the encrypted password in base64 and the nonce in base64
 */
char* encPassword(char* password_clear, char* key);

/*
 * Decrypt a password based on the encrypted password in base64 and the nonce in base64
 */
char* decPassword(char* passwordAndNonce, char* key);