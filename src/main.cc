#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <chrono>

class Window {
    public:
        Window(void);
        ~Window(void);
    private:
        struct termios oldTermConfig;
};

Window::Window(void) {
    // save old terminal config
    tcgetattr(STDOUT_FILENO, &oldTermConfig);

    // enter raw mode
    struct termios newConfig;
    cfmakeraw(&newConfig);
    tcsetattr(STDOUT_FILENO, TCSANOW, &newConfig);

    // enable alt buffer
    std::cout << "\x1b[?1049h\n";

    // clear screen
    std::cout << "\x1b[2J\n";

    // move cursor to home position
    std::cout << "\x1b[H";
}

Window::~Window(void) {
    // disable alt buffer
    std::cout << "\x1b[?1049l\n";

    // exit raw mode
    tcsetattr(STDOUT_FILENO, TCSANOW, &oldTermConfig);
}

int
main(int argc, char** argv)
{
    Window window = Window();

    std::cout << "\x1b[1;31mHello World!\n";
    std::this_thread::sleep_for (std::chrono::seconds(1));
}
