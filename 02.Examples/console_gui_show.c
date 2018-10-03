#include <stdio.h>
#include <stdlib.h>
#include <console_gui.h>

int main(int argc, char **argv) {
    struct Console *c;
    volatile bool need_refresh = false;
    volatile bool is_running = true;
    int x = 10, y = 10;

    console_init(&c);
    console_clear(c);
    console_text_position(c, 0, 0, "Enter 'q' to exit");
    console_move(c, x, y);
    console_text(c, "o");
    while (is_running) {
        switch (console_event(c)) {
            case KEY_DOWN: {
                int ch = console_ch(c);
                switch (console_translate_key(ch)) {
                    case KeyDown:
                        console_down(c);
                        need_refresh = true;
                        break;
                    case KeyUp:
                        console_up(c);
                        need_refresh = true;
                        break;
                    case KeyRight:
                        console_right(c);
                        need_refresh = true;
                        break;
                    case KeyLeft:
                        console_left(c);
                        need_refresh = true;
                        break;
                    default:
                        is_running = ch != 'q' && ch != 'Q';
                        need_refresh = false;
                        //tick
                        console_sleep(100);
                        break;
                }
                break;
            }
            case WINDOW: {
                need_refresh = true;
                break;
            }
            case NONE:
                break;
            case KEY_UP:
                break;
        }
        if (need_refresh) {
            char buffer[256];
            console_clear(c);
            sprintf(buffer, "Enter 'q' to exit.");
            console_text_position(c, 0, 0, buffer);
            console_text(c, "o");
            console_update(c);
            need_refresh = false;
        }
    }
    console_destroy(&c);
    return EXIT_SUCCESS;
}
