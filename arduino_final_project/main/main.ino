#include "game.hpp"
#include "inputs.hpp"
#include "screen.hpp"

#include <Arduino.h> // Only for the freeMemory function

/**
 * Not to complicated function who determine the heap
 * size in bytes using runtime variables. It is used
 * to determine if we reach an overflow.
 */
int freeMemory() {
    // This two address are determine by the linker script.
    extern char *__brkval; // The top address of the Heap
    extern char __heap_start; // The start address of the Heap

    char top; // Local variable: its address is on the stack

    // Calculation of free memory:
    // - If __brkval == 0: the heap has not yet been used, we take __heap_start as reference.
    // - Otherwise, we use __brkval (current address of the heap).
    // - &top gives the current address of the top of the stack.
    // - The difference between &top and the end of the heap gives the free space.
    return &top - (__brkval == 0 ? &__heap_start : __brkval);
}

// We done that for future heap allocation in setup()
Game* game = nullptr;
Screen* screen = nullptr;

void setup() {
    // 0. Starting the Serial Monitor
    Serial.begin(19200);
    Serial.println("BOOT OK");

    // 1. Creating a new Game object
    game = new Game(); // Allocation on the heap
    Serial.println("Game allocated");

    // 2. Constructing the Game object with default values
    game->start(); // Construct gameState 0 and put dummy values in each entities positions
    Serial.println("Game started");
    
    // 3. Loading the default level
    game->loadLevel(0); // Load debug level

    // 4. Creating a new Screen object
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