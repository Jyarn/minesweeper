#include <termios.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#define CELL_AT(x, y) (cells[y * w + x])

class Window {
    public:
        Window(int w, int h);
        ~Window(void);
        bool move(int x, int y);
    private:
        int countMines(int x, int y);
        void renderCell(int x, int y);
        struct termios oldTermConfig;
        std::vector<bool> cells;
        int w, h, x, y;
        const char* bgColour = "\x1b[48;2;255;255;255m";
        const char* colours[9] = {
            "",
            "\x1b[38;2;0;0;255m",
            "\x1b[38;2;128;0;255m",
            "\x1b[38;2;255;0;0m",
            "\x1b[38;2;0;0;128m",
            "\x1b[38;2;128;0;0m",
            "\x1b[38;2;0;128;128m",
            "\x1b[38;2;128;128;128m",
            "\x1b[38;2;128;0;128m",
        };
};


Window::Window(int w, int h)
{
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

    srand(time(NULL));
    this->w = w;
    this->h = h;
    x = 0;
    y = 0;

    cells = std::vector<bool>(w * h);
    for (auto cell : cells)
        cell = rand() & 1;

    std::cout << bgColour;
    for (int i = 1; i < w * 2; i++) {
        for (int j = 1; j < h + 1; j++) {
            std::cout << "\x1b[" << j << ";" << i << "f" << ' ';
        }
    }

    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
            renderCell(i, j);
}

Window::~Window(void)
{
    // disable alt buffer
    std::cout << "\x1b[?1049l\n";

    // exit raw mode
    tcsetattr(STDOUT_FILENO, TCSANOW, &oldTermConfig);
}

int
Window::countMines(int x, int y)
{
    int count = 0;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (i == 0 && j == 0) continue;
            int tx = x + i;
            int ty = y + j;

            if (tx < 0 || w <= tx) continue;
            if (ty < 0 || h <= ty) continue;

            count += CELL_AT(tx, ty);
        }
    }

    return count;
}

void
Window::renderCell(int x, int y)
{
    std::cout << "\x1b[";
    std::cout << y + 1 << ";";
    std::cout << x * 2 + 1 << "f";

    int nMines = countMines(x, y);

    assert(0 <= nMines && nMines < 9);
    bool isFocused = this->x == x && this->y == y;

    if (!CELL_AT(x, y)) {
        char cellTxt = nMines == 0 ? ' ' : nMines + '0';
        if (isFocused) std::cout << "\x1b[4m;\x1b[1m";
        std::cout << bgColour;
        std::cout << colours[nMines] << cellTxt;
        std::cout << "\x1b[22m\x1b[24m";

    } else {
        std::cout << "\x1b[38;2;0;0;0m\x1b[48;2;255;0;0m!";
    }

    std::cout << std::flush;
}

bool
Window::move(int dx, int dy)
{
    int tx = x + dx;
    int ty = y + dy;

    if (tx < 0 || w <= tx) return false;
    if (ty < 0 || h <= ty) return false;

    tx = x;
    x += dx;

    ty = y;
    y += dy;

    renderCell(x, y);
    renderCell(tx, ty);
    return CELL_AT(x, y);
}

int
main(int argc, char** argv)
{
    Window window = Window(25, 25);
    std::this_thread::sleep_for (std::chrono::seconds(10));
}
