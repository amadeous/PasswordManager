#include <stdio.h>
#include <sodium.h>
#include <string.h>

#include "crypto.h"
#include "files.h"
#include "base64.h"
#include "utils.h"
#include "const.h"   // This file contains the constants of the app

char* sodiumAlloc(const size_t length, const size_t unitSize);

// The first two methods are inspired by the correction of the Exercise sheet 6

char* createMasterPasswordFileAndKey() {
  // Take some secure space for both password and the key and some regular space for the hashed password and the salt
  char* masterPassword = sodiumAlloc(PW_SIZE, sizeof(char));
  char* masterPassword2 = sodiumAlloc(PW_SIZE, sizeof(char));
  char hashed_password[crypto_pwhash_STRBYTES];
  
  char* key = sodiumAlloc(KEY_LEN, sizeof(char));
  char salt[SALT_LEN];

  // Ask the first password
  printf("WARNING ! MAKE SURE TO REMEMBER THIS PASSWORD OTHERWISE YOU WOULD NOT BE ABLE TO ACCESS YOUR OTHER PASSWORDS ANYMORE !\n");
  printf("Please choose your password (max " TOSTRING(PW_SIZE) " chars): ");
  fgets(masterPassword, PW_SIZE, stdin);
  // We remove the last \n of the password (from the user hitted ENTER) if not already cropped by fgets
  char* lastCharMasterPassword = &masterPassword[strlen(masterPassword)-1];
  if (*lastCharMasterPassword == '\n') {
    *lastCharMasterPassword = '\0';
  }
  // If not, it is still in the buffer and will cause trouble later..
  else {
    emptyBuffer();
  }

  // Ask the second time the same password
  printf("Please repeat your password: ");
  fgets(masterPassword2, PW_SIZE, stdin);
  // We remove the last \n of the password (from the user hitted ENTER) if not already cropped by fgets
  char* lastCharMasterPassword2 = &masterPassword2[strlen(masterPassword2)-1];
  if (*lastCharMasterPassword2 == '\n') {
    *lastCharMasterPassword2 = '\0';
  }
  // If not, it is still in the buffer and will cause trouble later..
  else {
    emptyBuffer();
  }

  // Check it matches
  if (strcmp(masterPassword, masterPassword2) != 0) {
    printf("ERROR: Passwords you entered don't match...\n");
    sodiumFree(masterPassword);
    sodiumFree(masterPassword2);
    return NULL;
  }

  // We don't need the second master password anymore
  sodiumFree(masterPassword2);
  masterPassword2 = NULL;

  // Computing the hash of the password
  printf("Computing hash (this could takes time)...\n");
  if (crypto_pwhash_str(hashed_password, masterPassword, strlen(masterPassword),
  hash_OPSLIMIT, hash_MEMLIMIT) != 0) {
    fprintf(stderr, "Out of memory while computing hash.\n");
    // We free some variables
    sodiumFree(masterPassword);
    sodiumFreeKey(key);
    return NULL;
  }

  // We generate a salt for this password (no need to store it in a safe place in memory)
  randombytes_buf(salt, SALT_LEN);

  // We generate a key from the master password and the salt  
  if (crypto_pwhash((unsigned char*) key, KEY_LEN, masterPassword, strlen(masterPassword), (unsigned char*) salt,
                    KDF_OPSLIMIT, KDF_MEMLIMIT,
                    KDF_ALG) != 0) {
    fprintf(stderr, "Out of memory while computing the key from the password.\n");
    // We free some variables
    sodiumFree(masterPassword);
    sodiumFreeKey(key);
    return NULL;
  }

  // We transform the salt in base64 for latter storage
  size_t salt_base64_len = Base64encode_len(SALT_LEN);
  char salt_base64[salt_base64_len];
  Base64encode(salt_base64, salt, SALT_LEN);

  // We concatenate the hash with the salt in base64
  char strToStore[strlen(hashed_password) + 1 + salt_base64_len];
  strcpy(strToStore, hashed_password);
  strcat(strToStore, " ");
  strcat(strToStore, salt_base64);

  // We try to write the hash with the salt in the masterPassword file
  int ret = writeInFile(FOLDER_NAME "/" MASTERPASSWORD_FILE, strToStore);

#if DEBUG
  printf("Your password is: /%s/\n", masterPassword);
  printf("The comput hash is: %s\n", hashed_password);
#endif

  // If an error occured, we free the allocated memory and return NULL
  if (ret < 0) {
    sodiumFreeKey(key);
    key = NULL;
  }

  sodiumFree(masterPassword);
  masterPassword = NULL;

#if DEBUG
  char key_base64[Base64encode_len(KEY_LEN)];
  Base64encode(key_base64, key, KEY_LEN);
  printf("Key: %s\n", key_base64);
#endif

  // We return the key that will be used to encrypt and decrypt password later on
  return key;
}

