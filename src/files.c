#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "base64.h"
#include "files.h"
#include "const.h"  // This file contains the constants of the app

int isFilePresent(const char* file_name) {
  // We try to access the file
  return access(file_name, F_OK) != -1 ? 1 : 0;
}

int isFileMasterPasswordPresent() {
  return isFilePresent(FOLDER_NAME "/" MASTERPASSWORD_FILE);
}

char** getFilenames(const char* folderName, size_t nbFiles) {
  struct dirent *ent;
  DIR *dir = opendir(folderName);
  char** result = NULL;

  // We try to access the folder
  if (dir == NULL) {
    fprintf(stderr, "The folder \"%s\" does not exists.\n", folderName);
    closedir(dir);
    return result;
  }

  result = calloc(nbFiles, sizeof(char*));
  size_t counter = 0;
  while ((ent = readdir(dir)) != NULL) {
    char* file_name = ent->d_name;
    
    if (!strcmp(file_name, ".") || !strcmp(file_name, "..") || !strcmp(file_name, MASTERPASSWORD_FILE))
      continue;

    // We decode the filename
    char* decoded_file_name = calloc(Base64decode_len(file_name), sizeof(char));
    Base64decode(decoded_file_name, file_name);

    // We store it
    *(result + counter) = decoded_file_name;
    ++counter;
  }

  // We can close the folder
  closedir(dir);
  dir = NULL;

  return result;
}

char** getFilenamesBase64(const char* folderName, size_t nbFiles) {
  struct dirent *ent;
  DIR *dir = opendir(folderName);
  char** result = NULL;

  // We try to access the folder
  if (dir == NULL) {
    fprintf(stderr, "The folder \"%s\" does not exists.\n", folderName);
    closedir(dir);
    return result;
  }

  result = calloc(nbFiles, sizeof(char*));
  size_t counter = 0;
  while ((ent = readdir(dir)) != NULL) {
    char* file_name = ent->d_name;
    
    if (!strcmp(file_name, ".") || !strcmp(file_name, "..") || !strcmp(file_name, MASTERPASSWORD_FILE))
      continue;

    // We decode the filename
    char* stored_file_name = calloc(strlen(file_name) + 1, sizeof(char));
    strcpy(stored_file_name, file_name);

    // We store it
    *(result + counter) = stored_file_name;
    ++counter;
  }

  // We can close the folder
  closedir(dir);
  dir = NULL;

  return result;
}

int getNumberOfFiles(const char* folderName) {
  struct dirent *ent;
  DIR *dir = opendir(folderName);
  
  // We try to access the folder
  if (dir == NULL) {
    fprintf(stderr, "The folder \"%s\" does not exists.\n", folderName);
    closedir(dir);
    return -1;
  }

  // We iterate over the values to get the number of concerned files
  size_t counter = 0;
  while ((ent = readdir(dir)) != NULL) {
    if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, MASTERPASSWORD_FILE))
      continue;
    counter++;
  }

  // We can close the folder
  closedir(dir);
  dir = NULL;

  return counter;
}

int writeInFile(const char* file_name, const char* content) {
  FILE* fp;

  // We try to open the file
  fp = fopen(file_name, "w");
  if (NULL == fp) {
    fprintf(stderr, "Error while openning file %s (w mode) : %s\n", file_name, strerror(errno));
    return EXIT_FAILURE;
  }

  // We print what we want in it and check if something was written this way
  const size_t ret = fprintf(fp, "%s", content);
  if (ret < 0) {
    fprintf(stderr, "Error while printing in file %s", file_name);
    fclose(fp);
    return EXIT_FAILURE;
  }

  // We can close the file
  fclose(fp);
  fp = NULL;
  return EXIT_SUCCESS;
}

char* getFileContent(const char* file_name) {
  FILE* fp;
  char* content;
  size_t length;

  // We try to open the file
  fp = fopen(file_name, "r");
  if (NULL == fp) {
    fprintf(stderr, "Error while openning file %s (r mode) : %s\n", file_name, strerror(errno));
    return NULL;
  }

  // We get the size of the file
  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // We allocate the corresponding space
  content = calloc(length + 1, sizeof(char));

  // We read the content of the file
  fread(content, 1, length, fp);

  // We can close the file
  fclose(fp);
  fp = NULL;

  return content;
}

char* getPasswordFileContent(const char* file_name) {
  // We compute the base64 name of the file
  char file_name_base64[Base64decode_len(file_name)];
  Base64encode(file_name_base64, file_name, strlen(file_name));

  // We prepend it to the file_name
  char completeFilename[strlen(FOLDER_NAME) + 1 + strlen(file_name_base64)];
  strcpy(completeFilename, FOLDER_NAME "/");
  strcat(completeFilename, file_name_base64);

  // We call getFileContent with the corresponding name
  return getFileContent(completeFilename);
}

int deleteFile(const char* file_name) {
  int ret = remove(file_name);
  if (ret < 0) {
    fprintf(stderr, "Error while deleting file %s: %s", file_name, strerror(errno));
  }
  return ret;
}

int deletePasswordFileBase64(const char* file_name) {
  // We prepend the folder name to the file_name
  char completeFilename[strlen(FOLDER_NAME) + 1 + strlen(file_name)];
  strcpy(completeFilename, FOLDER_NAME "/");
  strcat(completeFilename, file_name);
  return deleteFile(completeFilename);
}

int deletePasswordFile(const char* file_name) {
  // We compute the base64 name of the file
  char file_name_base64[Base64decode_len(file_name)];
  Base64encode(file_name_base64, file_name, strlen(file_name));
  
  return deletePasswordFileBase64(file_name_base64);
}