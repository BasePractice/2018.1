#ifndef C_CW_PROGRAMMING_PRACTICE_CONSOLE_GUI_H
#define C_CW_PROGRAMMING_PRACTICE_CONSOLE_GUI_H

#include <base_macro.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum Key {
    KeyUp,
    KeyDown,
    KeyLeft,
    KeyRight,
    KeyUnknown
};

enum Event {
    KEY_DOWN,
    KEY_UP,
    WINDOW,
    NONE
};

struct Console;

bool console_init(struct Console **c);
bool console_destroy(struct Console **c);

void console_clear(struct Console *c);
void console_update(struct Console *c);


int console_height(struct Console *c);
int console_width(struct Console *c);

enum Key console_translate_key(int key);

void console_down(struct Console *c);
void console_up(struct Console *c);
void console_left(struct Console *c);
void console_right(struct Console *c);

void console_text(struct Console *c, const char *text);
void console_text_position(struct Console *c, int x, int y, const char *text);
void console_move(struct Console *c, int x, int y);

int console_ch(struct Console *c);
enum Event console_event(struct Console *c);

int console_sleep(int milliseconds);

#if defined(__cplusplus)
}
#endif

#endif //C_CW_PROGRAMMING_PRACTICE_CONSOLE_GUI_H
