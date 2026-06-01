#pragma once
#include <Arduino.h> // For uint8_t type
#define GAME_GRID_X_AXIS_LEN 28
#define GAME_GRID_Y_AXIS_LEN 36
#define GHOSTS_COUNT 4

/*
Simple solution because we need tuples of 2D positions in some part of the code
and I don't wanna call `std::tuple` because we are in a limited environnement
and this object have to many unusefull attributes and methods.
*/
struct GridPosition {
    uint8_t x;
    uint8_t y;

    constexpr GridPosition() : x(0), y(0) {}
    constexpr GridPosition(uint8_t x, uint8_t y) : x(x), y(y) {}

    // Surcharge of == operator for easier comparison of GridPosition objects, useful for ghosts AI and collision detection.
    constexpr bool operator==(const GridPosition& other) const {
        return x == other.x && y == other.y;
    }
};

// Level parameters struct, used when loading a level.
struct LevelParams {
    const uint8_t* background; // Pointer to LEVEL_X_BG (PROGMEM)
    GridPosition pacmanStart;
    GridPosition ghostStarts[4]; // 4 Ghosts: Red, Pink, Blue, Orange
    uint16_t totalDots;
    // Add more parameters here if needed (e.g., fruit spawn positions and timings)
};

enum EntityFacing {
    EF_NORTH,
    EF_SOUTH,
    EF_EAST,
    EF_WEST
};

inline char entityFacingToChar(EntityFacing facing) {
    switch (facing) {
        case EF_NORTH: return 'N';
        case EF_SOUTH: return 'S';
        case EF_EAST:  return 'E';
        case EF_WEST:  return 'W';
        default:       return '?';
    }
}

enum CellBackgroundType {
    BG_EMPTY=0,
    BG_WALL=1,
    BG_GUM=2,     // In french it's called : "Pac-gum"
    BG_ENERGIZE=3 // In french it's called : "Super pac-gum"
};

inline char cellBackgroundToChar(CellBackgroundType bg) {
    switch (bg) {
        case BG_EMPTY:    return ' ';
        case BG_WALL:     return '#';
        case BG_GUM:      return '.';
        case BG_ENERGIZE: return '*';
        default:          return '?';
    }
}

enum CellEntitiesType {
    /* Due to Arduino ram limitations, the overlapping
    of two entities cannot be done on the logic side
    but we can detect pacman collision and switch his
    sprite two an overlapping with the correct entity.
    */
    ENT_EMPTY=0,        // 000
    ENT_PACMAN=1,       // 001
    ENT_BLUE_GHOST=2,   // 010
    ENT_RED_GHOST=3,    // 011
    ENT_PINK_GHOST=4,   // 100
    ENT_ORANGE_GHOST=5, // 101
    ENT_FRUIT=6,        // 110
};

struct Cell {
    /* Optimized representation of a Cell using bit-packing 
    technique, each cell is 1 byte or 8 bits.

    B -> this bit is used by the background
    E -> this bit is used by entities
    - -> this bit is allocated but never used

    bit:  7 6 5 4 3 2 1 0
          - - - E E E B B
    */
    uint8_t data;

    // --- Background ---
    inline void setBackground(CellBackgroundType bg) {
        data = (data & 0b11111100) | (bg & 0b00000011);
    }

    inline CellBackgroundType getBackground() const {
        return (CellBackgroundType)(data & 0b00000011);
    }

    // --- Entity ---
    inline void setEntity(CellEntitiesType ent) {
        data = (data & 0b11100011) | ((ent & 0b00000111) << 2);
    }

    inline CellEntitiesType getEntity() const {
        return (CellEntitiesType)((data >> 2) & 0b00000111);
    }

    // Convert cell type to a char value which can be print using Serial.print(Cell.toChar())
    inline char toChar() const {
        switch (getEntity()) {
            case ENT_PACMAN:      return 'P';
            case ENT_RED_GHOST:   return 'R';
            case ENT_BLUE_GHOST:  return 'B';
            case ENT_PINK_GHOST:  return 'K';
            case ENT_ORANGE_GHOST:return 'O';
            case ENT_FRUIT:       return 'F';
            default: break;
        }
        switch (getBackground()) {
            case BG_WALL:     return '#';
            case BG_GUM:      return '.';
            case BG_ENERGIZE: return '*';
            case BG_EMPTY:    return ' ';
            default:          return '?';
        }
    }
};

/**
 * @brief AI Mode durations in seconds for each level, stored in PROGMEM to save SRAM.
 * 
 * Each row corresponds to a level (1, 2-4, 5+), and each column corresponds to a mode phase
 * (Scatter 1, Chase 1, Scatter 2, Chase 2, Scatter 3, Chase 3, Scatter 4, Chase 4).
 * The last phase (Chase 4) has a duration of 0, which means that the ghosts will stay
 * in Chase mode indefinitely after the last Scatter phase.
 * 
 * @source The Pac-Man dossiers by Jamey Pittman
 */
static const uint16_t MODE_DURATIONS[3][8] PROGMEM = {
//  Sc  Ch   Sc  Ch   Sc   Ch    Sc     Ch
    {7,  20,  7,  20,  5,   20,   5,    0},  // Level 1
    {7,  20,  7,  20,  5, 1033,   1,    0},  // Levels 2-4
    {5,  20,  5,  20,  5, 1037,   1,    0},  // Levels 5+
};

/**
 * @brief The GameState struct holds all the information about the current state of the game.
 * It is used by the Game class to update the game state and by the Ghosts to compute their targets and movements.
 * Due to memory limitations, we store the background in PROGMEM and we only keep the positions of entities in the GameState.
 */
struct GameState {
    // TODO: Implement a simple Pacman struct.

