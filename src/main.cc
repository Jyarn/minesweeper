#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>

#include "common.hh"
#include "window.hh"
#include "game.hh"
#include "poll.h"

Preset presets[] = {
    { .w = 9, .h = 9, .nMines = 10, .name = "beginner" },
    { .w = 16, .h = 16, .nMines = 30, .name = "intermediate" },
    { .w = 30, .h = 16, .nMines = 99, .name = "expert" }
};

int
main(int argc, char** argv)
{
    // seed rng
    srand(time(NULL));

    Preset preset = presets[2];

    bool customGame = false;
    uint_t w, h, nMines;

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

        } else if (!strcmp("--custom", argv[i]) && argc-i > 3) {
            customGame = true;
            w = strtoui(argv[++i]);
            h = strtoui(argv[++i]);
            nMines = strtoui(argv[++i]);

        } else if (!strcmp("--preset", argv[i]) && argc-i > 1) {
            i += 1;

            for (int j = 0; j < sizeof(presets) / sizeof(Preset); j++) {
                if (!strcmp(argv[i], presets[j].name)) {
                    customGame = false;
                    preset = presets[j];
                    goto succ_;
                }
            }

            fprintf(stderr, "Unknow preset \"%s\"\n", argv[i]);
            return -1;

succ_:      continue;
        }
    }


    Window window = Window();
    Game* game;

    struct pollfd pfd = {
        .fd = STDIN_FILENO,
        .events = POLLIN
    };


    GameState curState;
    int gameNo = 0;
    char buff;

    while (++gameNo) {
restart_:
        if (customGame) game = new Game(w, h, nMines, &window);
        else game = new Game(preset, &window);

        curState = NotStarted;

        while (curState != Win && curState != Lose) {
            pfd.revents = 0;
            game->printBar();

            // poll on stdin for 1000ms
            int ready = poll(&pfd, 1, 1000);

            if (ready == 1 && pfd.revents & POLLIN) {
                int sz = read(STDIN_FILENO, &buff, 1);

                // check if stdin is closed
                if (sz < 1) return -1;

                switch (buff) {
                    case 'q':
                        return 0;
                    case 'h':
                        game->move(-1, 0);
                        break;
                    case 'j':
                        game->move(0, 1);
                        break;
                    case 'k':
                        game->move(0, -1);
                        break;
                    case 'l':
                        game->move(1, 0);
                        break;
                    case 'f':
                        game->flag();
                        break;
                    case 'd':
                        curState = game->reveal();
                        break;
                    case 'r':
                        goto restart_;
                }

            } else if (ready == -1)
                return -1;
        }

        time_t timeElapsed;
        float percentRevealed;
        game->getGameInfo(&timeElapsed, &percentRevealed);

        game->revealAll();
        window.setCursor(0, -2);

        printf("\x1b[1;31m\x1b[?25l");

        assert(curState == Win || curState == Lose);
        switch (curState) {
            case Win:
                printf("Game %d: Win, %lds (%3.2f)\n", gameNo, timeElapsed, percentRevealed);
                break;
            case Lose:
                printf("Game %d: Loss, %lds (%3.2f)\n", gameNo, timeElapsed, percentRevealed);
                break;
        }

        fflush(stdout);

        do
            read(STDIN_FILENO, &buff, 1);
        while (buff != 'r' && buff != 'q');

        printf("\x1b[?25h");
        fflush(stdout);
        if (buff == 'q') return 0;
    }
}
