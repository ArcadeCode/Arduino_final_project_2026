/**
 * screen.cpp — TFT rendering implementation.
 *
 * Rendering strategy
 * ──────────────────
 * To avoid the ~30 ms full-screen clear + redraw cycle that causes visible
 * flicker on a slow SPI bus, we use a dirty-cell approach:
 *
 *  1. On the first frame (or after redrawBackground()), every background cell
 *     is drawn and its packed value cached in bgCache[][].
 *  2. On subsequent frames, only cells whose packed value changed (dot eaten,
 *     energizer collected) are redrawn.
 *  3. Each entity is erased by redrawing the background cell it occupied last
 *     frame, then the new position is painted on top.
 *
 * Entity draw order: background → ghosts → Pac-Man (Pac-Man is always on top).
 *
 * Ghost sprites
 * ─────────────
 * Each ghost is drawn as:
 *   - A rounded body (filled circle + rectangle below).
 *   - Two white "eyes" (small filled circles).
 * In Frightened mode the body is dark-blue; during the blink window the
 * body alternates dark-blue / white every 250 ms.
 *
 * Pac-Man sprite
 * ──────────────
 * Pac-Man is drawn as a yellow filled circle with a black wedge removed.
 * The wedge direction tracks pacmanFacing.  For simplicity on the small 7 px
 * cells we use a filled circle minus a triangle (two fillTriangle() calls).
 *
 * HUD
 * ───
 * A one-line strip below the grid shows "DOTS: xxx" and "LVL: n".
 * It is redrawn every frame (the strip is small so the cost is low).
 */

#include "screen.hpp"

// ─── Constructor ─────────────────────────────────────────────────────────────

Screen::Screen()
    : tft(TFT_CS, TFT_DC, TFT_RST),
      firstFrame(true)
{
    // Force hardware SPI initialisation before the display driver starts.
    // Required on Arduino Uno/Nano : without this, MOSI/SCK may not be
    // configured as outputs and the ILI9341 never receives valid data.
    // Configure hardware SPI pins explicitly before SPI.begin().
    // On Arduino Uno/Nano: MOSI=11, MISO=12, SCK=13.
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(SCK,  OUTPUT);
    SPI.begin();
    // Limit SPI clock to 8 MHz — some ILI9341 clones can't handle the default 16 MHz.
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    SPI.endTransaction();

    // Manual pin init — ensures CS is LOW and RST pulses before the driver starts.
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_CS,  OUTPUT);
    pinMode(TFT_DC,  OUTPUT);

    // Hard reset sequence — longer delays for slow modules.
    digitalWrite(TFT_CS,  LOW);
    digitalWrite(TFT_RST, HIGH);
    delay(50);
    digitalWrite(TFT_RST, LOW);
    delay(100);
    digitalWrite(TFT_RST, HIGH);
    delay(200);

    tft.begin(8000000); // Explicit 8 MHz SPI clock passed to the driver.

    // Diagnostic SPI : lit le Power Mode register (0x0A).
    // Valeur attendue : 0x9C ou 0x94  → ILI9341 initialisé correctement.
    // Valeur 0x00 ou 0xFF             → pas de communication (câblage SPI KO).
    uint8_t tftId = tft.readcommand8(0x0A);
    Serial.print(F("TFT Power Mode: 0x"));
    Serial.println(tftId, HEX);

    tft.setRotation(2);
    tft.fillScreen(0xF800); // TEST: rouge vif — à remettre en COL_BLACK une fois confirmé

    // Initialise caches to "not drawn".
    memset(bgCache, 0xFF, sizeof(bgCache));
    lastPacman = {255, 255}; // Sentinel — no last position.
    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        lastGhosts[i] = {255, 255};
    }
}

// ─── Low-level helpers ────────────────────────────────────────────────────────

void Screen::fillCell(uint8_t x, uint8_t y, uint16_t colour) {
    tft.fillRect(cellX(x), cellY(y), CELL_PX, CELL_PX, colour);
}

void Screen::drawBgCell(GameState& state, uint8_t x, uint8_t y) {
    CellBackgroundType bg = readLevelBackground(state, x, y);
    switch (bg) {
        case BG_WALL: {
            // Draw a solid wall tile.
            fillCell(x, y, COL_WALL);
            break;
        }
        case BG_GUM: {
            // Black background + small centred dot.
            fillCell(x, y, COL_BLACK);
            int16_t cx = cellX(x) + CELL_PX / 2;
            int16_t cy = cellY(y) + CELL_PX / 2;
            tft.fillCircle(cx, cy, 1, COL_GUM);
            break;
        }
        case BG_ENERGIZE: {
            // Black background + larger energizer dot.
            fillCell(x, y, COL_BLACK);
            int16_t cx = cellX(x) + CELL_PX / 2;
            int16_t cy = cellY(y) + CELL_PX / 2;
            tft.fillCircle(cx, cy, CELL_PX / 2 - 1, COL_ENERGIZER);
            break;
        }
        case BG_EMPTY:
        default:
            fillCell(x, y, COL_BLACK);
            break;
    }
}

