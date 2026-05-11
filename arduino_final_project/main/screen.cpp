#include "screen.hpp"

Screen::Screen() {
    //
}

Screen::~Screen() = default;


void Screen::print_frame(gameState &state) {
    Serial.println(F("@____________________________@"));
    for (uint8_t y = 0; y < GAME_GRID_Y_AXIS_LEN; y++) {
        char serial_s[GAME_GRID_X_AXIS_LEN + 4];
        serial_s[0] = '|';
        for (uint8_t x = 0; x < GAME_GRID_X_AXIS_LEN; x++) {
            serial_s[x + 1] = state.grid[x][y].toChar();
        }
        serial_s[GAME_GRID_X_AXIS_LEN + 1] = '|';
        serial_s[GAME_GRID_X_AXIS_LEN + 2] = '\n';
        serial_s[GAME_GRID_X_AXIS_LEN + 3] = '\0';
        Serial.print(serial_s);
    }
    Serial.println(F("@____________________________@"));
}