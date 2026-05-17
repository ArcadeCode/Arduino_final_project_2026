#include "ghost.hpp"

// Initialization list which is obligatory for constants.
Ghost::Ghost(GameState* state, GhostPersonality personality)
    : state(state), personality(personality), mode(GM_Scatter), lastFacing(EF_NORTH) {}

void Ghost::moveRandom() {
    EntityFacing opposite = getOpposite(this->lastFacing);
    EntityFacing valid[3]; // max 3 valid directions (cannot go back on itself)
    uint8_t count = 0;

    // Check all 4 directions
    EntityFacing dirs[4] = {EF_NORTH, EF_EAST, EF_SOUTH, EF_WEST};
    for (uint8_t i = 0; i < 4; i++) {
        if (dirs[i] == opposite) continue;
        if (isWalkable(this->state, getNeighbor(this->position, dirs[i]))) {
            valid[count++] = dirs[i];
        }
    }

    if (count == 0) return; // Dead-end, should not happen in a well-designed maze, but just in case...

    EntityFacing chosen = valid[random(count)]; // random() Arduino function
    this->lastFacing = chosen;
    this->position = getNeighbor(this->position, chosen);
}

void Ghost::moveTowardTarget() {
    EntityFacing opposite = getOpposite(this->lastFacing);
    EntityFacing dirs[4] = {EF_NORTH, EF_EAST, EF_SOUTH, EF_WEST};

    uint16_t bestDist = UINT16_MAX;
    EntityFacing bestDir = this->lastFacing;

    for (uint8_t i = 0; i < 4; i++) {
        if (dirs[i] == opposite) continue;
        
        GridPosition next = getNeighbor(this->position, dirs[i]);
        if (!isWalkable(this->state, next)) continue;

        uint16_t dist = squaredDistance(next, this->target);
        if (dist < bestDist) {
            bestDist = dist;
            bestDir = dirs[i];
        }
    }

    this->lastFacing = bestDir;
    this->position = getNeighbor(this->position, bestDir);
}

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
}

void Ghost::computeChaseTarget() {
    switch (this->personality) {
    
        case GP_RED: {
            // RED follow the current position of pacman.
            this->target = this->state->pacmanPosition;
            break;
        }
        
        case GP_PINK: {
            // PINK follow a position 4 cells ahead of pacman.
            GridPosition ahead = this->state->pacmanPosition;
            EntityFacing facing = this->state->pacmanFacing;
            
            for (uint8_t i = 0; i < 4; i++) {
                ahead = getNeighbor(ahead, facing);
            }
            
            // Original bug in the arcade version, if pacman is facing up, pink target is not 4 cells ahead but 4 cells up + 4 cells left. 
            if (facing == EF_NORTH) {
                for (uint8_t i = 0; i < 4; i++) {
                    ahead = getNeighbor(ahead, EF_WEST);
                }
            }
            
            this->target = ahead;
            break;
        }
        
        case GP_BLUE: {
            // BLUE have a more complex behavior, he use the position of pacman and red ghost to compute his target.
            // 1.Intermediate point, 2 cases ahead of Pac-Man
            GridPosition pivot = this->state->pacmanPosition;
            EntityFacing facing = this->state->pacmanFacing;
            
            for (uint8_t i = 0; i < 2; i++) pivot = getNeighbor(pivot, facing);
            
            // Original bug (same as Pink)
            if (facing == EF_NORTH) {
                for (uint8_t i = 0; i < 2; i++) pivot = getNeighbor(pivot, EF_WEST);
            }
            
            // 2. RED vector → pivot, and extend it twice to get the final target.
            GridPosition red = this->state->redGhostPosition;
            this->target.x = pivot.x + (pivot.x - red.x);
            this->target.y = pivot.y + (pivot.y - red.y);
            break;
        }

        case GP_ORANGE: {
            // ORANGE switch between two behaviors based on his distance to pacman, if he's far he follow pacman like red, if he's near he switch to scatter mode target and run away to his corner.
            // 8*8 = 64 to avoid sqrt calculation.
            if (squaredDistance(this->position, this->state->pacmanPosition) > 64) {
                this->target = this->state->pacmanPosition; // Same as RED
            } else {
                this->target = ORANGE_SCATTER_MODE_TARGET; // Back to his corner
            }
            break;
        }
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
}


void Ghost::computeNewPosition() {
    if (this->mode == GM_Frightened) {
        this->moveRandom();
    } else {
        this->computeNewTarget();
        this->moveTowardTarget();
    }
}

void Ghost::setPosition(GridPosition pos) {
    this->position = pos;
}

void Ghost::setFacing(EntityFacing facing) {
    this->lastFacing = facing;
}

void Ghost::setAiMode(GhostAiMode newMode) {
    this->mode = newMode;
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