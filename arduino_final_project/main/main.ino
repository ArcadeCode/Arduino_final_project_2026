#include "game.hpp"
#include "inputs.hpp"
#include "screen.hpp"

#include <Arduino.h> // Only for the freeMemory function

int freeMemory() {
  extern char *__brkval;
  extern char __heap_start;

  char top;
  
  return &top - (__brkval == 0 ? &__heap_start : __brkval);
}

// We done that for future heap allocation in setup()
Game* game = nullptr;
Screen* screen = nullptr;

void setup() {
    Serial.begin(19200);
    Serial.println("BOOT OK");

    game = new Game(); // Allocation on the heap
    Serial.println("Game allocated");

    game->start(); // Construct gameState 0 and put dummy values in each entities positions
    Serial.println("Game started");

    game->loadLevel(0); // Load debug level

    screen = new Screen(); // Allocation on the heap
    Serial.println("Screen allocated");
}

void loop() {
    /// 1. CALCULATING NEW GAME STATE ///
    gameState& state = game->step();

    /// 2. SENDING STATE TO THE OUTPUT SCREEN ///
    screen->print_frame(state);

    // 2.1 Debugging informations
    //  printPos is a lambda function for cleanup the debug texts.
    auto printPos = [](const char* name, GridPosition pos) {
        char buf[40]; // Represent a string
        // snprintf is a usefull function who write a formatted string on a buffer,
        // cannot overwrite sizeof(buffer) so really usefull.
        snprintf(buf, sizeof(buf), "%s: (%d, %d)", name, pos.x, pos.y);
        Serial.println(buf); // Send the output as a string
    };

    char buf[24];
    snprintf(buf, sizeof(buf), "Tick: %d | RAM: %d bytes", state.tick, freeMemory());
    Serial.println(buf);
    printPos("Pacman",       game->get_pacmanPosition());
    printPos("Blue Ghost",   game->get_blueGhostPosition());
    printPos("Red Ghost",    game->get_redGhostPosition());
    printPos("Pink Ghost",   game->get_pinkGhostPosition());
    printPos("Orange Ghost", game->get_orangeGhostPosition());

    // 2.2 Cleanup
    Serial.flush();
    delay(500);

    /// INPUT REGISTERING ///
    // We register his inputs directly into the Game object
    inputs newInputs = inputs(); // Default constructor will retrieve the current inputs state
    game->registerInputs(newInputs);
}