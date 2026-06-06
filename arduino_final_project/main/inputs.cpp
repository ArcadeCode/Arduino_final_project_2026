#include "inputs.hpp"

Inputs::Inputs(GameState* state) : state(state), joystickDirection(EF_NORTH), start_is_pressed(false), select_is_pressed(false) {
    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);

    #if USE_JOYSTICK == 0
    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(JOYSTICK_Y_PIN, INPUT);
    #endif
    #if USE_JOYSTICK == 1
    pinMode(DIRECTION_LEFT, INPUT);
    pinMode(DIRECTION_RIGHT, INPUT);
    pinMode(DIRECTION_UP, INPUT);
    pinMode(DIRECTION_DOWN, INPUT);
    #endif
}

void Inputs::read_start_button() {
    this->start_is_pressed = digitalRead(START_BUTTON_PIN) == LOW;
}

void Inputs::read_select_button() {
    this->select_is_pressed = digitalRead(SELECT_BUTTON_PIN) == LOW;
}

void Inputs::read_joystick() {
    #if USE_JOYSTICK == 0
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
    #endif
    
    #if USE_JOYSTICK == 1
    if (digitalRead(DIRECTION_LEFT) == LOW) {
        this->joystickDirection = EF_WEST;
    } else if (digitalRead(DIRECTION_RIGHT) == LOW) {
        this->joystickDirection = EF_EAST;
    } else if (digitalRead(DIRECTION_UP) == LOW) {
        this->joystickDirection = EF_NORTH;
    } else if (digitalRead(DIRECTION_DOWN) == LOW) {
        this->joystickDirection = EF_SOUTH;
    }
    #endif
    
    return this->joystickDirection;
}

void Inputs::update() {
    EntityFacing oldDirection = this->joystickDirection;

    read_start_button();
    read_select_button();
    read_joystick();

    // Lazy update
    if (oldDirection != this->joystickDirection) {
        this->state->pacmanFacing = this->joystickDirection;
    }
}
    // TODO: Implement start & select buttons

char* Inputs::get_informations() {
    static char debugString[50];
    const char* directionStr = "";

    switch (this->joystickDirection) {
        case EF_NORTH: directionStr = "North"; break;
        case EF_SOUTH: directionStr = "South"; break;
        case EF_EAST:  directionStr = "East";  break;
        case EF_WEST:  directionStr = "West";  break;
        default:       directionStr = "Unknown";
    }

    snprintf(debugString, sizeof(debugString), "Start: %s, Select: %s, Joystick: %s",
             this->start_is_pressed ? "Pressed" : "Released",
             this->select_is_pressed ? "Pressed" : "Released",
             directionStr);
    return debugString;
}