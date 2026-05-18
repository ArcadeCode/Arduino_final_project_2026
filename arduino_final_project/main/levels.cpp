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

    state.totalDots = pgm_read_word(&LEVELS_PARAMETERS[level].totalDots);
    state.remainingDots = state.totalDots;
}