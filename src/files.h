#pragma once

/*
 * This method checks if the masterPassword file is present or not
 */
int isFilePresent(const char* file_name);
int isFileMasterPasswordPresent();

/*
 * This method prints the list of the websites the user stored
 */
char** getFilenames(const char* folderName, size_t nbFiles);

/*
 * This method prints the list of the websites the user stored in base64
 */
char** getFilenamesBase64(const char* folderName, size_t nbFiles);

/*
 * This method returns the number of password the user stored
 */
int getNumberOfFiles(const char* folderName);

/*
 * This methods write something in a file
 */
int writeInFile(const char* file_name, const char* content);

/*
 * Get the content of a file
 * /!\ Must free the allocated memory
 */
char* getFileContent(const char* file_name);

/*
 * Get the content of a password file in the default folder
 */
char* getPasswordFileContent(const char* file_name);

/*
 * Delete a password file in base64 in the default folder
 */
int deletePasswordFileBase64(const char* file_name);

/*
 * Delete a password file in the default folder
 */
int deletePasswordFile(const char* file_name);