#ifndef ALT_CONIO_H
#define ALT_CONIO_H

extern "C" {

/**
 * @brief Check if a key has been pressed (non-blocking)
 * @return 1 if a key is available, 0 otherwise
 */
int unix_kbhit(void);

/**
 * @brief Get a single character from keyboard (blocking)
 * @return The character code
 */
int unix_getch(void);

/**
 * @brief Set terminal to raw mode for immediate key input
 */
void set_conio_terminal_mode(void);

/**
 * @brief Restore terminal to normal mode
 */
void reset_terminal_mode(void);

}

#endif /* ALT_CONIO_H */