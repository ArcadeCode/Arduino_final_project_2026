#include "game.hpp"
#include "audio.hpp"

// ─── Constructor / Destructor ─────────────────────────────────────────────────

Game::Game()
    : ghosts{
        Ghost(&this->state, GP_RED),
        Ghost(&this->state, GP_PINK),
        Ghost(&this->state, GP_BLUE),
        Ghost(&this->state, GP_ORANGE)
    }
{}

Game::~Game() = default;

// ─── start() ─────────────────────────────────────────────────────────────────

void Game::start() {
    this->state.tick = 0;
    this->state.isWin = false;
    this->state.isGameOver = false;

    // Dummy Pac-Man position/facing; overwritten by loadLevel().
    this->pacmanPosition = {20, 20};
    this->pacmanFacing   = EF_WEST;
    // TODO: Implement starting facing for Pac-Man and ghosts in the editor.

    // Initial ghost facings based on the canonical first level layout.
    this->ghosts[0].setFacing(EF_WEST);  // Red
    this->ghosts[1].setFacing(EF_SOUTH); // Pink
    this->ghosts[2].setFacing(EF_NORTH); // Blue
    this->ghosts[3].setFacing(EF_WEST);  // Orange

    AudioEngine::play(SFX_NORMAL_MOVE); // Background music, pacman moving sound.
}

// ─── step() ──────────────────────────────────────────────────────────────────

GameState& Game::step() {
    this->state.tick++;

    // Sync local copies from the shared GameState (inputs may have updated pacmanFacing).
    this->pacmanPosition = this->state.pacmanPosition;
    this->pacmanFacing   = this->state.pacmanFacing;

    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        this->ghosts[i].setPosition(this->state.ghostPositions[i]);
    }

    // ── Pac-Man movement ──────────────────────────────────────────────────────
    this->computePacmanPosition();

    // ── Dot / energizer collection ────────────────────────────────────────────
    CellBackgroundType cell = readLevelBackground(this->state, pacmanPosition.x, pacmanPosition.y);

    if (cell == BG_GUM || cell == BG_ENERGIZE) {
        writeLevelBackground(this->state, pacmanPosition.x, pacmanPosition.y, BG_EMPTY);
        if (this->state.remainingDots > 0) {
            this->state.remainingDots--;
        }
        if (cell == BG_ENERGIZE) {
            triggerFrightenedAll();
            AudioEngine::play(SFX_FRIGHTENED);
        } else {
            AudioEngine::play(SFX_EAT_DOT);
        }
    }


    // ── Win / lose check ──────────────────────────────────────────────────────
    if (this->state.remainingDots == 0) {
        AudioEngine::play(SFX_LEVEL_WIN);
        this->state.isWin = true;
    } else {
        for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
            if (this->ghosts[i].getPosition() == this->pacmanPosition) {
                if (this->ghosts[i].getMode() == GM_Frightened) {
                    AudioEngine::play(SFX_EAT_GHOST);
                } else {
                    AudioEngine::play(SFX_DEATH);
                    this->state.isGameOver = true;
                }
                break;
            }
        }
    }

    // ── Ghost AI ──────────────────────────────────────────────────────────────
    updateGhostModes();

    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        this->ghosts[i].computeNewPosition();
        // Write back the updated position to the shared state.
        this->state.ghostPositions[i] = this->ghosts[i].getPosition();
    }

    return this->state;
}

// ─── updateGhostModes() ───────────────────────────────────────────────────────

/**
 * Drives the global Scatter ↔ Chase phase schedule from MODE_DURATIONS[].
 * Ghosts in Frightened mode are left alone — their own timer handles the return.
 * When a phase transition occurs, Ghost::setAiMode() is called, which internally
 * stores the new mode as "mode to restore to" if the ghost is currently Frightened.
 */
void Game::updateGhostModes() {
    unsigned long elapsed = millis() - this->state.lastModeChangeMs;

    uint8_t levelIndex = (this->state.level == 1) ? 0 :
                         (this->state.level <= 4) ? 1 : 2;

    uint16_t durationSeconds = pgm_read_word(&MODE_DURATIONS[levelIndex][this->state.modePhase]);

    // 0 = final indefinite Chase — never transition again.
    if (durationSeconds == 0) return;

    unsigned long durationMs = (unsigned long)durationSeconds * 1000UL;

    if (elapsed >= durationMs) {
        this->state.lastModeChangeMs = millis();
        this->state.modePhase++;

        // Even phase → Scatter, odd phase → Chase.
        GhostAiMode newMode = (this->state.modePhase % 2 == 0) ? GM_Scatter : GM_Chase;

        for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
            // setAiMode() handles the Frightened guard internally.
            this->ghosts[i].setAiMode(newMode);
        }
    }
}

// ─── triggerFrightenedAll() ───────────────────────────────────────────────────

void Game::triggerFrightenedAll() {
    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        this->ghosts[i].triggerFrightened();
    }
}

// ─── computePacmanPosition() ─────────────────────────────────────────────────

void Game::computePacmanPosition() {
    // inputs.cpp has already written the desired direction into state.pacmanFacing.
    GridPosition neighbor = getNeighbor(this->pacmanPosition, this->get_pacmanFacing());

    // Check the NEIGHBOR tile, not the current position.
    if (!isWalkable(&this->state, neighbor)) {
        // Bump — stay in place.
        return;
    }

    this->state.pacmanPosition = neighbor;
    this->pacmanPosition       = neighbor;
}