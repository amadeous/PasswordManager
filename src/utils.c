#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void emptyBuffer() {
   int buf;
   do {
      buf = getchar();
   } while (buf != EOF && buf != '\n');
}

int getInputInt(unsigned int min, unsigned int max) {
  int r = -1;
  while (1) {
    printf("Your choice [%d-%d]: ", min, max);
    if (scanf("%d", &r) == 1) {
      if (r >= min && r <= max) {
        emptyBuffer();
        return r;
      }
    }
    printf("Wrong input..\n");
    emptyBuffer();
  }
  return r;
}

char** str_split(char* a_str, const char a_delim) {
  char** result    = 0;
  size_t count     = 0;
  char* tmp        = a_str;
  char* last_delim = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp)
  {
    if (a_delim == *tmp)
    {
      count++;
      last_delim = tmp;
    }
    tmp++;
  }

  /* Add space for trailing token. */
  count += last_delim < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
      knows where the list of returned strings ends. */
  count++;

  result = calloc(count, sizeof(char*));

  if (result)
  {
    size_t idx  = 0;
    char* token = strtok(a_str, delim);

    while (token)
    {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}