#pragma once
#include "inputs.hpp"
#include "types.hpp"
#include "ghost.hpp"

class Game {
private:
    GameState state;

    // Pacman
    GridPosition pacmanPosition;
    EntityFacing pacmanFacing;
    
    // Ghosts
    Ghost ghosts[GHOSTS_COUNT]; // 0: Blue, 1: Red, 2: Pink, 3: Orange

    void computePacmanPosition();
    // Ghosts positions are directly computed in their class and shared to the global state for easier access to ghosts AI movement, instead of searching the grid for them.

    // Check if the ghosts can exit the ghost house based on their dot threshold and the remaining dots count.
    void updateGhostModes();

    public:
    Game(/* args */);
    ~Game();

    void start(); /* Start a new game */
    GameState& step(); /* Step from one new frame */

    // Getters for entities positions
    GridPosition get_pacmanPosition() const { return pacmanPosition; }
    EntityFacing get_pacmanFacing() const { return pacmanFacing; }

    GameState& getState() { return this->state; } // Get the current state without stepping. (used by levels.hpp)

    char* get_ghostInformations(uint8_t index) const { return ghosts[index].getGhostInformations(); }
};