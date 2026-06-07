#include "ghost.hpp"

// ─── Constructor ─────────────────────────────────────────────────────────────

Ghost::Ghost(GameState* state, GhostPersonality personality)
    : state(state),
      personality(personality),
      mode(GM_Scatter),
      modeBeforeFrightened(GM_Scatter),
      houseState(GS_IN_HOUSE),
      lastFacing(EF_NORTH),
      frightenedStartMs(0),
      frightenedDurationMs(0)
{
    switch (personality) {
        case GP_RED:
            // Blinky starts outside; mark him as immediately normal.
            this->dotThreshold = 0;
            this->houseState   = GS_NORMAL;
            break;
        case GP_PINK:
            // Pinky exits as soon as Blinky has moved (threshold = 0 dots eaten).
            this->dotThreshold = 0;
            break;
        case GP_BLUE:
            this->dotThreshold = 35; // ~30-40 % of dots eaten.
            break;
        case GP_ORANGE:
            this->dotThreshold = 33; // >1/3 of dots eaten.
            break;
    }
}

// ─── Dot threshold ───────────────────────────────────────────────────────────

bool Ghost::isDotThresholdReached() const {
    uint16_t dotsEaten  = this->state->totalDots - this->state->remainingDots;
    uint16_t threshold  = (static_cast<uint16_t>(this->state->totalDots) * this->dotThreshold) / 100;
    return dotsEaten >= threshold;
}

// ─── Frightened helpers ───────────────────────────────────────────────────────

uint16_t Ghost::getFrightenedDurationMs() const {
    uint8_t levelIndex = (this->state->level == 1) ? 0 :
                         (this->state->level <= 4) ? 1 : 2;
    uint8_t secs = pgm_read_byte(&FRIGHTENED_DURATIONS[levelIndex]);
    return (uint16_t)secs * 1000u;
}

void Ghost::triggerFrightened() {
    if (this->mode != GM_Frightened) {
        this->modeBeforeFrightened = this->mode;
    }
    this->mode                 = GM_Frightened;
    this->frightenedStartMs    = millis();
    this->frightenedDurationMs = getFrightenedDurationMs();
}

bool Ghost::updateFrightened() {
    if (this->mode != GM_Frightened) return false;

    unsigned long elapsed = millis() - this->frightenedStartMs;
    if (elapsed >= this->frightenedDurationMs) {
        this->mode = this->modeBeforeFrightened;
        // Plays the eye retreat sound if no other ghost is Frightened.
        // Note: we let Game handle it to avoid multiple calls.
        // Simplified call here, the priority system handles duplicates.
        AudioEngine::play(SFX_GHOST_RETREAT);
        return false;
    }

    return true; // Still frightened.
}

bool Ghost::isBlinking() const {
    if (this->mode != GM_Frightened) return false;
    unsigned long elapsed   = millis() - this->frightenedStartMs;
    unsigned long remaining = this->frightenedDurationMs - elapsed;
    // Blink every 250 ms during the last FRIGHTENED_BLINK_TICKS worth of time.
    // We use remaining < 2000 ms as the trigger window, toggling at 250 ms.
    if (remaining > 2000UL) return false;
    return ((remaining / 250UL) % 2) == 0;
}

// ─── Movement helpers ─────────────────────────────────────────────────────────

void Ghost::moveRandom() {
    EntityFacing opposite = getOpposite(this->lastFacing);
    EntityFacing valid[3];
    uint8_t count = 0;

    EntityFacing dirs[4] = {EF_NORTH, EF_EAST, EF_SOUTH, EF_WEST};
    for (uint8_t i = 0; i < 4; i++) {
        if (dirs[i] == opposite) continue;
        if (isWalkable(this->state, getNeighbor(this->position, dirs[i]))) {
            valid[count++] = dirs[i];
        }
    }

    if (count == 0) return;

    EntityFacing chosen = valid[random(count)];
    this->lastFacing = chosen;
    this->position   = getNeighbor(this->position, chosen);
}

void Ghost::moveTowardTarget() {
    EntityFacing opposite = getOpposite(this->lastFacing);
    EntityFacing dirs[4]  = {EF_NORTH, EF_EAST, EF_SOUTH, EF_WEST};

    uint16_t     bestDist = UINT16_MAX;
    EntityFacing bestDir  = this->lastFacing;
    bool         found    = false;

    for (uint8_t i = 0; i < 4; i++) {
        if (dirs[i] == opposite) continue;
        GridPosition next = getNeighbor(this->position, dirs[i]);
        if (!isWalkable(this->state, next)) continue;

        uint16_t dist = squaredDistance(next, this->target);
        if (dist < bestDist) {
            bestDist = dist;
            bestDir  = dirs[i];
            found    = true;
        }
    }

    if (!found) return;
    this->lastFacing = bestDir;
    this->position   = getNeighbor(this->position, bestDir);
}

/**
 * @brief Move one step toward the ghost-house exit tile {GHOST_HOUSE_EXIT_X, GHOST_HOUSE_EXIT_Y}.
 *
 * Strategy: move vertically toward the exit row first, then horizontally toward the
 * exit column.  We bypass isWalkable() for wall checks because the ghost house
 * interior tiles may be marked as walls in the background data — the ghost is
 * allowed to traverse them while exiting.
 */
void Ghost::moveTowardExit() {
    const uint8_t exitX = GHOST_HOUSE_EXIT_X;
    const uint8_t exitY = GHOST_HOUSE_EXIT_Y;

    if (this->position.y > exitY) {
        // Move up.
        this->lastFacing = EF_NORTH;
        this->position.y--;
    } else if (this->position.x < exitX) {
        // Move right.
        this->lastFacing = EF_EAST;
        this->position.x++;
    } else if (this->position.x > exitX) {
        // Move left.
        this->lastFacing = EF_WEST;
        this->position.x--;
    } else {
        // Reached exit column and row — transition to normal play.
        this->houseState = GS_NORMAL;
        // Give an initial facing so the ghost immediately enters the maze.
        this->lastFacing = EF_WEST;
    }
}

