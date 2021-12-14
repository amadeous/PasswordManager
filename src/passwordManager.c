#include <stdio.h>
#include <sodium.h>
#include <string.h>
#include <signal.h>

#include "base64.h"
#include "files.h"
#include "crypto.h"
#include "utils.h"
#include "menuOptions.h"
#include "const.h"  // This file contains the constants of the app

int main() {
  if (sodium_init() < 0) {
    /* panic! the library couldn't be initialized, it is not safe to use */
    printf("Please install libsodium.\n");
    return EXIT_FAILURE;
  }
  
  char* key = NULL;
  unsigned int remainNbTries = NB_ALLOWED_TRIES;

  while (NULL == key) {
    // We check if the masterPassword file is present
    if (isFileMasterPasswordPresent()) {
      // (Optionnal) We check if the consistancy matches

      // Ask for the masterPassword
      key = askAndCheckMasterPassword();
      if (NULL == key) {
        printf("Wrong password. ");
        if (--remainNbTries > 0) {
          printf("Number of tries left: %i\n", remainNbTries);
        }
        else {
          printf("You have no more tries.\n");
          break;
        }
      }
      else {
        printf("Welcome back !");
        remainNbTries = NB_ALLOWED_TRIES;
#if DEBUG
        char key_base64[Base64encode_len(KEY_LEN)];
        Base64encode(key_base64, key, KEY_LEN);
        printf("\nKey: %s\n", key_base64);
#endif
      }
    }
    // If absent, offer to create one
    else {
      printf("Hi ! It seems it is your first time here...\nLet's define a master password for you.\n");
      key = createMasterPasswordFileAndKey();
      if (NULL != key) {
        printf("Your password has been successfully defined.\n");
#if DEBUG
        char key_base64[Base64encode_len(KEY_LEN)];
        Base64encode(key_base64, key, KEY_LEN);
        printf("Key: %s\n", key_base64);
#endif
      }
      else {
        return EXIT_FAILURE;
      }
    }

    if (NULL != key) {
      // The user is authenticated !
      unsigned int wantToLock = 0;
      unsigned int wantToQuit = 0;
      while(!wantToLock && !wantToQuit) {
        printf("\nPlease make a choice:\n"
              " 1. Retrieve a password\n"
              " 2. Store a new password\n"
              " 3. Delete a password\n"
              " 4. Change the master password\n"
              " 5. Lock the app\n"
              " 6. Quit the app\n");
        size_t choice = getInputInt(1, 6);
        switch(choice) {
          case 1:
            retrievePassword(key);
            break;
          case 2:
            storePassword(key);
            break;
          case 3:
            deletePassword();
            break;
          case 4: // Change the master password
            key = changeMasterPassword(key);
            break;
          case 5: // Lock the application
            wantToLock = 1;
            break;
          case 6: // Lock the application and exit
            wantToQuit = 1;
            break;
          default:
            fprintf(stderr, "This case should never be accessed...\n");
        }
      }

      // We delete the masterPassword from the memory
      sodiumFreeKey(key);
      key = NULL;
      printf("Access is locked.\n\n");
  
      if (wantToQuit) {
        break;
      }
    }
  }

  printf("Thank you for using passwordManager !\n");

  return EXIT_SUCCESS;
}