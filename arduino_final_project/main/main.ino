#include "game.hpp"
#include "inputs.hpp"
#include "screen.hpp"

Game* game = nullptr; // Variable globale
Screen* screen = nullptr;

void setup() {
    Serial.begin(19200);
    delay(100);
    game = new Game(); // Allocation dans le tas
    game->start();

    screen = new Screen();
}

void loop() {
    // 1. Calculating new game state
    gameState& state = game->step();

    // 2. Sending state to the output Screen
    screen->print_frame(state);
    delay(500);

    // 3. Here the user saw a game state and react
    // We register his inputs directly into the Game
    // object
    inputs newInputs = inputs(); // Default constructor will retrieve the current inputs state
    game->registerInputs(newInputs);
}