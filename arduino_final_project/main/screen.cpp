#include "screen.hpp"

Screen::Screen() {
    //
}

Screen::~Screen() = default;

void Screen::print_frame(gameState &state) {
    Serial.begin(9600);

    for (size_t i = 0; i < GAME_GRID_X_AXIS_LEN; i++) {
        for (size_t j = 0; j < GAME_GRID_Y_AXIS_LEN; j++) {
            Serial.print(state.grid[i][j].toChar());
        }
        Serial.println();
        
    }
    
}