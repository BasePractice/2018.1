#include <assert.h>
#include "console_gui.h"

#if defined(WIN32)

#include <windows.h>
#else

#include <unistd.h>
#endif

int console_sleep(int milliseconds) {
#if defined(WIN32)
    Sleep((DWORD) milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}
