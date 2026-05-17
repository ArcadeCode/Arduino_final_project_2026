#include "screen.hpp"

Screen::Screen() = default;

/**
 * @brief Print a frame of the current game state in the Serial Monitor
 * @note Need to put a Serial.begin() before calling this function.
 * 
 * @attention we print line by line instead of frame by frame due to memory limitations.
 */
void Screen::print_frame(GameState &state) {
    char serial_s[GAME_GRID_X_AXIS_LEN + 4]; // Each line contain '|' + 28 Cells + '|' + '\n + '\0' = 4
    // This values in the array will never changed, we set them here to speed up allocation.
    serial_s[0] = '|';
    serial_s[GAME_GRID_X_AXIS_LEN + 1] = '|';
    serial_s[GAME_GRID_X_AXIS_LEN + 2] = '\n';
    serial_s[GAME_GRID_X_AXIS_LEN + 3] = '\0';

    // For each frame
    Serial.println(F("@____________________________@"));
    for (uint8_t y = 0; y < GAME_GRID_Y_AXIS_LEN; y++) {
        for (uint8_t x = 0; x < GAME_GRID_X_AXIS_LEN; x++) {
            // For each line
            serial_s[x + 1] = state.grid[y][x].toChar();
        }
        Serial.print(serial_s); // Print the line
    }
    Serial.println(F("@____________________________@"));
    // A frame was printed
}