char* askAndCheckMasterPassword() {
  // Allow some space for the key, password, the hash and the salt
  char* key = sodiumAlloc(KEY_LEN, sizeof(char));
  char* masterPassword = sodiumAlloc(PW_SIZE, sizeof(char));
  char hashed_password[crypto_pwhash_STRBYTES];
  char salt[SALT_LEN];
  char salt_base64[Base64encode_len(SALT_LEN)];

  // Get the actual hashed password and the salt
  char* masterPasswordFileContent = getFileContent(FOLDER_NAME "/" MASTERPASSWORD_FILE);
  char** content = str_split(masterPasswordFileContent, ' ');
  strcpy(hashed_password, content[0]);
  strcpy(salt_base64, content[1]);

  size_t content_size = 2;
  for (size_t i = 0; i < content_size; ++i) {
    free(content[i]);
  }
  free(content);
  free(masterPasswordFileContent);
  masterPasswordFileContent = NULL;

  // Ask for the master password
  printf("Enter your password: ");
  fgets(masterPassword, PW_SIZE, stdin);
  // We remove the last \n of the password (from the user hitted ENTER) if not already cropped by fgets
  char* lastCharMasterPassword = &masterPassword[strlen(masterPassword)-1];
  if (*lastCharMasterPassword == '\n') {
    *lastCharMasterPassword = '\0';
  }
  // If not, it is still in the buffer and will cause trouble later..
  else {
    emptyBuffer();
  }

  printf("Checking (this could takes time)...\n");

#if DEBUG
  printf("The stored hash is: %s\n", hashed_password);
  printf("Your password is: /%s/\n", masterPassword);
#endif

  // Check if it matches
  if (crypto_pwhash_str_verify(hashed_password, masterPassword, strlen(masterPassword)) != 0) {
    sodiumFree(masterPassword);
    sodiumFreeKey(key);
    return NULL;
  }

  // We generate a key from the master password and the salt
  Base64decode(salt, salt_base64);
  if (crypto_pwhash((unsigned char*) key, KEY_LEN, masterPassword, strlen(masterPassword), (unsigned char*) salt,
                    KDF_OPSLIMIT, KDF_MEMLIMIT,
                    KDF_ALG) != 0) {
    fprintf(stderr, "Out of memory while computing the key from the password.\n");
    // We free some variables
    sodiumFree(masterPassword);
    sodiumFreeKey(key);
    return NULL;
  }

#if DEBUG
  printf("And got the salt: %s\n", salt_base64);
  char key_base64[Base64encode_len(KEY_LEN)];
  Base64encode(key_base64, key, KEY_LEN);
  printf("Key: %s\n", key_base64);
#endif

  return key;
}

char* changeMasterPassword(char* oldKey) {

  // We get the new key
  char* newKey = createMasterPasswordFileAndKey();
  if (newKey == NULL) {
    return oldKey;
  }

  // Just in case there will be some error while re-encrypting..
  size_t nbLostPasswords = 0;

  // If there is some password to re-encrypt
  size_t nbFiles = getNumberOfFiles(FOLDER_NAME);
  if (nbFiles > 0) {
    printf("Re-encrypting the stored passwords with the new key...\n");
  
    // We get the password files
    char** filenames = getFilenamesBase64(FOLDER_NAME, nbFiles);

    // For each, we will do the following:
    for (size_t i = 0; i < nbFiles; ++i) {

      // We get the password and nonce
      char completeFileName[strlen(FOLDER_NAME) + 1 + strlen(filenames[i])];
      strcpy(completeFileName, FOLDER_NAME "/");
      strcat(completeFileName, filenames[i]);
      char* passwordAndNonce = getFileContent(completeFileName);

      // We delete the file storing the password
      if (deletePasswordFileBase64(filenames[i]) != 0) {
        fprintf(stderr, "Error while delete old password file during re-encryption...\n");
        ++nbLostPasswords;
        free(filenames[i]);
        continue;
      }

      // We free the filename since we don't need it anymore
      free(filenames[i]);

      // We decrypt the password with the old key. Result is send in secure storage
      char* password_clear = decPassword(passwordAndNonce, oldKey);

      if (password_clear == NULL) {
        fprintf(stderr, "Error while decrypting a password during re-encryption...\n");
        ++nbLostPasswords;
        continue;
      }

      // We encrypt the password (from secure storage) with the new key
      // Returns the encrypted password and nonce in regular storage
      char* newPasswordAndNonce = encPassword(password_clear, newKey);

      if (newPasswordAndNonce == NULL) {
        fprintf(stderr, "Error while encrypting a password during re-encryption...\n");
        ++nbLostPasswords;
        free(newPasswordAndNonce);
        continue;
      }

      // We save it in the file
      if (writeInFile(completeFileName, newPasswordAndNonce) != 0) {
        fprintf(stderr, "Error while saving the new password during re-encryption...\n");
        ++nbLostPasswords;
        free(newPasswordAndNonce);
        continue;
      }

      // We free the related memory for the passwords and the filename
      free(newPasswordAndNonce);
    }

    // We free the filename container
    free(filenames);
    filenames = NULL;
  }

  // We delete the old key
  sodiumFreeKey(oldKey);
  oldKey = NULL;

  // If lost some passwords, we declare it
  if (0 < nbLostPasswords) {
    printf("We are sorry to tell that %ld password(s) could not be recovered "
    "(encrypted with old password, altered,...) during the process.\n", nbLostPasswords);
  }

  return newKey;
}

