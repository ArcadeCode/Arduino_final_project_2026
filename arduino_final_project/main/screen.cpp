#include "screen.hpp"

Screen::Screen() {
    //
}

Screen::~Screen() = default;

void Screen::print_frame(gameState &state) {
    // uint8_t because GAME_GRID_._AXIS_LEN is < 255
    Serial.print(F("@____________________________________@\n"));
    for (uint8_t i = 0; i < GAME_GRID_X_AXIS_LEN; i++) {
        Serial.print("|");
        for (uint8_t j = 0; j < GAME_GRID_Y_AXIS_LEN; j++) {
            Serial.print(state.grid[i][j].toChar());
        }
        Serial.print("|\n");
        Serial.flush();
    }
    Serial.print(F("@____________________________________@\n"));
}