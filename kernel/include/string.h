#ifndef STRING_H
#define STRING_H

#include "types.h"

u32  strlen(const char *str);
int  strcmp(const char *a, const char *b);
int  strncmp(const char *a, const char *b, u32 n);
void strcpy(char *dest, const char *src);
void strcat(char *dest, const char *src);
void *memset(void *dest, int val, u32 len);
void *memcpy(void *dest, const void *src, u32 len);
int  str_to_int(const char *str);
void int_to_str(int num, char *out);

/* splits str in-place on the first space, returns pointer to the
 * remainder (or NULL if there is no second word) */
char *split_first_word(char *str);

#endif
