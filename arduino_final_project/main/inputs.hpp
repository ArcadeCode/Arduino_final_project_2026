#pragma once

/**
 * This file store all inputs gate to the game.
 */

/* Represent the result output of a joystick */
struct joystickPosition {
    float x; // x is the left - right value
    float y; // y is the top - bottom value
};

/* Retrieve joystick position */
joystickPosition get_joystick_position();

/* Return if the start button is pressed */
bool start_btn_is_pressed();

/* Return if the select button is pressed */
bool select_btn_is_pressed();


/* Represent all inputs */
struct inputs {
    joystickPosition joystick;
    bool start_is_pressed;
    bool select_is_pressed;

    inputs() : joystick{get_joystick_position()}, start_is_pressed(start_btn_is_pressed()), select_is_pressed(select_btn_is_pressed()) {}
};