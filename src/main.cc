#include <iostream>
#include <thread>
#include <chrono>

int
main(int argc, char** argv)
{
    // enable alt buffer
    std::cout << "\x1b[?1049h\n";

    // clear screen
    std::cout << "\x1b[2J\n";

    // move cursor to home position
    std::cout << "\x1b[H";

    std::cout << "\x1b[1;31mHello World!\n";
    std::this_thread::sleep_for (std::chrono::seconds(1));

    // disable alt buffer
    std::cout << "\x1b[?1049l\n";
}
