#include "ghost.hpp"

// Initialization list which is obligatory for constants.
Ghost::Ghost(GameState* state, GhostPersonality personality)
    : state(state), personality(personality), mode(GM_Scatter), lastFacing(EF_NORTH) {}

// In Scatter mode, each ghost has a fixed target tile, each of which is located just outside a different corner of the maze. 
void Ghost::computeScatterTarget() {
    switch (this->personality) {
    case GP_RED:
        this->target = RED_SCATTER_MODE_TARGET;
        break;
    
    case GP_PINK:
        this->target = PINK_SCATTER_MODE_TARGET;
        break;

    case GP_BLUE:
        this->target = BLUE_SCATTER_MODE_TARGET;
        break;

    case GP_ORANGE:
        this->target = ORANGE_SCATTER_MODE_TARGET;
        break;

    default:
        this->target = GridPosition(0, 0);
        break;
    }
};

void Ghost::computeChaseTarget() {
    switch (this->personality) {
    
    case GP_RED:
        // Red always target the current pacman position.
        break;
    
    case GP_PINK:
        // Pink alway target 4 tiles ahead of pacman, but due to a bug in the original game, when pacman is facing up, pink target is 4 tiles ahead and 4 tiles to the left of pacman.
        break;

    case GP_BLUE:
        //
        break;

    case GP_ORANGE:
        //
        break;

    default:
        this->target = {0, 0}; // Default target, this will help us to check errors
        break;
    }
}

/*
This function is called when AI mode is Chase or Scatter.
Frightened mode is based on random movement and don't have a target.
*/
void Ghost::computeNewTarget() {
    switch(this->mode) {
        case GM_Chase:
            this->computeChaseTarget();
            break;
        case GM_Scatter:
            this->computeScatterTarget();
            break;
        default:
            this->target =  {0, 0};
            break;
    }
};

void Ghost::computeNewPosition() {
    // Check if we are in dummy mode :
    if (this->mode == GM_Frightened) {
        // In Frightened, there is none target.

        // 1. Search further intersection
           
        // 2. Take a random direction
    } else { 
        // If we are in SCATTER or CHASE mode...
        this->computeNewTarget();
        
        // TODO: Implement the pathfinding
    }
};

void Ghost::setPosition(GridPosition pos) {
    this->position = pos;
}

void Ghost::setFacing(EntityFacing facing) {
    this->lastFacing = facing;
}

// Shared buffer across all Ghost instances, safe as long as getGhostInformations() result is consumed before the next call.
static char ghostInfo[64];

char* Ghost::getGhostInformations() {
    const char* colorStr;
    switch (this->personality) {
        case GP_RED:    colorStr = "Red";    break;
        case GP_BLUE:   colorStr = "Blue";   break;
        case GP_PINK:   colorStr = "Pink";   break;
        case GP_ORANGE: colorStr = "Orange"; break;
        default:        colorStr = "???";    break;
    }

    char facingChar = entityFacingToChar(this->lastFacing);

    const char* modeStr;
    switch (this->mode) {
        case GM_Chase:      modeStr = "Chase";      break;
        case GM_Scatter:    modeStr = "Scatter";     break;
        case GM_Frightened: modeStr = "Frightened";  break;
        default:            modeStr = "???";         break;
    }

    sprintf(ghostInfo, "%s ghost (%d, %d) facing: [%c] mode: %s target (%d, %d)",
        colorStr, this->position.x, this->position.y, facingChar, modeStr, this->target.x, this->target.y);

    return ghostInfo;
}