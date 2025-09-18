#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <random>

#include "game.hh"
#include "window.hh"
#include "common.hh"


void
Game::printBar(void)
{
    window->setCursor(0, -2);
    printf("\x1b[0m");
    printf("\x1b[2K\x1b[1;31mTime Elapsed: %ld\x1b[1E", std::min(time(NULL) - timeStarted, (time_t)999));
    printf("\x1b[2K\x1b[1;31mMines Left: %d/%d", nPlaced, nMines);
    window->setCursor(x, y);
    fflush(stdout);
}

Game::Game(Preset preset, Window* window) : Game(preset.w, preset.h, preset.nMines, window) {}

Game::Game(uint_t _w, uint_t _h, uint_t _nMines, Window* _window)
{
    nRevealed = nPlaced = 0;
    timeStarted = time(NULL);
    window = _window;
    window->clearScreen();

    // get terminal dimensions
    uint_t ww, wh;
    window->getWindowSize(&ww, &wh);

    // clamp game board dimensions
    w = std::min(_w, ww);
    h = std::min(_h, wh);
    nMines = std::min(_nMines, w * h - 1);

    x = w / 2;
    y = h / 2;
    currentState = NotStarted;

    window->setCursor(0, 0);
    cells = std::vector<Cell>(w * h, { 0 });

    // set bg
    printf("\x1b[1;31m");
    printf("%s\x1b[38;2;0;0;0m", undiscoveredBgColour);

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w - 1; i++)
            printf("? ");

        // print a ? and move cursor down 1 at begining of new line
        printf("?\x1b[1E");
    }

    window->setCursor(x, y);
    fflush(stdout);
}

void
Game::start(void)
{
    if (currentState != NotStarted) return;

    currentState = OnGoing;
    int nCells = w * h;
    assert(nMines < nCells);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, nCells - 1);

    for (int n = 0; n < nMines; n++) {
        int i;

        do
            i = dist(rng);
        while (cells[i].isMine || i == y * w + x);

        cells[i].isMine = true;
    }

    assert(CELL_AT(x, y).isMine == false);
}

uint_t
Game::countMines(uint_t x, uint_t y)
{
    int count = 0;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (i == 0 && j == 0) continue;
            int tx = x + i;
            int ty = y + j;

            if (tx < 0 || w <= tx) continue;
            if (ty < 0 || h <= ty) continue;

            count += CELL_AT(tx, ty).isMine;
        }
    }

    return count;
}

void
Game::renderCell(uint_t x, uint_t y)
{
    window->setCursor(x, y);
    uint_t nmines = countMines(x, y);

    assert(0 <= nmines && nmines < 9);
    assert(0 <= x && x < w);
    assert(0 <= y && y < h);

    Cell cell = CELL_AT(x, y);
    Cell *cl, *cr;              // Cell-(Left/Right)
    const char *lc, *rc;        // (Left/Right)-Colour

    cl = cr = NULL;

    if (1 <= x) cl = &CELL_AT(x-1, y);
    if (x < w - 1) cr = &CELL_AT(x+1, y);

    lc = rc = undiscoveredBgColour;

    if (cl != NULL) {
        if (cl->isRevealed && !cl->isMine) lc = bgColour;
        else if (cell.isFlagged && cl->isFlagged) lc = mineColour;
        else if (cl->isRevealed && cell.isRevealed && cl->isMine && cell.isMine)
            lc = mineColour;
    }

    if (cr != NULL) {
        if (cr->isRevealed && !cr->isMine) rc = bgColour;
        else if (cell.isFlagged && cr->isFlagged) rc = mineColour;
        else if (cr->isRevealed && cell.isRevealed && cr->isMine && cell.isMine)
            rc = mineColour;
    }

    char cellText = '?';
    const char* textColour = "\x1b[38;2;0;0;0m";
    const char* textBg = undiscoveredBgColour;

    if (cell.isRevealed) {
        if (!cell.isMine) {
            cellText = nmines == 0 ? ' ' : nmines + '0';
            textColour = colours[nmines];
            textBg = lc = rc = bgColour;
        } else {
            cellText = '!';
            textBg = mineColour;
        }

    } else if (cell.isFlagged) {
        cellText = 'F';
        textBg = mineColour;
    }

    printf("\x1b[1m");
    if (cl) printf("\x1b[1D%s ", lc);
    printf("%s%s%c", textColour, textBg, cellText);
    if (cr) printf("%s ", rc);

    printf("\x1b[0m");
}

void
Game::move(uint_t dx, uint_t dy)
{
    uint_t tx = x + dx;
    uint_t ty = y + dy;

    if (tx < 0 || w <= tx) return;
    if (ty < 0 || h <= ty) return;

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
Game::flag(void)
{
    Cell* cell = &CELL_AT(x, y);
    if (cell->isRevealed) return;
    else if (nPlaced >= nMines && !cell->isFlagged) return;
    else if (currentState == NotStarted) return;

    cell->isFlagged = !cell->isFlagged;
    assert(cell->isFlagged == 0 || cell->isFlagged == 1);

    nPlaced += 2 * cell->isFlagged - 1;
    renderCell(x, y);

    // move left by one unit
    printf("\x1b[1D");
    fflush(stdout);
}

GameState
Game::reveal(void)
{
    Cell* cell = &CELL_AT(x, y);
    if (currentState == NotStarted) start();
    else if (cell->isFlagged) return OnGoing;

    if (cell->isRevealed) {
        int nr, nm, nf, nc;
        nc = nf = nr = nm = 0;
        for (int dx = -1; dx < 2; dx++) {
            for (int dy = -1; dy < 2; dy++) {
                uint_t nx = x + dx;
                uint_t ny = y + dy;

                if (0 > nx || nx >= w) continue;
                else if (0 > ny || ny >= h) continue;

                Cell* tc = &CELL_AT(nx, ny);
                nr += tc->isRevealed;   // #n revealed
                nm += tc->isMine;       // #n mines
                nf += tc->isFlagged;    // #n flagged
                nc += 1;                // #n cells
            }
        }

        if (nm == nf)
            for (int dx = -1; dx < 2; dx++)
                for (int dy = -1; dy < 2; dy++) {
                    uint_t nx = x + dx;
                    uint_t ny = y + dy;

                    if (0 > nx || nx >= w) continue;
                    else if (0 > ny || ny >= h) continue;
                    Cell* tc = &CELL_AT(nx, ny);

                    if (tc->isMine && ! tc->isFlagged) return Lose;
                    reveal(nx, ny);
                }
    }

    reveal(x, y);
    window->setCursor(x, y);
    fflush(stdout);

    if (cell->isMine) {
        cell->isRevealed = true;
        renderCell(x, y);
    }
    if (nRevealed == nMines) return Win;
    return OnGoing;
}

void
Game::reveal(uint_t x, uint_t y)
{
    if (0 > x || x >= w) return;
    else if (0 > y || y >= h) return;

    Cell* cell = &CELL_AT(x, y);
    if (cell->isFlagged || cell->isRevealed || cell->isMine) return;

    cell->isRevealed = true;
    nRevealed += 1;
    renderCell(x, y);

    int count = countMines(x, y);

    if (count == 0)
        for (int dx = -1; dx < 2; dx++) {
            for (int dy = -1; dy < 2; dy++) {
                reveal(x + dx, y + dy);
            }
        }
}
