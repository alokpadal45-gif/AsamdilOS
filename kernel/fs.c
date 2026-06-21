/* =======================================================
 * fs.c
 * Samrat Karki - File System & Shell
 *
 * A small, fully in-memory "file system": a flat table of
 * named entries that can be either files (with text
 * content) or directories. This is sufficient to
 * demonstrate file/directory creation, deletion, listing
 * and reading from the shell without needing a real disk
 * driver.
 * ======================================================= */

#include "fs.h"
#include "screen.h"
#include "string.h"

typedef struct {
    char name[MAX_NAME_LEN];
    fs_type_t type;
    char content[MAX_FILE_CONTENT];
    u32 size;
    u8 used;
} fs_entry_t;

static fs_entry_t entries[MAX_FILES];

void fs_init(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        entries[i].used = 0;
    }
}

static int find_entry(const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) return i;
    }
    return -1;
}

int fs_mkdir(const char *name) {
    if (find_entry(name) != -1) {
        screen_print_color("Error: a file or directory with that name already exists\n", RED_ON_BLACK);
        return -1;
    }

    int slot = find_free_slot();
    if (slot == -1) {
        screen_print_color("Error: file system full\n", RED_ON_BLACK);
        return -1;
    }

    strcpy(entries[slot].name, name);
    entries[slot].type = FS_DIR;
    entries[slot].size = 0;
    entries[slot].content[0] = '\0';
    entries[slot].used = 1;

    screen_print("Directory created: ");
    screen_print(name);
    screen_print("\n");
    return 0;
}

int fs_create(const char *name) {
    if (find_entry(name) != -1) {
        screen_print_color("Error: a file or directory with that name already exists\n", RED_ON_BLACK);
        return -1;
    }

    int slot = find_free_slot();
    if (slot == -1) {
        screen_print_color("Error: file system full\n", RED_ON_BLACK);
        return -1;
    }

    strcpy(entries[slot].name, name);
    entries[slot].type = FS_FILE;
    entries[slot].size = 0;
    entries[slot].content[0] = '\0';
    entries[slot].used = 1;

    screen_print("File created: ");
    screen_print(name);
    screen_print("\n");
    return 0;
}

int fs_delete(const char *name) {
    int idx = find_entry(name);
    if (idx == -1) {
        screen_print_color("Error: no such file or directory\n", RED_ON_BLACK);
        return -1;
    }

    entries[idx].used = 0;
    screen_print("Deleted: ");
    screen_print(name);
    screen_print("\n");
    return 0;
}

int fs_write(const char *name, const char *content) {
    int idx = find_entry(name);
    if (idx == -1) {
        screen_print_color("Error: no such file\n", RED_ON_BLACK);
        return -1;
    }
    if (entries[idx].type != FS_FILE) {
        screen_print_color("Error: not a file\n", RED_ON_BLACK);
        return -1;
    }

    strcpy(entries[idx].content, content);
    entries[idx].size = strlen(content);
    return 0;
}

void fs_read(const char *name) {
    int idx = find_entry(name);
    if (idx == -1) {
        screen_print_color("Error: no such file\n", RED_ON_BLACK);
        return;
    }
    if (entries[idx].type != FS_FILE) {
        screen_print_color("Error: not a file\n", RED_ON_BLACK);
        return;
    }

    if (entries[idx].size == 0) {
        screen_print("(empty file)\n");
    } else {
        screen_print(entries[idx].content);
        screen_print("\n");
    }
}

void fs_list(void) {
    int any = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) continue;
        any = 1;

        if (entries[i].type == FS_DIR) {
            screen_print_color("[DIR]  ", YELLOW_ON_BLACK);
        } else {
            screen_print_color("[FILE] ", GREEN_ON_BLACK);
        }

        screen_print(entries[i].name);

        if (entries[i].type == FS_FILE) {
            screen_print("  (");
            screen_print_dec(entries[i].size);
            screen_print(" bytes)");
        }

        screen_print("\n");
    }

    if (!any) {
        screen_print("(file system is empty)\n");
    }
}
