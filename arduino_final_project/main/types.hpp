#pragma once
#include <Arduino.h> // For uint8_t type
#define GAME_GRID_X_AXIS_LEN 28
#define GAME_GRID_Y_AXIS_LEN 36

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

struct GameState {
    // TODO: OPTIMISATION MÉMOIRE (priorité haute, ~990 octets à économiser)
    //
    // Problème actuel : grid[][] occupe 28×36 = 1008 octets de heap, sur un
    // total de 2048 octets de SRAM disponibles sur ATmega328P. Cela laisse
    // ~479 octets pour toute la call stack de loop(), ce qui provoque un
    // stack overflow silencieux (freeMemory() == 0 en loop).
    //
    // Solution à implémenter :
    //
    // ÉTAPE 1 — Déplacer le background en Flash (PROGMEM) dans levels.hpp :
    //   const uint8_t LEVEL_0[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN] PROGMEM = { ... };
    //   Lecture : pgm_read_byte(&LEVEL_0[y][x])
    //   Gain : 1008 octets de SRAM récupérés
    //
    // ÉTAPE 2 — Supprimer grid[][] de GameState, ne garder que les positions
    //   des entités (pacman + 4 fantômes + fruit), soit ~20 octets au total :
    //   GridPosition pacman;
    //   GridPosition ghosts[4];
    //   GridPosition fruit;
    //   bool fruitActive;
    //
    // ÉTAPE 3 — Adapter Screen::print_frame() pour reconstruire la vue à la
    //   volée en combinant PROGMEM (background) + positions (entités),
    //   sans jamais matérialiser la grille complète en RAM.
    //
    // Résultat attendu : GameState ~20 octets, ~1400 octets libres en loop.

    // Tick counter, incremented at each game step, is used for timing and animations.
    uint16_t tick;
    uint8_t level; // Current level, used for loading the correct background and AI mode duration from PROGMEM.
    uint8_t modePhase; // Index in the current mode phase, used for timing and AI mode switching.
    unsigned long lastModeChangeMs;

    Cell grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN]; // Official grid size from the first game

    // Centralized positions of entities for easier access to Ghosts AI movement, instead of searching the grid for them.
    GridPosition pacmanPosition;
    EntityFacing pacmanFacing;
    GridPosition blueGhostPosition;
    GridPosition redGhostPosition;
    GridPosition pinkGhostPosition;
    GridPosition orangeGhostPosition;
    uint8_t totalDots;
    uint8_t remainingDots;

    GameState() : tick(0), level(0),modePhase(0),pacmanPosition({0, 0}), blueGhostPosition({0, 0}), redGhostPosition({0, 0}), pinkGhostPosition({0, 0}), orangeGhostPosition({0, 0}), pacmanFacing(EF_NORTH), totalDots(0), remainingDots(0) {
        // Initialize the grid with empty cells (0b00000000) which is BG_EMPTY + ENT_EMPTY
        // We use memset which is fastest to initialize a grid of Cell.
        memset(grid, 0, sizeof(grid));
    }
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
    
    if (state->grid[pos.y][pos.x].getBackground() == BG_WALL) {
        return false; // Not walkable if it's a wall
    }

    // TODO: FIXME: CLEAN THIS WHEN SWITCHING TO PROGMEM BACKGROUND AND ENTITY POSITION ONLY
    if (state->grid[pos.y][pos.x].getEntity() != ENT_EMPTY) {
        return false; // Not walkable if there is an entity
    }
    // In the original game, ghosts can walk on the same cell,
    // this will be fix when we will switch to entity position only and not grid anymore.

    return true; // Walkable if it's not a wall and there is no entity
};