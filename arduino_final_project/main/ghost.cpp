#include "ghost.hpp"

char* Ghost::getGhostInformations() {
    static char info[40];
    sprintf(info, "(%d, %d) Mode: %d", this->position.x, this->position.y, this->mode);
    return info;
}

void Ghost::setPosition(GridPosition pos) {
    this->position = pos;
}

// initialization list which is obligatory for constants.
Ghost::Ghost(gameState* state, GhostPersonality personality)
    : state(state), personality(personality) {}

GridPosition Ghost::computeScatterTarget() {
    switch (this->personality) {
    case GP_RED:
        return RED_SCATTER_MODE_TARGET;
        break;
    
    case GP_PINK:
        return PINK_SCATTER_MODE_TARGET;
        break;

    case GP_BLUE:
        return BLUE_SCATTER_MODE_TARGET;
        break;

    case GP_ORANGE:
        return ORANGE_SCATTER_MODE_TARGET;
        break;

    default:
        return GridPosition(0, 0);
        break;
    }
};

GridPosition Ghost::computeChaseTarget() {
    switch (this->personality) {
    case GP_RED:
        return RED_SCATTER_MODE_TARGET;
        break;
    
    case GP_PINK:
        return PINK_SCATTER_MODE_TARGET;
        break;

    case GP_BLUE:
        return BLUE_SCATTER_MODE_TARGET;
        break;

    case GP_ORANGE:
        return ORANGE_SCATTER_MODE_TARGET;
        break;

    default:
        return GridPosition(0, 0);
        break;
    }
}

GridPosition Ghost::computeNewTarget() {
    switch(this->mode) {
        case GM_Chase:
            return computeChaseTarget();
        case GM_Scatter:
            return computeScatterTarget();
        default:
            return GridPosition(0, 0);
    }
}

bool Ghost::isPositionInIntersection(GridPosition pos) {
    uint8_t ways = 0;
    if (this->state->grid[pos.y+1][pos.x].getBackground() != BG_WALL) {
        ways += 1;
    } else if (this->state->grid[pos.y-1][pos.x].getBackground() != BG_WALL) {
        ways += 1;
    } else if (this->state->grid[pos.y][pos.x+1].getBackground() != BG_WALL) {
        ways += 1;
    } else if (this->state->grid[pos.y][pos.x-1].getBackground() != BG_WALL) {
        ways += 1;
    }

    // We check if there is 3 ways or more it no more a corridor.
    if (ways > 2) {
        return true;
    } else {
        return false;
    }
    
};

GridPosition Ghost::computeNewPosition() {
    // Check if we are in dummy mode :
    if (this->mode == GM_Frightened) {
        // In Frightened, there is none target.

        // 1. Search further intersection
        if (!this->isPositionInIntersection(this->position)) {
            return GridPosition(this->position.x, this->position.y);
        }
           
        // 2. Take a random direction
    } else { 
        // If we are in SCATTER or CHASE mode...
        GridPosition target = computeNewTarget();

        switch (this->personality) {
        case GP_RED:
            return RED_SCATTER_MODE_TARGET;
            break;
        
        case GP_PINK:
            return PINK_SCATTER_MODE_TARGET;
            break;

        case GP_BLUE:
            return BLUE_SCATTER_MODE_TARGET;
            break;

        case GP_ORANGE:
            return ORANGE_SCATTER_MODE_TARGET;
            break;

        default:
            break;
        }
    }
};