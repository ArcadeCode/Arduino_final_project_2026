#include "game.hpp"

Game::Game(/* args */) {};
Game::~Game() = default;

void Game::start() {
    this->state = gameState();
};

gameState& Game::step() {
    this->state.tick++;

    // Compute new positions for all entities
    

    // In the end return the new state to be printed on the screen
    return this->state;
};

void Game::registerInputs(inputs &newInputs) {
    this->currentInputs = newInputs;
};

void Game::loadLevel(uint8_t level) {
    //
};