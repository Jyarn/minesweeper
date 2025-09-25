# Minesweeper
Simple minsweeper clone

# Dependencies
Linux

# Build Steps
To build the project, run the following in a linux (or WSL) terminal:

 1. `mkdir build`
 2. `cd build`
 3. `cmake ..`
 4. `make`

# Playing The Game
To run the game make sure that you are in the `build/` directory and that the project is built 

To play with the expert preset, in the terminal run: `./minesweeper`

To play with other presets, run: `./minesweeper --preset ${PRESET}`, where `${PRESET} = {beginner, intermediate, expert}`

To play with a custom game, run: `./minesweeper --custom ${WIDTH} ${HEIGHT} ${MINE_COUNT}`

To debug, run: `./minesweeper --debug ...`
