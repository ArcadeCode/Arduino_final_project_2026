#pragma once
#include "inputs.hpp"
#include "types.hpp"
#include "ghost.hpp"

static uint8_t lfsrRandomDirection();

class Game {
private:
    gameState state;
    inputs currentInputs;

    // Helpers for computing
    int moveEntity(CellEntitiesType Entity, uint8_t old_x, uint8_t old_y, uint8_t new_x, uint8_t new_y);
    GridPosition pacmanPosition;
    EntityFacing pacmanFacing;

    Ghost blueGhost;
    Ghost redGhost;
    Ghost pinkGhost;
    Ghost orangeGhost;

    // Computing functions called each step
    void computePacmanPosition();
    void computeBlueGhostPosition();
    void computeRedGhostPosition();
    void computePinkGhostPosition();
    void computeOrangeGhostPosition();

public:
    Game(/* args */);
    ~Game();

    void start(); /* Start a new game */
    gameState& step(); /* Step from one new frame */
    void registerInputs(inputs &newInputs);

    // Level information retrieve
    void loadLevel(char level);

    // Getters for entities positions
    GridPosition get_pacmanPosition() const { return pacmanPosition; }
    EntityFacing get_pacmanFacing() const { return pacmanFacing; }

    char* get_blueGhostInformation() const { return blueGhost.getGhostInformations(); }
    char* get_redGhostInformation() const { return redGhost.getGhostInformations(); }
    char* get_pinkGhostInformation() const { return pinkGhost.getGhostInformations(); }
    char* get_orangeGhostInformation() const { return orangeGhost.getGhostInformations(); }
};