// ─── Target computation ───────────────────────────────────────────────────────

void Ghost::computeScatterTarget() {
    switch (this->personality) {
        case GP_RED:    this->target = RED_SCATTER_MODE_TARGET;    break;
        case GP_PINK:   this->target = PINK_SCATTER_MODE_TARGET;   break;
        case GP_BLUE:   this->target = BLUE_SCATTER_MODE_TARGET;   break;
        case GP_ORANGE: this->target = ORANGE_SCATTER_MODE_TARGET; break;
        default:        this->target = GridPosition(0, 0);         break;
    }
}

void Ghost::computeChaseTarget() {
    switch (this->personality) {

        case GP_RED: {
            // RED targets Pac-Man's current tile.
            this->target = this->state->pacmanPosition;
            break;
        }

        case GP_PINK: {
            // PINK targets 4 tiles ahead of Pac-Man.
            GridPosition ahead  = this->state->pacmanPosition;
            EntityFacing facing = this->state->pacmanFacing;
            for (uint8_t i = 0; i < 4; i++) ahead = getNeighbor(ahead, facing);
            // Original arcade overflow bug: facing up → also shift 4 left.
            if (facing == EF_NORTH) {
                for (uint8_t i = 0; i < 4; i++) ahead = getNeighbor(ahead, EF_WEST);
            }
            this->target = ahead;
            break;
        }

        case GP_BLUE: {
            // BLUE: pivot = 2 tiles ahead of Pac-Man; target = mirror of Red through pivot.
            GridPosition pivot  = this->state->pacmanPosition;
            EntityFacing facing = this->state->pacmanFacing;
            for (uint8_t i = 0; i < 2; i++) pivot = getNeighbor(pivot, facing);
            if (facing == EF_NORTH) {
                for (uint8_t i = 0; i < 2; i++) pivot = getNeighbor(pivot, EF_WEST);
            }
            GridPosition red = this->state->ghostPositions[0]; // Red is index 0.
            int16_t tx = (int16_t)pivot.x + ((int16_t)pivot.x - (int16_t)red.x);
            int16_t ty = (int16_t)pivot.y + ((int16_t)pivot.y - (int16_t)red.y);
            if (tx < 0)          tx = 0;
            else if (tx > 255)   tx = 255;
            if (ty < 0)          ty = 0;
            else if (ty > 255)   ty = 255;
            this->target.x = (uint8_t)tx;
            this->target.y = (uint8_t)ty;
            break;
        }

        case GP_ORANGE: {
            // ORANGE: chase like Red when far (>8 tiles), scatter corner when close.
            if (squaredDistance(this->position, this->state->pacmanPosition) > 64) {
                this->target = this->state->pacmanPosition;
            } else {
                this->target = ORANGE_SCATTER_MODE_TARGET;
            }
            break;
        }
    }
}

void Ghost::computeNewTarget() {
    switch (this->mode) {
        case GM_Chase:   this->computeChaseTarget();   break;
        case GM_Scatter: this->computeScatterTarget(); break;
        default:         this->target = {0, 0};        break;
    }
}

// ─── Main per-tick update ─────────────────────────────────────────────────────

void Ghost::computeNewPosition() {
    // 1. Update frightened timer — may restore previous mode.
    updateFrightened();

    // 2. Handle house exit state machine.
    switch (this->houseState) {
        case GS_IN_HOUSE:
            if (isDotThresholdReached()) {
                this->houseState = GS_EXITING;
                // Fall through to start moving immediately.
            } else {
                return; // Stay put.
            }
            // fall through
        case GS_EXITING:
            moveTowardExit();
            return; // Do not apply normal AI while exiting.

        case GS_NORMAL:
            break; // Continue with normal AI below.
    }

    // 3. Normal maze movement.
    if (this->mode == GM_Frightened) {
        moveRandom();
    } else {
        computeNewTarget();
        moveTowardTarget();
    }
}

// ─── Setters ─────────────────────────────────────────────────────────────────

void Ghost::setPosition(GridPosition pos) { this->position = pos; }
void Ghost::setFacing(EntityFacing facing) { this->lastFacing = facing; }

void Ghost::setAiMode(GhostAiMode newMode) {
    // Never override an active Frightened mode via the normal scheduler.
    if (this->mode == GM_Frightened) {
        // Remember what we should return to once frightened ends.
        this->modeBeforeFrightened = newMode;
        return;
    }
    this->mode = newMode;
}

// ─── Debug string ─────────────────────────────────────────────────────────────

static char ghostInfo[80];

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
        case GM_Scatter:    modeStr = "Scatter";    break;
        case GM_Frightened: modeStr = "Frightened"; break;
        default:            modeStr = "???";        break;
    }

    const char* houseStr;
    switch (this->houseState) {
        case GS_IN_HOUSE: houseStr = "House";   break;
        case GS_EXITING:  houseStr = "Exiting"; break;
        case GS_NORMAL:   houseStr = "Normal";  break;
        default:          houseStr = "???";     break;
    }

    sprintf(ghostInfo, "%s (%d,%d) [%c] %s tgt(%d,%d) %s blink:%s",
        colorStr,
        this->position.x, this->position.y,
        facingChar,
        modeStr,
        this->target.x, this->target.y,
        houseStr,
        isBlinking() ? "Y" : "N");

    return ghostInfo;
}