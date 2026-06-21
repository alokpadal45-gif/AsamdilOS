#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Blocks until a line of input (terminated by Enter) is
 * available, echoes characters to the screen, and copies
 * the result (without the trailing newline) into buf. */
void keyboard_read_line(char *buf, int max_len);

/* Blocks until a single key is pressed and returns its
 * ASCII value (0 for non-printable keys). */
char keyboard_getchar(void);

#endif
