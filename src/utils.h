#pragma once

/*
 * This methods empty the buffer until it finds a '\n' or an EOF
 */
void emptyBuffer();

/*
 * Safely get an int from the user
 */
int getInputInt(unsigned int min, unsigned int max);

/*
 * Split a string based on char delimiter
 * Inspired by: https://stackoverflow.com/questions/921str_splitstr_split
 */
char** str_split(char* a_str, const char a_delim);