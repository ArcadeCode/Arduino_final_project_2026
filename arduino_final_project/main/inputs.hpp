#pragma once
#include "types.hpp"

#define USE_JOYSTICK 0 // Set to 1 to replace the joystick with 4 directional buttons, 0 to use the joystick (not implemented yet)

static const float JOYSTICK_DEADZONE = 0.2f; // Deadzone threshold for joystick input
static const int JOYSTICK_MAX_VALUE = 512; // Maximum value for joystick axis (assuming 10-bit ADC centered at 512)
// NOTE: A5 is already use in main.ino for random noise reading.
static const int SELECT_BUTTON_PIN = A0;
static const int START_BUTTON_PIN = A1;

#if USE_JOYSTICK == 0
static const int JOYSTICK_X_PIN = A2;
static const int JOYSTICK_Y_PIN = A3;
#endif

#if USE_JOYSTICK == 1
static const int DIRECTION_LEFT = 3;
static const int DIRECTION_RIGHT = 2;
static const int DIRECTION_UP = 1;
static const int DIRECTION_DOWN = 0;
#endif

/**
 * This file store all inputs gate to the game.
 */

/* Represent all inputs */
class Inputs {
private:
    GameState* state;

    EntityFacing joystickDirection; // Represent where the player want pacman to go, based on the joystick position and deadzone.
    bool start_is_pressed;
    bool select_is_pressed;

    void read_start_button();
    void read_select_button();
    void read_joystick();

public:
    Inputs(GameState* state);
    ~Inputs() = default;

    void attributeGameState(GameState& state) { this->state = &state; }
    void update(); // Read all inputs and update the state of the Inputs object.

    bool getStartButtonState() const { return this->start_is_pressed; }
    bool getSelectButtonState() const { return this->select_is_pressed; }
    EntityFacing getJoystickDirection() const { return this->joystickDirection; }

    char* get_informations(); // Return a string representation of the current inputs state, for debugging purposes.
};

