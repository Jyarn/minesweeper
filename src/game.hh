#pragma once

#include <termios.h>
#include <time.h>

#include <vector>

#include "window.hh"
#include "common.hh"

#define CELL_AT(x, y) (cells[y * w + x])

struct Cell {
    bool isMine, isRevealed, isFlagged;
};

typedef enum {
    Win,
    Lose,
    OnGoing
} GameState;

class Game {
    public:
        Game(unsigned int _w, unsigned int _h, unsigned int _nMines, Window* _window, int seed);
        void move(unsigned int x, unsigned int y);
        void flag(void);
        GameState reveal(void);
        void printBar(void);
    private:
        uint_t countMines(unsigned int x, unsigned int y);
        void renderCell(unsigned int x, unsigned int y);
        void reveal(uint_t x, uint_t y);
        void start(void);

        std::vector<Cell> cells;
        uint_t w, h, x, y, nMines, nPlaced, nRevealed;
        const char* bgColour = "\x1b[48;2;255;255;255m";
        const char* undiscoveredBgColour = "\x1b[48;2;160;160;160m";
        const char* mineColour = "\x1b[48;2;255;0;0m";
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

        Window* window;
        time_t timeStarted;
        bool started;
};
