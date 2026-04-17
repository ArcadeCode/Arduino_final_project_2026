#include <cstdint> // For uint8_t
#include "inputs.cpp"

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
};

struct gameState {
    unsigned long tick;
    Cell grid[28][36]; // Official grid size from the first game
};


class Game {
private:
    gameState state;

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
    gameState* step(); /* Step from one new frame */
    void registerInputs(joystickPosition joystickPos, bool isStartPressed, bool isSelectPressed);

    // Level information retrieve
    gameState loadLevel(uint8_t level);

};