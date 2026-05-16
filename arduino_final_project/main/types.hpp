#pragma once
#include <Arduino.h> // For uint8_t type
#define GAME_GRID_X_AXIS_LEN 28
#define GAME_GRID_Y_AXIS_LEN 36

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

struct gameState {
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
    // ÉTAPE 2 — Supprimer grid[][] de gameState, ne garder que les positions
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
    // Résultat attendu : gameState ~20 octets, ~1400 octets libres en loop.

    unsigned long tick;
    Cell grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN]; // Official grid size from the first game

    gameState() : tick(0) {
        // Initialize the grid with empty cells (0b00000000) which is BG_EMPTY + ENT_EMPTY
        // We use memset which is fastest to initialize a grid of Cell.
        memset(grid, 0, sizeof(grid));
    }
};

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
};