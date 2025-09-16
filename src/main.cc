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

int
main(int argc, char** argv)
{
    // seed rng
    srand(time(NULL));

    uint_t w, h, nMines;
    time_t seed = time(NULL);
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

        else if (!strcmp("--seed", argv[i]))
            seed = strtoui(argv[++i]);
    }


    Window window = Window();
    Game game = Game(w, h, nMines, &window, seed);
    struct pollfd pfd = {
        .fd = STDIN_FILENO,
        .events = POLLIN
    };

    for (;;) {
        char c;

        // async io with poll
        pfd.revents = 0;
        game.printBar();
        int ready = poll(&pfd, 1, 1000);

        if (ready == 1 && pfd.revents & POLLIN) {
            int sz = read(STDIN_FILENO, &c, 1);
            if (sz < 1) return 0;

            switch(c) {
                case 'q':
                    return 0;
                case 'h':
                    game.move(-1, 0);
                    break;
                case 'j':
                    game.move(0, 1);
                    break;
                case 'k':
                    game.move(0, -1);
                    break;
                case 'l':
                    game.move(1, 0);
                    break;
                case 'f':
                    game.flag();
                    break;
                case 'd':
                    game.reveal();
                    break;
                case 'r':
                    game = Game(w, h, nMines, &window, seed);
                    break;
            }

        } else if (ready == -1) {
            return -1;
        }
    }
}
