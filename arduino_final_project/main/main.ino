#include "game.cpp"
#include "inputs.cpp"
#include "outputs.cpp"

void setup() {
    Game game = Game();
    Screen screen = Screen();
    game.start();
}

void loop() {
    // 1. Calculating new game step
    gameState state = game.step();

    // 2. 
    screen.print_frame(state)


    game.registerInputs(
        get_joystick_position(),
        start_btn_is_pressed(),
        select_btn_is_pressed
    )
}