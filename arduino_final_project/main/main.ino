#include "game.hpp"
#include "inputs.hpp"
#include "screen.hpp"
#include "levels.hpp"
#include "audio.hpp" 

#include <Arduino.h> // Only for the freeMemory function

#define DEBUG 1 // Set to 1 to enable debug output in the Serial Monitor, 0 to disable

static const int RANDOM_NOISE_PIN = A5; // Used for randomSeed()

/**
 * Not to complicated function who determine the heap
 * size in bytes using runtime variables. It is used
 * to determine if we reach an overflow.
 */
int freeMemory() {
    extern char __heap_start;
    extern char *__brkval;

    // SP (Stack Pointer) donne le sommet RÉEL de la pile en ce moment
    int free_memory;
    if ((int)__brkval == 0) {
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
    } else {
        free_memory = ((int)&free_memory) - ((int)__brkval);
    }
    return free_memory;
}

/*
We done that for future heap allocation in setup()
Note: We will allocate on the heap this objects
using `new` but, we have the responsibility to call
`free` after using game and screen, however this
two variables will never be stopping being used
so no need to free, when reset or unplugging the
arduino the memory will be cleared.
*/
Game*      game   = nullptr;
GameState* state  = nullptr; // Pointeur global — déréférencé partout avec * ou ->
Screen*    screen = nullptr;
Inputs     inputs = Inputs(nullptr);

void gameLoop(); // Forward declaration nécessaire dans les .ino

void setup() {
    // 0. Starting the Serial Monitor & set a random seed
    Serial.begin(19200);
    randomSeed(analogRead(RANDOM_NOISE_PIN)); // True randomness using electric noise on A0 pin;
    Serial.println(F("BOOT OK"));

    // 1. Creating a new Game object
    game = new Game(); // Allocation on the heap
    Serial.println(F("Game allocated"));

    Serial.print(F("Memory : "));
    Serial.println(freeMemory());

    // On pointe vers le state interne du Game, une fois pour toutes
    state = &game->getState();

    // 5. Creating a new Inputs object
    inputs.attributeGameState(*state);

    // Call AudioEngine
    AudioEngine::init();
    AudioEngine::play(SFX_INTRO); // Play intro music.

    // 4. Constructing the Game object with default values
    game->start(); // Construct GameState 0 and put dummy values in each entities positions
    Serial.println(F("Game started"));

    // 3. Loading the default level
    loadLevel(*state, 0); // Load debug level directly in the global state to save memory and avoid stack overflow from loading a level in Game::start().
    Serial.println(F("Level 0 loaded in GameState"));

    Serial.print(F("Memory : "));
    Serial.println(freeMemory());

    // 4. Creating a new Screen object
    screen = new Screen(); // Allocation on the heap
    Serial.println(F("Screen allocated"));

    Serial.print(F("Memory : "));
    Serial.println(freeMemory());
}

void loop() {
    gameLoop();

    if (state->isWin) {
        Serial.println(F("LEVEL COMPLETE!"));
        loadLevel(*state, state->level + 1);
        AudioEngine::play(SFX_INTRO); // Replay intro music on new level
        game->start();
    } else if (state->isGameOver) {
        Serial.println(F("GAME OVER!"));
        loadLevel(*state, state->level);
        game->start();
    }
}

void gameLoop() {
    AudioEngine::update(); // Update audio (handle note timing and sequence progression)
    /// 1. CALCULATING NEW GAME STATE ///
    // step() retourne une GameState& — on l'utilise comme ref locale,
    // pas besoin de réassigner state (il pointe déjà sur le même objet)
    GameState& s = game->step();

    /// 2. SENDING STATE TO THE OUTPUT SCREEN ///
    screen->print_frame(s, game->getGhosts());
    screen->serial_print_frame(s); // Debug

    // 2.1 Debugging informations
    // NOTE: This debug section can consume a lot of ram,
    // there was a lot of stack overflow error from here.
    #if DEBUG
    Serial.print(F("Tick: "));
    Serial.print(s.tick);
    Serial.print(F(" | RAM: "));
    Serial.print(freeMemory());
    Serial.println(F(" bytes"));

    // Print the remaining dots count
    Serial.print(F("Remaining dots: "));
    Serial.print(s.remainingDots);
    Serial.print(F("/"));
    Serial.println(s.totalDots);

    // Print pacman position & facing
    Serial.print(F("Pacman (x, y): "));
    Serial.print(game->get_pacmanPosition().x);
    Serial.print(F(", "));
    Serial.print(game->get_pacmanPosition().y);
    Serial.print(F(" | Facing: "));
    Serial.print(entityFacingToChar(game->get_pacmanFacing()));
    Serial.println();

    // Calling getGhostInformations() for each ghost
    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        Serial.println(game->get_ghostInformations(i));
    }

    // Print inputs debug informations
    Serial.println(inputs.get_informations());
    #endif

    // 2.2 Cleanup
    Serial.flush();
    delay(0);

    /// INPUT REGISTERING ///
    // We register player inputs and edit the GameState.pacmanFacing
    inputs.update();
}