// ─── Entity sprites ───────────────────────────────────────────────────────────

/**
 * @brief Internal: draw Pac-Man with explicit facing.
 *        Defined first so that Screen::drawPacman() and print_frame() can call it.
 */
static void drawPacmanFacing(Adafruit_ILI9341& tft, int16_t cx, int16_t cy, int16_t r, EntityFacing facing) {
    tft.fillCircle(cx, cy, r, COL_PACMAN);

    // Build the mouth: two triangles meeting at the centre, fanning outward.
    int16_t ox = 0, oy = 0; // Facing unit vector (scaled by r).
    int16_t px = 0, py = 0; // Perpendicular unit vector (scaled by r/2).
    switch (facing) {
        case EF_EAST:  ox =  r; oy =  0; px =  0; py = r / 2 + 1; break;
        case EF_WEST:  ox = -r; oy =  0; px =  0; py = r / 2 + 1; break;
        case EF_NORTH: ox =  0; oy = -r; px = r / 2 + 1; py = 0; break;
        case EF_SOUTH: ox =  0; oy =  r; px = r / 2 + 1; py = 0; break;
    }
    // Upper triangle.
    tft.fillTriangle(cx, cy,
                     cx + ox,      cy + oy,
                     cx + ox + px, cy + oy + py,
                     COL_BLACK);
    // Lower triangle.
    tft.fillTriangle(cx, cy,
                     cx + ox,      cy + oy,
                     cx + ox - px, cy + oy - py,
                     COL_BLACK);
}

/**
 * @brief Draw Pac-Man as a yellow circle with a directional mouth wedge.
 *
 * Delegates to drawPacmanFacing() with EF_EAST as default.
 * The actual call in print_frame() passes state.pacmanFacing directly.
 */
void Screen::drawPacman(uint8_t x, uint8_t y) {
    int16_t cx = cellX(x) + CELL_PX / 2;
    int16_t cy = cellY(y) + CELL_PX / 2;
    drawPacmanFacing(tft, cx, cy, CELL_PX / 2, EF_EAST);
}

/**
 * @brief Draw a ghost sprite at cell (x, y) with the given body colour.
 *
 * Shape:
 *   - Upper half: filled circle (the "head").
 *   - Lower half: filled rectangle.
 *   - Two small white circles as eyes (not drawn in frightened mode).
 */
void Screen::drawGhost(uint8_t x, uint8_t y, uint16_t bodyColour, bool drawEyes) {
    int16_t x0 = cellX(x);
    int16_t y0 = cellY(y);
    int16_t cx  = x0 + CELL_PX / 2;
    int16_t cy  = y0 + CELL_PX / 2;
    int16_t r   = CELL_PX / 2;

    // Upper dome.
    tft.fillCircle(cx, cy, r, bodyColour);
    // Lower rectangle (fills from centre down to bottom of cell).
    tft.fillRect(x0, cy, CELL_PX, r + 1, bodyColour);

    if (drawEyes && CELL_PX >= 7) {
        // Two white dots as eyes.
        int16_t eyeR = max(1, CELL_PX / 7);
        tft.fillCircle(cx - 2, y0 + 2, eyeR, COL_GHOST_EYES);
        tft.fillCircle(cx + 2, y0 + 2, eyeR, COL_GHOST_EYES);
    }
}

// ─── HUD ─────────────────────────────────────────────────────────────────────

void Screen::drawHUD(GameState& state) {
    // Erase the HUD strip.
    tft.fillRect(0, SCORE_PANEL_Y, 240, 16, COL_BLACK);

    tft.setTextColor(COL_TEXT);
    tft.setTextSize(1);

    tft.setCursor(4, SCORE_PANEL_Y + 2);
    tft.print(F("DOTS:"));
    tft.print(state.remainingDots);

    tft.setCursor(100, SCORE_PANEL_Y + 2);
    tft.print(F("LVL:"));
    tft.print(state.level);

    tft.setCursor(160, SCORE_PANEL_Y + 2);
    tft.print(F("T:"));
    tft.print(state.tick);
}

// ─── redrawBackground() ───────────────────────────────────────────────────────

void Screen::redrawBackground(GameState& state) {
    tft.fillScreen(COL_BLACK);
    for (uint8_t y = 0; y < PLAYABLE_ROWS; y++) {
        for (uint8_t x = 0; x < GAME_GRID_X_AXIS_LEN; x++) {
            drawBgCell(state, x, y);
            bgCache[y][x / 4] = state.levelBackground[y][x / 4];
        }
    }
    // Invalidate entity caches so they are redrawn on the next frame.
    lastPacman = {255, 255};
    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        lastGhosts[i] = {255, 255};
    }
    firstFrame = false;
}

