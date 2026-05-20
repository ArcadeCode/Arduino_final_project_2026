#include "levels.hpp"
#include <avr/pgmspace.h> // For pgm_read_byte and pgm_read_word

/**
 * @brief Initialize a GameState for the given level, reading parameters from PROGMEM.
 */
void loadLevel(GameState& state, uint8_t level) {
    // Reset counters
    state.tick = 0;
    state.level = level;
    state.modePhase = 0;
    state.lastModeChangeMs = 0;

    // Load parameters from PROGMEM
    state.pacmanPosition = {
        pgm_read_byte(&LEVELS_PARAMETERS[level].pacmanStart.x),
        pgm_read_byte(&LEVELS_PARAMETERS[level].pacmanStart.y)
    };

    for (uint8_t i = 0; i < 4; i++) {
        state.ghostPositions[i] = {
            pgm_read_byte(&LEVELS_PARAMETERS[level].ghostStarts[i].x),
            pgm_read_byte(&LEVELS_PARAMETERS[level].ghostStarts[i].y)
        };
    }

    // Copy current level background from PROGMEM to RAM for easier access during the game.
    const uint8_t* src = (const uint8_t*)pgm_read_ptr(&LEVELS_PARAMETERS[level].background);
    for (uint8_t y = 0; y < GAME_GRID_Y_AXIS_LEN; y++) {
        for (uint8_t x = 0; x < GAME_GRID_X_AXIS_LEN / 4; x++) {
            state.levelBackground[y][x] = pgm_read_byte(&src[y * (GAME_GRID_X_AXIS_LEN / 4) + x]);   
        }
    }
    state.totalDots = pgm_read_word(&LEVELS_PARAMETERS[level].totalDots);
    state.remainingDots = state.totalDots;
}