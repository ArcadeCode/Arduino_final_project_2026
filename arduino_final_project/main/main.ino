#include "game.hpp"
#include "inputs.hpp"
#include "screen.hpp"

Game game;
Screen screen;

void setup() {
    game.start();
}

void loop() {
    // 1. Calculating new game state
    gameState state = game.step();

    // 2. Sending state to the output Screen
    screen.print_frame(state);

    // 3. Here the user saw a game state and react
    // We register his inputs directly into the Game
    // object
    inputs newInputs = inputs(); // Default constructor will retrieve the current inputs state
    game.registerInputs(newInputs);
}