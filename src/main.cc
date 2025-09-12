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
        Window(unsigned int w, unsigned int h, unsigned int nMines);
        ~Window(void);
        bool move(unsigned int x, unsigned int y);
    private:
        unsigned int countMines(unsigned int x, unsigned int y);
        void renderCell(unsigned int x, unsigned int y);
        void setCursor(unsigned int x, unsigned int y);
        struct termios oldTermConfig;
        std::vector<bool> cells;
        unsigned int w, h, x, y, nMines;
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


Window::Window(unsigned int w, unsigned int h, unsigned int nMines)
{
    // save old terminal config
    tcgetattr(STDOUT_FILENO, &oldTermConfig);

    // enter raw mode
    struct termios newConfig;
    cfmakeraw(&newConfig);
    tcsetattr(STDOUT_FILENO, TCSANOW, &newConfig);

    // enable alt buffer
    printf("\x1b[?1049h");

    // clear screen
    printf("\x1b[2J");

    // move cursor to home position
    printf("\x1b[H");

    // seed rng
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

    // set bg
    printf("%s", bgColour);

    for (int i = 0; i < w - 1; i++) {
        for (int j = 0; j < h; j++)
            renderCell(i, j);
}

Window::~Window(void)
{
    // disable alt buffer
    printf("\x1b[?1049l");
    fflush(stdout);

    // exit raw mode
    tcsetattr(STDOUT_FILENO, TCSANOW, &oldTermConfig);
}

unsigned int
Window::countMines(unsigned int x, unsigned int y)
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
Window::setCursor(unsigned int x, unsigned int y)
{
    printf("\x1b[%d;%df", y + 1, x*2 + 1);
}

void
Window::renderCell(unsigned int x, unsigned int y)
{
    setCursor(x, y);
    unsigned int nMines = countMines(x, y);

    assert(0 <= nMines && nMines < 9);
    bool isFocused = this->x == x && this->y == y;

    if (!CELL_AT(x, y)) {
        char cellTxt = nMines == 0 ? ' ' : nMines + '0';
        if (isFocused) printf("\x1b[1m");
        printf("%s%s%c", bgColour, colours[nMines], cellTxt);
        printf("\x1b[22m");

    } else {
       printf("\x1b[38;2;0;0;0m\x1b[48;2;255;0;0m!");
    }

    printf("\x1b[1D");
}

bool
Window::move(unsigned int dx, unsigned int dy)
{
    unsigned int tx = x + dx;
    unsigned int ty = y + dy;

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
