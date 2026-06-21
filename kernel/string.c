/* =======================================================
 * string.c
 * Small freestanding string/memory helper library used
 * throughout the kernel (no libc is available).
 * ======================================================= */

#include "string.h"

u32 strlen(const char *str) {
    u32 len = 0;
    while (str[len] != '\0') len++;
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, u32 n) {
    for (u32 i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0') {
            return (unsigned char)a[i] - (unsigned char)b[i];
        }
    }
    return 0;
}

void strcpy(char *dest, const char *src) {
    u32 i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void strcat(char *dest, const char *src) {
    u32 dlen = strlen(dest);
    u32 i = 0;
    while (src[i] != '\0') {
        dest[dlen + i] = src[i];
        i++;
    }
    dest[dlen + i] = '\0';
}

void *memset(void *dest, int val, u32 len) {
    u8 *d = (u8*)dest;
    for (u32 i = 0; i < len; i++) d[i] = (u8)val;
    return dest;
}

void *memcpy(void *dest, const void *src, u32 len) {
    u8 *d = (u8*)dest;
    const u8 *s = (const u8*)src;
    for (u32 i = 0; i < len; i++) d[i] = s[i];
    return dest;
}

/* Parses a (possibly negative) decimal integer. Returns 0 on
 * an empty or invalid string. */
int str_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    u32 i = 0;

    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') break;
        result = result * 10 + (str[i] - '0');
    }

    return result * sign;
}

void int_to_str(int num, char *out) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    char tmp[12];
    while (num > 0) {
        tmp[i++] = '0' + (num % 10);
        num /= 10;
    }

    int j = 0;
    if (is_negative) out[j++] = '-';
    while (i > 0) out[j++] = tmp[--i];
    out[j] = '\0';
}

char *split_first_word(char *str) {
    u32 i = 0;
    while (str[i] != '\0') {
        if (str[i] == ' ') {
            str[i] = '\0';
            /* skip any extra spaces */
            i++;
            while (str[i] == ' ') i++;
            if (str[i] == '\0') return NULL;
            return &str[i];
        }
        i++;
    }
    return NULL;
}
