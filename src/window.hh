#pragma once
#include <termios.h>

class Window {
    public:
        Window(void);
        ~Window(void);
        void getWindowSize(unsigned int* x, unsigned int* y);
        void setCursor(unsigned int x, unsigned int y);
        void clearScreen(void);
    private:
        struct termios oldTermConfig;
};