// ─── print_frame() ───────────────────────────────────────────────────────────

void Screen::print_frame(GameState& state, const Ghost* ghosts) {
    // Force a full background redraw on the very first call.
    if (firstFrame) {
        redrawBackground(state);
    }

    // ── 1. Update changed background cells (dots eaten, energizers) ──────────
    for (uint8_t y = 0; y < PLAYABLE_ROWS; y++) {
        for (uint8_t bx = 0; bx < GAME_GRID_X_AXIS_LEN / 4; bx++) {
            uint8_t newPacked = state.levelBackground[y][bx];
            if (newPacked != bgCache[y][bx]) {
                // At least one cell in this byte changed — redraw all 4.
                for (uint8_t bit = 0; bit < 4; bit++) {
                    drawBgCell(state, bx * 4 + bit, y);
                }
                bgCache[y][bx] = newPacked;
            }
        }
    }

    // ── 2. Erase entities at their previous positions ─────────────────────────
    // Erase Pac-Man.
    if (lastPacman.x != 255) {
        drawBgCell(state, lastPacman.x, lastPacman.y);
    }
    // Erase ghosts.
    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        if (lastGhosts[i].x != 255) {
            drawBgCell(state, lastGhosts[i].x, lastGhosts[i].y);
        }
    }

    // ── 3. Draw ghosts at new positions ──────────────────────────────────────
    static const uint16_t ghostColours[GHOSTS_COUNT] = {
        COL_GHOST_RED,
        COL_GHOST_PINK,
        COL_GHOST_BLUE,
        COL_GHOST_ORANGE
    };

    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        GridPosition gp = state.ghostPositions[i];
        if (gp.y >= PLAYABLE_ROWS) continue; // Skip if outside playable area.

        uint16_t bodyCol = ghostColours[i];
        bool     eyes    = true;

        if (ghosts != nullptr) {
            GhostAiMode gm = ghosts[i].getMode();
            if (gm == GM_Frightened) {
                eyes    = false;
                bodyCol = ghosts[i].isBlinking() ? COL_GHOST_BLINK : COL_GHOST_FRIGHT;
            }
        }

        drawGhost(gp.x, gp.y, bodyCol, eyes);
        lastGhosts[i] = gp;
    }

    // ── 4. Draw Pac-Man on top ────────────────────────────────────────────────
    GridPosition pp = state.pacmanPosition;
    if (pp.y < PLAYABLE_ROWS) {
        int16_t cx = cellX(pp.x) + CELL_PX / 2;
        int16_t cy = cellY(pp.y) + CELL_PX / 2;
        drawPacmanFacing(tft, cx, cy, CELL_PX / 2, state.pacmanFacing);
        lastPacman = pp;
    }

    // ── 5. HUD strip ──────────────────────────────────────────────────────────
    drawHUD(state);
}

// Compatibility overload — no ghost mode info available.
void Screen::print_frame(GameState& state) {
    print_frame(state, nullptr);
}


/**
 * @brief Print a frame of the current game state in the Serial Monitor
 * @note Need to put a Serial.begin() before calling this function.
 * 
 * Work by loading the background from PROGMEM and combining it with
 * the entities positions from the GameState to print a char representation
 * of each cell.
 * 
 * @attention we print line by line instead of frame by frame due to memory limitations.
 */
void Screen::serial_print_frame(GameState &state) {
    char serial_s[GAME_GRID_X_AXIS_LEN + 4];

    serial_s[0] = '|';
    serial_s[GAME_GRID_X_AXIS_LEN + 1] = '|';
    serial_s[GAME_GRID_X_AXIS_LEN + 2] = '\n';
    serial_s[GAME_GRID_X_AXIS_LEN + 3] = '\0';

    Serial.println(F("@____________________________@"));

    for (uint8_t y = 0; y < GAME_GRID_Y_AXIS_LEN; y++) {

        for (uint8_t x = 0; x < GAME_GRID_X_AXIS_LEN; x++) {

            serial_s[x + 1] =
                cellBackgroundToChar(readLevelBackground(state, x, y));

            if (state.ghostPositions[0] == GridPosition(x, y)) {
                serial_s[x + 1] = 'R';
            }
            else if (state.ghostPositions[1] == GridPosition(x, y)) {
                serial_s[x + 1] = 'K';
            }
            else if (state.ghostPositions[2] == GridPosition(x, y)) {
                serial_s[x + 1] = 'B';
            }
            else if (state.ghostPositions[3] == GridPosition(x, y)) {
                serial_s[x + 1] = 'O';
            }
            else if (state.pacmanPosition == GridPosition(x, y)) {
                serial_s[x + 1] = 'P';
            }
        }

        // Print ONLY after the line is fully built
        Serial.print(serial_s);
    }

    Serial.println(F("@____________________________@"));
}