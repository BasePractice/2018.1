#include <assert.h>
#include <windows.h>
#include "console_gui.h"

struct Console {
    int width;
    int height;
    HANDLE h_output;
    HANDLE h_input;
    COORD coord;

    int ch;
};

bool console_init(struct Console **c) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    CONSOLE_CURSOR_INFO info;

    if (c == 0) {
        return false;
    }
    (*c) = calloc(1, sizeof(struct Console));
    (*c)->h_output = GetStdHandle(STD_OUTPUT_HANDLE);
    (*c)->h_input = GetStdHandle(STD_INPUT_HANDLE);
    (*c)->ch = -1;

    SetConsoleMode((*c)->h_input, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
    GetConsoleScreenBufferInfo((*c)->h_output, &csbi);
    (*c)->width = csbi.dwSize.X;
    (*c)->height = csbi.dwSize.Y;
    GetConsoleCursorInfo((*c)->h_output, &info);
    info.bVisible = FALSE;
    SetConsoleCursorInfo((*c)->h_output, &info);
    (*c)->coord.X = 0;
    (*c)->coord.Y = 0;
    return true;
}

bool console_destroy(struct Console **c) {
    if (c != 0) {
        if ((*c) != 0) {
            CloseHandle((*c)->h_output);
            CloseHandle((*c)->h_input);
            free((*c));
        }
        (*c) = 0;
    }
    return true;
}

void console_update(struct Console *c) {
    assert(c != 0);
    FlushFileBuffers(c->h_output);
}

void console_clear(struct Console *c) {
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    assert(c != 0);
    dwConSize = (DWORD) (c->width * c->height);
    FillConsoleOutputCharacter(c->h_output, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten);
    GetConsoleScreenBufferInfo(c->h_output, &csbi);
    FillConsoleOutputAttribute(c->h_output, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(c->h_output, coordScreen);
}

enum Key console_translate_key(int key) {
    switch (key) {
        case VK_UP:
            return KeyUp;
        case VK_DOWN:
            return KeyDown;
        case VK_LEFT:
            return KeyLeft;
        case VK_RIGHT:
            return KeyRight;
        default:
            return KeyUnknown;
    }
}

void console_down(struct Console *c) {
    assert(c != 0);
    ++c->coord.Y;
    if (c->coord.Y >= console_height(c) - 1)
        c->coord.Y = (SHORT) (console_height(c) - 1);
    SetConsoleCursorPosition(c->h_output, c->coord);
}

void console_up(struct Console *c) {
    assert(c != 0);
    --c->coord.Y;
    if (c->coord.Y < 0)
        c->coord.Y = 0;
    SetConsoleCursorPosition(c->h_output, c->coord);
}

void console_left(struct Console *c) {
    assert(c != 0);
    --c->coord.X;
    if (c->coord.X < 0)
        c->coord.X = 0;
    SetConsoleCursorPosition(c->h_output, c->coord);
}

void console_right(struct Console *c) {
    assert(c != 0);
    ++c->coord.X;
    if (c->coord.X >= console_width(c) - 1)
        c->coord.X = (SHORT) (console_width(c) - 1);
    SetConsoleCursorPosition(c->h_output, c->coord);
}


int console_height(struct Console *c) {
    assert(c != 0);
    return c->height;
}

int console_width(struct Console *c) {
    assert(c != 0);
    return c->width;
}

void console_text_position(struct Console *c, int x, int y, const char * text) {
    COORD coord = {(SHORT) x, (SHORT) y};

    assert(c != 0);
    SetConsoleCursorPosition(c->h_output, coord);
    console_text(c, text);
    SetConsoleCursorPosition(c->h_output, c->coord);
}

void console_text(struct Console *c, const char * text) {
    DWORD written;

    assert(c != 0);
    WriteConsole(c->h_output, text, (DWORD) lstrlen(text), &written, 0);
}

void console_move(struct Console *c, int x, int y) {
    assert(c != 0);
    c->coord.X = (SHORT) x;
    c->coord.Y = (SHORT) y;
    SetConsoleCursorPosition(c->h_output, c->coord);
}

enum Event console_event(struct Console *c) {
    INPUT_RECORD record;
    DWORD read = 0;

    assert(c != 0);
    if (ReadConsoleInput(c->h_input, &record, 1, &read)) {
        if (read == 1) {
            switch (record.EventType) {
                case KEY_EVENT: {
                    if (record.Event.KeyEvent.bKeyDown) {
                        if (record.Event.KeyEvent.uChar.UnicodeChar == 0) {
                            c->ch = record.Event.KeyEvent.wVirtualKeyCode;
                        } else {
                            c->ch = record.Event.KeyEvent.uChar.UnicodeChar;
                        }
                        return KeyDownEvent;
                    } else {
                        return KeyUpEvent;
                    }
                }
                case WINDOW_BUFFER_SIZE_EVENT: {
                    c->width = record.Event.WindowBufferSizeEvent.dwSize.X;
                    c->height = record.Event.WindowBufferSizeEvent.dwSize.Y;
                    return WindowEvent;
                }
                default:
                    break;
            }
        }
    }
    return NoneEvent;
}

int console_ch(struct Console *c) {
    assert(c != 0);
    return c->ch;
}
