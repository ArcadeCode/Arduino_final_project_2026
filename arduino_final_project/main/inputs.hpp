#pragma once
#include "types.hpp"

static const float JOYSTICK_DEADZONE = 0.2f; // Deadzone threshold for joystick input
static const int JOYSTICK_MAX_VALUE = 512; // Maximum value for joystick axis (assuming 10-bit ADC centered at 512)
// NOTE: A5 is already use in main.ino for random noise reading.
static const int SELECT_BUTTON_PIN = A0;
static const int START_BUTTON_PIN = A1;
static const int JOYSTICK_X_PIN = A2;
static const int JOYSTICK_Y_PIN = A3;

/**
 * This file store all inputs gate to the game.
 */

/* Represent all inputs */
class Inputs {
private:
    GameState& state;

    EntityFacing joystickDirection; // Represent where the player want pacman to go, based on the joystick position and deadzone.
    bool start_is_pressed;
    bool select_is_pressed;

    void read_start_button();
    void read_select_button();
    void read_joystick();

public:
    Inputs();
    ~Inputs() = default;

    void update(); // Read all inputs and update the state of the Inputs object.

    bool getStartButtonState() const { return start_is_pressed; }
    bool getSelectButtonState() const { return select_is_pressed; }
    EntityFacing getJoystickDirection() const { return joystickDirection; }

    char* get_informations(); // Return a string representation of the current inputs state, for debugging purposes.
};

