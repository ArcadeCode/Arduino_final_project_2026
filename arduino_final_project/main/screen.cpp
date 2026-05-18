#include "screen.hpp"
#include "types.hpp"

#include "levels.hpp"

Screen::Screen() = default;

/**
 * @brief Print a frame of the current game state in the Serial Monitor
 * @note Need to put a Serial.begin() before calling this function.
 * 
 * Work by loading the background from PROGMEM and combining it with
 * the entities positions from the GameState to print a char representation
 * of each cell.
 * 
 * @attention we print line by line instead of frame by frame due to memory limitations.
 */
void Screen::print_frame(GameState &state) {
    char serial_s[GAME_GRID_X_AXIS_LEN + 4];

    serial_s[0] = '|';
    serial_s[GAME_GRID_X_AXIS_LEN + 1] = '|';
    serial_s[GAME_GRID_X_AXIS_LEN + 2] = '\n';
    serial_s[GAME_GRID_X_AXIS_LEN + 3] = '\0';

    Serial.println(F("@____________________________@"));

    for (uint8_t y = 0; y < GAME_GRID_Y_AXIS_LEN; y++) {

        for (uint8_t x = 0; x < GAME_GRID_X_AXIS_LEN; x++) {

            serial_s[x + 1] =
                cellBackgroundToChar(
                    // TODO: Not hardcoding the level background, we should read it from the current level in the GameState.
                    readLevelBackground(LEVEL_0_BG, x, y)
                );

            if (state.ghostPositions[0] == GridPosition(x, y)) {
                serial_s[x + 1] = 'R';
            }
            else if (state.ghostPositions[1] == GridPosition(x, y)) {
                serial_s[x + 1] = 'K';
            }
            else if (state.ghostPositions[2] == GridPosition(x, y)) {
                serial_s[x + 1] = 'B';
            }
            else if (state.ghostPositions[3] == GridPosition(x, y)) {
                serial_s[x + 1] = 'O';
            }
            else if (state.pacmanPosition == GridPosition(x, y)) {
                serial_s[x + 1] = 'P';
            }
        }

        // Print ONLY after the line is fully built
        Serial.print(serial_s);
    }

    Serial.println(F("@____________________________@"));
}