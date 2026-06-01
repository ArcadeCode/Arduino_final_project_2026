/*
ghost.hpp contain all the Ghost logic which is a very large part of the game.
All behavior of each Ghosts were learn by reading this Steam Community Guide :
https://steamcommunity.com/sharedfiles/filedetails/?id=593226813
For take his source from
https://cs.au.dk/~ocaprani/GameAI/PacMan/The%20Pac-Man%20Dossier.pdf

Thank to Night Druid for this guide and Jamey Pittman for his work !
*/

#pragma once
#include "types.hpp"

// See https://steamcommunity.com/sharedfiles/filedetails/?id=593226813 for more informations about scatter mode target.
static constexpr GridPosition PINK_SCATTER_MODE_TARGET   = {.x = 2,                    .y = 0};
static constexpr GridPosition RED_SCATTER_MODE_TARGET    = {.x = GAME_GRID_X_AXIS_LEN-2, .y = 0};
static constexpr GridPosition ORANGE_SCATTER_MODE_TARGET = {.x = 0,                    .y = GAME_GRID_Y_AXIS_LEN};
static constexpr GridPosition BLUE_SCATTER_MODE_TARGET   = {.x = GAME_GRID_X_AXIS_LEN, .y = GAME_GRID_Y_AXIS_LEN};

// Ghost house exit column (the door tile x, just above the ghost house).
static constexpr uint8_t GHOST_HOUSE_EXIT_X = 14;
static constexpr uint8_t GHOST_HOUSE_EXIT_Y = 11; // First walkable row above the house door.

// Frightened mode duration in seconds per level group (mirrors the MODE_DURATIONS layout).
// Index: 0 = level 1, 1 = levels 2-4, 2 = levels 5+.
static const uint8_t FRIGHTENED_DURATIONS[3] PROGMEM = {6, 5, 3};

// How many ticks before end of frightened the ghost starts blinking (at ~1 tick/step).
static constexpr uint8_t FRIGHTENED_BLINK_TICKS = 60; // ~2 s at 30 tps

// TODO: Bit masking GhostPersonality & GhostAiMode into 1 GhostComportment struct if optimization is needed.
enum GhostPersonality {
    GP_RED, GP_BLUE, GP_PINK, GP_ORANGE

    /*
    === GP_RED ===
    - The red ghost Blinky, starts outside of the ghost spawn room / house, and is usually the first one to be seen as a threat. 
    - Blinky's target tile in Chase mode is defined as Pac-Man's current location. 
    - Blinky's aggressive AI is based on the number of dots remaining, giving him increased speed by 5% while also changing his behavior in Scatter mode to be more like his chase mode.

    === GP_PINK ===
    - The pink ghost starts inside the ghost house, but she will always exit immediately after Blinky moves out of the entrance. 
    - Pinky moves at the same speed as Blinky and Inky.
    - Pinky's targeting scheme attempts to move her to the place where Pac-Man is going, instead of where he currently is. Her target tile in Chase mode is determined by looking at Pac-Man's current position and orientation, and selecting the location four tiles straight ahead of Pac-Man. 
    - While this was the intended result, it only really works when Pac-Man is facing to the left, down, or right. Unfortunately, when Pac-Man is facing upwards, an overflow error in the game's code causes Pinky's target tile to actually be set as four tiles ahead of Pac-Man and four tiles to the left of him. TODO: Implement this bug as a feature in our version.
    
    === GP_BLUE ===
    - The blue ghost is nicknamed Inky, and remains inside the ghost house for a short time on the first level, not joining the chase until Pac-Man has managed to consume at least 30-40 percent of the dots. 
    - He is the only one of the ghosts that uses a factor other than Pac-Man's position and orientation when determining his target tile.
    - During his target selection, Inky uses both Pac-Man's position and orientation as well as Blinky's position in his calculation. 
    - To locate Inky's target, we first start by selecting the position two tiles in front of Pac-Man in his current direction of travel, similar to Pinky's targeting method. From there, imagine drawing a vector from Blinky's position to this tile, and then doubling the length of the vector. The tile that this new, extended vector ends on will be Inky's actual target.
    - Though Inky is difficult to evade, his "two tiles in front of Pac-Man" calculation suffers from the same overflow error as Pinky's four-tile equivalent, If Pac-Man is heading upwards, the endpoint of the initial vector from Blinky (before doubling) will actually be two tiles up and two tiles left of Pac-Man. TODO: Implement this bug as a feature in our version.

    === GP_ORANGE ===
    - The orange ghost, "Clyde", is the last to leave the ghost house, and does not exit at all in the first level until over a third of the dots have been eaten. 
    - His targeting method can give the impression that he is just "doing his own thing", without concerning himself with Pac-Man at all. 
    - The unique feature of Clyde's targeting is that it has two separate modes which he constantly switches back and forth between, based on his proximity to Pac-Man. Whenever Clyde needs to determine his target tile, he first calculates his distance from Pac-Man. If he is farther than eight tiles away, his targeting is identical to Blinky's, using Pac-Man's current tile as his target.
    - As soon as his distance to Pac-Man becomes less than eight tiles, Clyde's target is set to the same tile as his fixed one in Scatter mode, just outside the bottom-left corner of the maze.
    - The combination of these two methods has the overall effect of Clyde alternating between coming directly towards Pac-Man, and then changing his mind and heading back to his corner whenever he gets too close to Pac-Man.
    
    */
};

