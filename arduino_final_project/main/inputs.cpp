#include "inputs.hpp"

Inputs::Inputs() : joystickDirection(EF_NORTH), start_is_pressed(false), select_is_pressed(false) {
    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(JOYSTICK_Y_PIN, INPUT);
}

void Inputs::read_start_button() {
    this->start_is_pressed = digitalRead(START_BUTTON_PIN) == LOW;
}

void Inputs::read_select_button() {
    this->select_is_pressed = digitalRead(SELECT_BUTTON_PIN) == LOW;
}

void Inputs::read_joystick() {
    int xValue = analogRead(JOYSTICK_X_PIN) - JOYSTICK_MAX_VALUE; // Centered at 0
    int yValue = analogRead(JOYSTICK_Y_PIN) - JOYSTICK_MAX_VALUE; // Centered at 0

    float xNorm = static_cast<float>(xValue) / JOYSTICK_MAX_VALUE;
    float yNorm = static_cast<float>(yValue) / JOYSTICK_MAX_VALUE;

    if (sqrt(xNorm * xNorm + yNorm * yNorm) < JOYSTICK_DEADZONE) {
        return joystickDirection; // No change if within deadzone
    }

    if (abs(xNorm) > abs(yNorm)) {
        joystickDirection = (xNorm > 0) ? EF_EAST : EF_WEST;
    } else {
        joystickDirection = (yNorm > 0) ? EF_SOUTH : EF_NORTH;
    }

    this->joystickDirection = joystickDirection;
}

void Inputs::update() {
    read_start_button();
    read_select_button();
    read_joystick();
    
    this->state.pacmanFacing = this->joystickDirection;

    // TODO: Implement start & select buttons
}

char* Inputs::get_informations() {
    static char debugString[50];
    snprintf(debugString, sizeof(debugString), "Start: %s, Select: %s, Joystick: %d",
             start_is_pressed ? "Pressed" : "Released",
             select_is_pressed ? "Pressed" : "Released",
             joystickDirection);
    return debugString;
}