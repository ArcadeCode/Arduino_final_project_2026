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
    Ghost ghosts[GHOSTS_COUNT]; // 0: Red, 1: Pink, 2: Blue, 3: Orange

    void computePacmanPosition();

    /**
     * @brief Drive the global Scatter/Chase phase schedule.
     * Does NOT touch ghosts that are currently in Frightened mode —
     * those track their own timer inside the Ghost class.
     */
    void updateGhostModes();

    /**
     * @brief Trigger Frightened mode on all ghosts (called when Pac-Man eats an energizer).
     */
    void triggerFrightenedAll();

public:
    Game(/* args */);
    ~Game();

    void start();         /* Start / reset a new game */
    GameState& step();    /* Advance one tick */

    // Getters
    GridPosition get_pacmanPosition() const { return pacmanPosition; }
    EntityFacing get_pacmanFacing()   const { return pacmanFacing; }
    GameState&   getState()                 { return this->state; }
    const Ghost* getGhosts() const { return this->ghosts; }

    char* get_ghostInformations(uint8_t index) const { return ghosts[index].getGhostInformations(); }
};