    uint16_t tick; // Tick counter, incremented at each game step, is used for timing and animations.
    unsigned long lastModeChangeMs; // Timestamp of the last mode change, used to determine when to switch modes based on MODE_DURATIONS.
    
    // Variables set by levels.hpp when loading a level, used for timing and AI mode switching.
    uint8_t level; // Current level, set by levels.hpp when loading a level, used for timing and AI mode switching.
    uint8_t levelBackground[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN / 4]; // Local copy of the level background, initialized from PROGMEM when loading a level
    uint8_t modePhase; // Index in the current mode phase, used for timing and AI mode switching.
    uint16_t totalDots;
    uint16_t remainingDots;
    
    // Centralized positions of entities for easier access to Ghosts AI movement, instead of searching the grid for them.
    GridPosition pacmanPosition;
    EntityFacing pacmanFacing;
    GridPosition ghostPositions[GHOSTS_COUNT]; // 0: Red, 1: Pink, 2: Blue, 3: Orange

    GameState() : tick(0), level(0),modePhase(0),pacmanPosition({0, 0}), ghostPositions{{0, 0}, {0, 0}, {0, 0}, {0, 0}}, pacmanFacing(EF_NORTH), totalDots(0), remainingDots(0) {}
};

/**
 * @brief Read a cell background from a PROGMEM level array.
 *
 * @param level_bg  Pointer to a LEVEL_x_BG array (e.g. LEVEL_0_BG).
 * // TODO: Replace level_bg for level index and read the background from the current level.
 * The real ptr to level will be store in level.hpp.
 * @param x         Column (0..GAME_GRID_X_AXIS_LEN-1).
 * @param y         Row    (0..GAME_GRID_Y_AXIS_LEN-1).
 * @return CellBackgroundType
 */
inline CellBackgroundType readLevelBackground(const GameState& state, uint8_t x, uint8_t y)
{
    uint8_t packed = state.levelBackground[y][x >> 2];
    return (CellBackgroundType)((packed >> ((x & 0x03) << 1)) & 0x03);
}

/**
 * @brief Write a cell background to a PROGMEM level array.
 *
 * @param level_bg  Pointer to a LEVEL_x_BG array (e.g. LEVEL_0_BG).
 * @param x         Column (0..GAME_GRID_X_AXIS_LEN-1).
 * @param y         Row    (0..GAME_GRID_Y_AXIS_LEN-1).
 * @return nothing
 */
inline void writeLevelBackground(
    GameState& state,
    uint8_t x, uint8_t y, CellBackgroundType bg)
{
    uint8_t &packed = state.levelBackground[y][x >> 2];
    uint8_t mask = 0x03 << ((x & 0x03) << 1);
    packed = (packed & ~mask) | ((bg & 0x03) << ((x & 0x03) << 1));
}

/**
 * @brief Get all entities on a given cell.
 */
inline CellEntitiesType* getCellEntities(const GameState* state, uint8_t x, uint8_t y) {
    CellEntitiesType entities[6] = {}; // We can have at most 6 entities on a cell (Pacman, 4 ghosts, 1 fruit) but due 

    if (state->pacmanPosition == GridPosition(x, y)) {
        entities[0] = ENT_PACMAN;
    } else {
        for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
            if (state->ghostPositions[i] == GridPosition(x, y)) {
                switch (i) {
                    case 0: entities[1] = ENT_RED_GHOST; break;
                    case 1: entities[2] = ENT_PINK_GHOST; break;
                    case 2: entities[3] = ENT_BLUE_GHOST; break;
                    case 3: entities[4] = ENT_ORANGE_GHOST; break;
                }
                break; // Stop checking after finding the first ghost on the cell
            }
        }
    }

    return entities;
};

/**
 * @brief Get the neighboring position in the given direction.
 */
inline GridPosition getNeighbor(GridPosition pos, EntityFacing dir) {
    switch (dir) {
        case EF_NORTH: return {pos.x, pos.y - 1};
        case EF_SOUTH: return {pos.x, pos.y + 1};
        case EF_EAST:  return {pos.x + 1, pos.y};
        case EF_WEST:  return {pos.x - 1, pos.y};
        default:       return pos; // No movement if direction is invalid
    }
};

/**
 * @brief Get the opposite direction.
 * 
 * @note This mean, what the opposite direction of the one which is facing.
 */
inline EntityFacing getOpposite(EntityFacing dir) {
    switch(dir) {
        case EF_NORTH: return EF_SOUTH;
        case EF_SOUTH: return EF_NORTH;
        case EF_EAST:  return EF_WEST;
        case EF_WEST:  return EF_EAST;
        default:       return dir; // No movement if direction is invalid
    }
};

/**
 * @brief Calculate the squared distance between two grid positions.
 */
inline uint16_t squaredDistance(GridPosition a, GridPosition b) {
    int8_t dx = (int8_t)a.x - (int8_t)b.x;
    int8_t dy = (int8_t)a.y - (int8_t)b.y;
    return (uint16_t)(dx * dx) + (uint16_t)(dy * dy);
}

/**
 * @brief Check if a position is walkable (not a wall and within bounds).
 */
inline bool isWalkable(const GameState* state, GridPosition pos) {
    if (pos.x >= GAME_GRID_X_AXIS_LEN || pos.y >= GAME_GRID_Y_AXIS_LEN) {
        return false; // Out of bounds
    }
    // No need to check for pos < 0 because pos is unsigned (uint8_t)
    
    if (readLevelBackground(*state, pos.x, pos.y) == BG_WALL) {
        return false; // Not walkable if it's a wall
    }

    return true; // Walkable if it's not a wall and there is no entity
};