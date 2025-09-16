#include <unistd.h>
#include <stdio.h>

#include <algorithm>

#include "window.hh"
#include "common.hh"

Window::Window(void)
{
    // save old terminal config
    tcgetattr(STDOUT_FILENO, &oldTermConfig);

    // enter raw mode
    struct termios newConfig;
    cfmakeraw(&newConfig);
    tcsetattr(STDOUT_FILENO, TCSANOW, &newConfig);

    // enable alt buffer
    printf("\x1b[?1049h");

    clearScreen();
}

void
Window::clearScreen(void)
{
    printf("\x1b[2J");
}

void
Window::getWindowSize(uint_t* ww, uint_t* wh)
{
    setCursor(99999, 99999);
    printf("\x1b[6n\n");

    char bff[16];
    int sz = read(STDIN_FILENO, bff, 16);
    int i = 0;

    while (i < sz && bff[i++] != '\x1b');
    *wh = std::max(strtoui(&bff[++i]) - 2, (uint_t)0);

    while (i < sz && bff[i++] != ';');
    *ww = strtoui(&bff[i]);
    *ww = *ww / 2 + (*ww & 1); // ceil
}

void
Window::setCursor(uint_t x, uint_t y)
{
    printf("\x1b[%d;%df", y + 3, x*2 + 1);
}


Window::~Window(void)
{
    // disable alt buffer
    printf("\x1b[?1049l");
    fflush(stdout);

    // exit raw mode
    tcsetattr(STDOUT_FILENO, TCSANOW, &oldTermConfig);
}
