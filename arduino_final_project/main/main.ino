#include "game.hpp"
#include "inputs.hpp"
#include "screen.hpp"

#include <Arduino.h>

int freeMemory() {
  extern char *__brkval;
  extern char __heap_start;

  char top;
  
  return &top - (__brkval == 0 ? &__heap_start : __brkval);
}

// We done that for heap allocation
Game* game = nullptr; // Variable globale
Screen* screen = nullptr;

void setup() {
    Serial.begin(19200);
    delay(2000);

    Serial.println("BOOT OK");

    game = new Game();
    Serial.println("Game allocated");


    game->start();
    Serial.println("Game started");

    game->loadLevel(0);

    screen = new Screen();
    Serial.println("Screen allocated");
}

void loop() {
    // 1. Calculating new game state
    gameState& state = game->step();

    // 2. Sending state to the output Screen
    screen->print_frame(state);

    // 2.1 Debuging informations
    Serial.print("Tick : ");
    Serial.println(state.tick);

    Serial.print("Pacman position : (");
    Serial.print(game->get_pacmanPosition().x);
    Serial.print(", ");
    Serial.print(game->get_pacmanPosition().y);
    Serial.println(")");
    Serial.print("Blue Ghost position : (");
    Serial.print(game->get_blueGhostPosition().x);
    Serial.print(", ");
    Serial.print(game->get_blueGhostPosition().y);
    Serial.println(")");
    Serial.print("Red Ghost position : (");
    Serial.print(game->get_redGhostPosition().x);
    Serial.print(", ");
    Serial.print(game->get_redGhostPosition().y);
    Serial.println(")");
    Serial.print("Pink Ghost position : (");
    Serial.print(game->get_pinkGhostPosition().x);
    Serial.print(", ");
    Serial.print(game->get_pinkGhostPosition().y);
    Serial.println(")");
    Serial.print("Orange Ghost position : (");
    Serial.print(game->get_orangeGhostPosition().x);
    Serial.print(", ");
    Serial.print(game->get_orangeGhostPosition().y);
    Serial.println(")");
    Serial.print("Free RAM : ");
    Serial.print(freeMemory());
    Serial.println(" bytes");
    Serial.flush();

    delay(500);

    // We register his inputs directly into the Game
    // object
    inputs newInputs = inputs(); // Default constructor will retrieve the current inputs state
    game->registerInputs(newInputs);
}