void encryptMessage(char* ciphertext, char* message, size_t message_len, char* nonce, char* key) {
  crypto_aead_xchacha20poly1305_ietf_encrypt((unsigned char*) ciphertext, NULL,
                                            (unsigned char*) message, message_len,
                                            ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
                                            NULL, (unsigned char*) nonce, (unsigned char*) key);
}

int decryptMessage(char* decrypted, size_t* decrypted_len, char* ciphertext, char* nonce, char* key) {
  return crypto_aead_xchacha20poly1305_ietf_decrypt((unsigned char*) decrypted, (unsigned long long*) decrypted_len,
                                                NULL,
                                                (unsigned char*) ciphertext, ENC_PW_LEN,
                                                ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
                                                (unsigned char*) nonce, (unsigned char*) key);
}

char* encPassword(char* password_clear, char* key) {

  char password[ENC_PW_LEN];
  char nonce[NONCE_LEN];

  // We generate a nonce
  randombytes_buf((unsigned char*) nonce, sizeof nonce);

  // We encrypt the password
  encryptMessage(password, password_clear, PW_SIZE, nonce, key);

  // We delete the password since we don't need it any more
  sodiumFree(password_clear);
  password_clear = NULL;

  // We transform the password in base64
  const size_t password_base64_len = Base64encode_len(ENC_PW_LEN);
  char password_base64[password_base64_len];
  Base64encode(password_base64, password, ENC_PW_LEN);

  // We transform the nonce in base64
  const size_t nonce_base64_len = Base64encode_len(NONCE_LEN);
  char nonce_base64[nonce_base64_len];
  Base64encode(nonce_base64, nonce, NONCE_LEN);

#if DEBUG
  printf("Password: %s\n", password_base64);
  printf("Nonce: %s\n", nonce_base64);
#endif

  char* passwordAndNonce = calloc(password_base64_len + 1 + nonce_base64_len, sizeof(char));

  strcpy(passwordAndNonce, nonce_base64);
  strcat(passwordAndNonce, " ");
  strcat(passwordAndNonce, password_base64);

  return passwordAndNonce;
}

char* decPassword(char* passwordAndNonce, char* key) {

  // We get the encrypted password and the nonce in base64
  char** content = str_split(passwordAndNonce, ' ');

  // We free some unused space
  free(passwordAndNonce);
  
  // We decode the nonce
  char* nonce_base64 = content[0];
  char nonce[NONCE_LEN];
  Base64decode(nonce, nonce_base64);

  // We decode the password
  char* password_base64 = content[1];
  char enc_password[ENC_PW_LEN];
  Base64decode(enc_password, password_base64);

#if DEBUG
  printf("Password: %s\n", password_base64);
  printf("Nonce: %s\n", nonce_base64);
#endif

  // We free allocated memory
  for (size_t i = 0; i < 2; ++i) {
    free(content[i]);
  }
  free(content);
  
  // We decrypt the password
  char* password_clear = sodiumAlloc(PW_SIZE, sizeof(char));
  size_t decrypted_len;
  int ret = decryptMessage(password_clear, &decrypted_len, enc_password, nonce, key);
  if (ret != 0) {
    printf("The password has been forged!\n");
    sodiumFree(password_clear);
    return NULL;
  }

#if DEBUG
  printf("Password: %s\n", password_clear);
  printf("Password length: %ld\n", strlen(password_clear));
  printf("Decrypted length: %ld\n", decrypted_len);
#endif

  return password_clear;
}

char* sodiumAlloc(const size_t length, const size_t unitSize) {
  // Take the required space
  char* masterPassword = (char*)sodium_allocarray(length, unitSize);

  // Prevent from swapping the allocated memory space
  sodium_mlock(masterPassword, length);

  return masterPassword;
}

void sodiumFree(char* masterPassword) {
  // Erase memory and re-allow swapping
  sodium_munlock(masterPassword, PW_SIZE);

  // Free memory
  sodium_free(masterPassword);
}

void sodiumFreeKey(char* key) {
  // Erase memory and re-allow swapping
  sodium_munlock(key, KEY_LEN);

  // Free memory
  sodium_free(key);
}