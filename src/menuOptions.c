#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menuOptions.h"
#include "crypto.h"
#include "files.h"
#include "base64.h"
#include "utils.h"
#include "const.h"   // This file contains the constants of the app

void storePassword(char* key) {
  // Ask for the keyword
  size_t keyword_size = 0;
  size_t keyword_base64_size = 0;
  char* keyword_base64 = NULL;
  char* keyword = calloc(PW_SIZE, sizeof(char));
  printf("Enter the keyword (URL, website name,...) (max " TOSTRING(PW_SIZE) "-1 chars): ");
  fgets(keyword, PW_SIZE, stdin);
  // We remove the last \n of the keyword (from the user hitted ENTER) if not already cropped by fgets
  char* lastCharKeyword = &keyword[strlen(keyword)-1];
  if (*lastCharKeyword == '\n') {
    *lastCharKeyword = '\0';
  }
  // If not, it is still in the buffer and will cause trouble later..
  else {
    emptyBuffer();
  }

  // Convert it in Base64
  keyword_size = strlen(keyword);
  keyword_base64_size = Base64encode_len(keyword_size);
  keyword_base64 = calloc(keyword_base64_size, sizeof(char));
  Base64encode(keyword_base64, keyword, keyword_size);

#if DEBUG
  printf(" Original: %s\n", keyword);
  printf(" Encoded : %s\n", keyword_base64);
#endif

  // Compute the complete path of the file
  char* completeFileName = calloc(strlen(FOLDER_NAME) + 1 + keyword_base64_size, sizeof(char));
  strcpy(completeFileName, FOLDER_NAME "/");
  strcat(completeFileName, keyword_base64);

  // Clear the allocated memory for the keyword and its base64
  free(keyword);
  free(keyword_base64);
  keyword = NULL;
  keyword_base64 = NULL;

  // Exit if there is already an associated password
  if (isFilePresent(completeFileName)) {
    printf("You already stored a password with this keyword.\n");
    free(completeFileName);
    return;
  }

  // Alloc some space for the password and it's encryption
  char* password_clear = sodiumAlloc(PW_SIZE, sizeof(char));

  // Ask for the password
  printf("Please choose your password (max " TOSTRING(PW_SIZE) "-1 chars): ");
  fgets(password_clear, PW_SIZE, stdin);
  // We remove the last \n of the password (from the user hitted ENTER) if not already cropped by fgets
  char* lastCharPasswordClear = &password_clear[strlen(password_clear)-1];
  if (*lastCharPasswordClear == '\n') {
    *lastCharPasswordClear = '\0';
  }
  // If not, it is still in the buffer and will cause trouble later..
  else {
    emptyBuffer();
  }

#if DEBUG
  printf(" Password: %s\n", password_clear);
  printf(" Password size: %ld\n", strlen(password_clear));
#endif

  // Encrypt the password
  char* passwordAndNonce = encPassword(password_clear, key);

  // Store the password and the nonce
  writeInFile(completeFileName, passwordAndNonce);
  
  // Clear the allocated memory for the keyword and the password
  free(completeFileName);
  free(passwordAndNonce);
  passwordAndNonce = NULL;
  completeFileName = NULL;

  printf("Your password has been saved !\n");
}

void retrievePassword(char* key) {
  // We see how many passwords the user has
  size_t nbFiles = getNumberOfFiles(FOLDER_NAME);
  char** filenames = NULL;

  // If no one, we do nothing
  if (nbFiles == 0) {
    printf("You do not have any password stored yet.\n");
    return;
  }

  // We get the names of the keyword and show them to the user
  filenames = getFilenames(FOLDER_NAME, nbFiles);
  printf("You have %li password(s) stored. Please make a choice:\n", nbFiles);
  for (size_t i = 0; i < nbFiles; ++i) {
    printf(" %ld. %s\n", i + 1, filenames[i]);
  }

  // We ask which one it want to get
  size_t choice = getInputInt(1, nbFiles);

  // We get it
  char* passwordAndNonce = getPasswordFileContent(filenames[choice-1]);

  // We free the allocated space for the filenames
  for (size_t i = 0; i < nbFiles; ++i) free(filenames[i]);
  free(filenames);
  filenames = NULL;

  // We decrypt the password
  char* password = decPassword(passwordAndNonce, key);
  if (password == NULL) {
    return;
  }
  
  printf("Here is the password:\n%s\n", password);

  // We free the allocated space for the password
  sodiumFree(password);
  password = NULL;
}

void deletePassword() {
  // We see how many passwords the user has
  size_t nbFiles = getNumberOfFiles(FOLDER_NAME);
  char** filenames = NULL;

  // If no one, we do nothing
  if (nbFiles == 0) {
    printf("You do not have any password stored yet.\n");
    return;
  }

  // We get the names of the keyword and show them to the user
  filenames = getFilenames(FOLDER_NAME, nbFiles);
  printf("You have %li password(s) stored. Please choose the one you want to delete:\n", nbFiles);
  for (size_t i = 0; i < nbFiles; ++i) {
    printf(" %ld. %s\n", i + 1, filenames[i]);
  }

  // We ask which one it want to delete
  size_t choice = getInputInt(1, nbFiles);

  // We ask to confirm this choice
  printf("If you are sure, confirm this choice:\n");
  size_t choice2 = getInputInt(1, nbFiles);
  if (choice == choice2) {
    // We delete the corresponding file
    if (deletePasswordFile(filenames[choice-1]) == 0) {
      printf("Password deleted !\n");
    }
  }
  else {
    printf("Your 2 choices don't match. Password %s will not be deleted.\n", filenames[choice-1]);
  }

  // We free the allocated space
  for (size_t i = 0; i < nbFiles; ++i) free(filenames[i]);
  free(filenames);
  filenames = NULL;
}