enum GhostAiMode {
    GM_Chase, GM_Scatter, GM_Frightened

    /*
    The standard mode is Chase with the ghosts pursuing Pac-Man.
    While in Chase mode, all of the ghosts use Pac-Man's position
    as a factor in selecting their target tile, however for some
    ghosts, it is more significant than others.

    In Scatter mode, each ghost has a fixed target tile, each of
    which is located just outside a different corner of the maze.
    This causes the four ghosts to retreat to the corners whenever
    they are in this mode.

    Frightened mode is the most unique mode, due to the fact that
    the ghosts do not have a specific target tile. Instead, they
    randomly decide which turns to make at every intersection making
    it difficult to catch them all.

    Source: https://steamcommunity.com/sharedfiles/filedetails/?id=593226813
    */
};

/**
 * @brief Internal state machine for a ghost leaving the house.
 * 
 * GS_IN_HOUSE   – ghost is inside and waiting for its dot threshold.
 * GS_EXITING    – threshold reached; ghost moves toward the exit tile.
 * GS_NORMAL     – ghost is on the main maze and follows AI rules.
 */
enum GhostHouseState {
    GS_IN_HOUSE,
    GS_EXITING,
    GS_NORMAL
};

class Ghost {
private:
    GridPosition position;
    GridPosition target;
    EntityFacing lastFacing; // Used for computing
    uint8_t dotThreshold;    // Dot-eaten threshold (in %) to exit the ghost house.

    const GhostPersonality personality;
    GhostAiMode  mode;
    GhostAiMode  modeBeforeFrightened; // Mode to restore after Frightened ends.
    GhostHouseState houseState;        // Exit state machine.

    // Frightened timer
    unsigned long frightenedStartMs; // millis() when Frightened began.
    uint16_t      frightenedDurationMs; // Duration in ms (set from PROGMEM table).

    GameState* state; // The GameState is shared by Game (non-const: we need millis context).

    // Move one step toward the ghost-house exit tile.
    void moveTowardExit();

    void moveTowardTarget();   // Greedy 1-step toward this->target.
    void moveRandom();         // Frightened mode movement.

    bool isDotThresholdReached() const;

    // Compute the frightened duration for the current level (reads PROGMEM).
    uint16_t getFrightenedDurationMs() const;

public:
    Ghost(GameState* state, GhostPersonality personality);
    ~Ghost() = default;

    // We don't need a computeFrightenedTarget() because in this mode there is no target.
    void computeChaseTarget();
    void computeScatterTarget();
    // Determine the next target cell based on personality and AI mode.
    void computeNewTarget();

    /*
    computeNewPosition() is called each tick.
    It handles the house-exit state machine first, then the normal AI.
    */
    void computeNewPosition();

    /*
    AiMode is determined by in-game timing.

    +--------+----------+------------+----------+
    | Mode   | Level 1  | Levels 2–4 | Levels 5+|
    +--------+----------+------------+----------+
    | Scatter| 7        | 7          | 5        |
    | Chase  | 20       | 20         | 20       |
    | Scatter| 7        | 7          | 5        |
    | Chase  | 20       | 20         | 20       |
    | Scatter| 5        | 5          | 5        |
    | Chase  | 20       | 1033       | 1037     |
    | Scatter| 5        | 1/60       | 1/60     |
    | Chase  | indefinite| indefinite | indefinite|
    +--------+----------+------------+----------+
    */
    void setAiMode(GhostAiMode newMode);

    /**
     * @brief Enter Frightened mode, saving the previous mode for restoration.
     * Duration is read from FRIGHTENED_DURATIONS[] in PROGMEM based on the current level.
     */
    void triggerFrightened();

    /**
     * @brief Called each tick. Handles Frightened timeout and restores previous mode.
     * @return true if the ghost is currently blinking (last ~2 s of frightened).
     */
    bool updateFrightened();

    /** @return true if the ghost is in Frightened mode and should blink. */
    bool isBlinking() const;

    void setPosition(GridPosition pos);
    void setFacing(EntityFacing facing);

    GridPosition  getPosition()  const { return this->position; }
    GhostAiMode   getMode()      const { return this->mode; }
    GhostHouseState getHouseState() const { return this->houseState; }

    char* getGhostInformations();
};