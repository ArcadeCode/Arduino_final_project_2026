#pragma once
#include "inputs.hpp"
#include "types.hpp"
#include "ghost.hpp"

static uint8_t lfsrRandomDirection();

class Game {
private:
    GameState state;
    inputs currentInputs;

    // Helpers for computing
    int moveEntity(CellEntitiesType Entity, uint8_t old_x, uint8_t old_y, uint8_t new_x, uint8_t new_y);
    GridPosition pacmanPosition;
    EntityFacing pacmanFacing;

    Ghost blueGhost;
    Ghost redGhost;
    Ghost pinkGhost;
    Ghost orangeGhost;

    void computePacmanPosition();
    // Ghosts positions are directly computed in their class and shared to the global state for easier access to ghosts AI movement, instead of searching the grid for them.

    void updateGhostModes();

    public:
    Game(/* args */);
    ~Game();

    void start(); /* Start a new game */
    GameState& step(); /* Step from one new frame */
    void registerInputs(inputs &newInputs);

    // Getters for entities positions
    GridPosition get_pacmanPosition() const { return pacmanPosition; }
    EntityFacing get_pacmanFacing() const { return pacmanFacing; }

    GameState& getState() { return this->state; } // Get the current state without stepping.

    char* get_blueGhostInformation() const { return blueGhost.getGhostInformations(); }
    char* get_redGhostInformation() const { return redGhost.getGhostInformations(); }
    char* get_pinkGhostInformation() const { return pinkGhost.getGhostInformations(); }
    char* get_orangeGhostInformation() const { return orangeGhost.getGhostInformations(); }
};