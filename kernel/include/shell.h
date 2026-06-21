#ifndef SHELL_H
#define SHELL_H

/* Simple username/password login screen.
 * Loops until the correct credentials are entered. */
void login_screen(void);

/* Main command loop - never returns. */
void shell_run(void);

#endif
