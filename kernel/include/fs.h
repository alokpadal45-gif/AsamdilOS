#ifndef FS_H
#define FS_H

#include "types.h"

#define MAX_FILES        32
#define MAX_NAME_LEN     32
#define MAX_FILE_CONTENT 256

typedef enum {
    FS_FILE = 0,
    FS_DIR  = 1
} fs_type_t;

void fs_init(void);

int  fs_mkdir(const char *name);
int  fs_create(const char *name);
int  fs_delete(const char *name);
int  fs_write(const char *name, const char *content);
int  fs_copy(const char *src, const char *dest);  /* cp command */
void fs_read(const char *name);
void fs_list(void);

#endif
