/**
 * screen.hpp — TFT rendering for the Pac-Man game.
 *
 * Hardware target: ILI9341 240×320 display driven via SPI.
 * Libraries required (install through Arduino Library Manager):
 *   - Adafruit GFX Library
 *   - Adafruit ILI9341
 *
 * Grid layout
 * ──────────────────────────────────────────────────────────────────────────────
 * The Pac-Man grid is 28×31 playable cells (rows 0-30 of the 36-row array;
 * rows 31-35 are empty padding produced by the editor).
 *
 * With CELL_PX = 7 px and a 240-wide display:
 *   - 28 columns × 7 px = 196 px  → centred with GRID_OFFSET_X = 22 px
 *   - 31 rows    × 7 px = 217 px  → top margin GRID_OFFSET_Y   =  8 px
 *
 * Pin mapping (default — change TFT_CS / TFT_DC / TFT_RST if needed):
 *   TFT_CS  → Arduino pin 10
 *   TFT_DC  → Arduino pin 9   (labelled RS on some boards)
 *   TFT_RST → Arduino pin 8
 *   MOSI    → Arduino pin 11  (hardware SPI — MUST be wired)
 *   SCK     → Arduino pin 13  (hardware SPI — MUST be wired)
 *   MISO    → Arduino pin 12  (hardware SPI — wire even if unused for reads)
 *   LED/BL  → 3.3 V via 100 Ω resistor  (or a PWM pin for brightness control)
 *
 * Colour palette (ILI9341 uses 16-bit RGB565):
 *   Background colours match the classic arcade look as closely as possible.
 */

#pragma once

#include "game.hpp"
#include "ghost.hpp"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// ── Pin definitions ──────────────────────────────────────────────────────────
#define TFT_CS   10
#define TFT_DC    9 // Called RS pin on our screen
#define TFT_RST   8

// ── Grid geometry ────────────────────────────────────────────────────────────
// Number of playable rows (skip the empty padding rows 31-35).
#define PLAYABLE_ROWS   31

// Pixel size of one grid cell.  7 px fits 28 cols in 196 px on a 240-px display.
#define CELL_PX          7

// Top-left pixel of the grid on the TFT.
#define GRID_OFFSET_X   22   // (240 - 28*7) / 2
#define GRID_OFFSET_Y    8

// Score panel height at the bottom of the screen.
#define SCORE_PANEL_Y  (GRID_OFFSET_Y + PLAYABLE_ROWS * CELL_PX + 4)

// ── Colour palette (RGB565) ───────────────────────────────────────────────────
#define COL_BLACK        0x0000u
#define COL_WALL         0x1A5Fu   // Deep blue  (#001AA0 → approx)
#define COL_GUM          0xFDB6u   // Pale pink  (#FFDB6C → approx)
#define COL_ENERGIZER    0xFFFFu   // White
#define COL_PACMAN       0xFFE0u   // Yellow
#define COL_GHOST_RED    0xF800u   // Red
#define COL_GHOST_PINK   0xFB56u   // Pink
#define COL_GHOST_BLUE   0x051Fu   // Blue (normal)
#define COL_GHOST_ORANGE 0xFD20u   // Orange
#define COL_GHOST_FRIGHT 0x001Fu   // Dark blue (frightened)
#define COL_GHOST_BLINK  0xFFFFu   // White (blinking)
#define COL_GHOST_EYES   0xFFFFu   // Eyes highlight
#define COL_TEXT         0xFFFFu   // UI text

// ─────────────────────────────────────────────────────────────────────────────

class Screen {
public:
    Screen();
    ~Screen() = default;

    /**
     * @brief Draw the complete frame: background + entities + score bar.
     *
     * @param state    Current game state (background, positions, dot count).
     * @param ghosts   Pointer to the 4 Ghost objects (for mode / blink queries).
     */
    void print_frame(GameState& state, const Ghost* ghosts);

    /**
     * @brief Compatibility overload — draw without ghost mode information.
     *        Ghosts are rendered in their default colour without blink.
     */
    void print_frame(GameState& state);

    /**
     * @brief Full-screen wipe + redraw the static maze walls.
     *        Call once at level start or after a screen mode change.
     */
    void redrawBackground(GameState& state);    

    // ── Serial debug output ───────────────────────────────────────────────────
    void serial_print_frame(GameState& state);

private:
    Adafruit_ILI9341 tft;

    // Cache of the last rendered background to enable dirty-cell optimisation.
    // Stores the packed byte for every cell; 0xFF = not yet drawn.
    uint8_t bgCache[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN / 4];

    // Last positions drawn for each entity (to erase them next frame).
    GridPosition lastPacman;
    GridPosition lastGhosts[GHOSTS_COUNT];

    bool     firstFrame; // true → force full redraw on first call

    // ── Low-level draw helpers ────────────────────────────────────────────────

    /** Return the top-left pixel of grid cell (x, y). */
    inline int16_t cellX(uint8_t x) const { return GRID_OFFSET_X + x * CELL_PX; }
    inline int16_t cellY(uint8_t y) const { return GRID_OFFSET_Y + y * CELL_PX; }

    /** Fill one grid cell with a solid colour. */
    void fillCell(uint8_t x, uint8_t y, uint16_t colour);

    /** Draw the background tile for cell (x, y) based on GameState. */
    void drawBgCell(GameState& state, uint8_t x, uint8_t y);

    // ── Entity renderers ──────────────────────────────────────────────────────

    void drawPacman(uint8_t x, uint8_t y);
    void drawGhost(uint8_t x, uint8_t y, uint16_t bodyColour, bool drawEyes);

    // ── Score / HUD ───────────────────────────────────────────────────────────
    void drawHUD(GameState& state);
};