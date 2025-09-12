#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#include <vector>
#include <algorithm>

#define CELL_AT(x, y) (cells[y * w + x])
#define MAX_NUM_LEN 6

unsigned int
strtoui(const char* s)
{
    unsigned int r = 0;
    for (int i = 0; i < MAX_NUM_LEN; i++) {
        if (s[i] < '0' || '9' < s[i]) return r;
        r = r * 10 + (s[i] - '0');
    }

    return r;
}


struct Cell {
    bool isMine, isRevealed, isFlagged;
};

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
            "\x1b[38;2;0;0;0m",
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

    unsigned int t = 0;
    for (auto cell : cells) {
        if (t == nMines)
            cell = 0;
        else {
            cell = rand() & 1;
            t -= cell;
        }
    }

    // set bg
    printf("%s", bgColour);

    for (int i = 0; i < w - 1; i++) {
        for (int j = 0; j < h; j++)
            printf("? ");

        printf("?\x1b[1E");
    }

    renderCell(0, 0);
    fflush(stdout);
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

    renderCell(tx, ty);
    renderCell(x, y);

    // since cursor is only moved right by one in the last call to renderCell
    // iff cell is boldened
    if (CELL_AT(x, y).isRevealed)
        printf("\x1b[1D");

    fflush(stdout);
}

void
Window::flag(void)
{
    Cell* cell = &CELL_AT(x, y);
    if (cell->isRevealed) return;

    cell->isFlagged = true;
    renderCell(x, y);

    // move left by one unit
    printf("\x1b[1D");
    fflush(stdout);
}

bool
Window::reveal(void)
{
    Cell* cell = &CELL_AT(x, y);
    if (cell->isFlagged) return false;

    cell->isRevealed = true;
    renderCell(x, y);
    printf("\x1b[1D");
    fflush(stdout);
    return cell->isMine;
}


int
main(int argc, char** argv)
{
    unsigned int w, h, nMines;
    w = h = 3;
    nMines = 2;

    for (int i = 1; i < argc; i++) {
        // provide developer with enough time to hook gdb up to monitor the
        // process. needed since we use the terminal instance as the display
        // and can't use it to debug
        //
        // in a seperate terminal, run `gdb -p {PID}}`
        // once in gdb, run `set var a = 0`
        // note: don't forget to build with debug symbols enabled
        if (!strcmp("--debug", argv[i])) {
            printf("Waiting for dev to connect\n");
            printf("(pid)=%d\n", getpid());
            char a = 1;
            while (a);
        }
        else if (!strcmp("--width", argv[i]))
            w = strtoui(argv[++i]);

        else if (!strcmp("--height", argv[i]))
            h = strtoui(argv[++i]);

        else if (!strcmp("--mines", argv[i]))
            nMines = strtoui(argv[++i]);
    }


    Window window = Window(w, h, nMines);
    sleep(5);
}
