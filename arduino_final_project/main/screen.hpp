#pragma once

#include "game.hpp"
#include <Arduino.h>

class Screen {
private:
    /* data */
public:
    Screen(/* args */);
    ~Screen() = default;

    void print_frame(GameState &state);
};

