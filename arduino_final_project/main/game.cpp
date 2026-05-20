#include "game.hpp"

Game::Game()
    : ghosts{
        Ghost(&this->state, GP_RED),
        Ghost(&this->state, GP_PINK),
        Ghost(&this->state, GP_BLUE),
        Ghost(&this->state, GP_ORANGE)
    }
{};

Game::~Game() = default;

void Game::start() {
    this->state.tick = 0;

    this->pacmanPosition = {20, 20}; // Dummy position for pacman, we will change it in loadLevel() with the real position of pacman.
    this->pacmanFacing = EF_WEST; // Dummy facing for pacman, can be changed in loadLevel() with the real facing of pacman.
    // TODO:: Implement starting facing for pacman in the editor.

    // Initialize ghosts is done in the constructor due to presence of consts in them.
    // Initializing ghosts facing to dummy value based on the official first level, can be changed in loadLevel() with the real starting facing of each ghost.
    // TODO:: Implement starting facing for ghosts in the editor.
    this->ghosts[0].setFacing(EF_WEST);  // Red
    this->ghosts[1].setFacing(EF_SOUTH); // Pink
    this->ghosts[2].setFacing(EF_NORTH); // Blue
    this->ghosts[3].setFacing(EF_WEST);  // Orange
};

GameState& Game::step() {
    this->state.tick++;

    // At this point, the level was loaded,
    // We get current position of entities
    // from the GameState
    this->pacmanPosition = this->state.pacmanPosition;
    this->pacmanFacing = this->state.pacmanFacing;

    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        this->ghosts[i].setPosition(this->state.ghostPositions[i]);
    }
    
    

    // Compute Pacman movement based on the current inputs and the grid.
    this->computePacmanPosition();

    // Check if pacman eat a dot or an energizer, and update the grid and the remaining dots count.
    if (readLevelBackground(this->state, pacmanPosition.x, pacmanPosition.y) == BG_GUM
        || readLevelBackground(this->state, pacmanPosition.x, pacmanPosition.y) == BG_ENERGIZE) {
        // Update the grid and the remaining dots count
        writeLevelBackground(this->state, pacmanPosition.x, pacmanPosition.y, BG_EMPTY);
        if (this->state.remainingDots > 0) {
            this->state.remainingDots--;
        }
    }

    // Check for win/lose conditions
    if (this->state.remainingDots == 0) {
        // Send WIN_FLAG
    } else {
        for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
            if (this->ghosts[i].getPosition() == this->pacmanPosition) {
                // Send LOSE_FLAG
                break;
            }
        }
    }
    
    // Compute new positions for all ghosts
    this->updateGhostModes();
    for (uint8_t i = 0; i < GHOSTS_COUNT; i++) {
        this->ghosts[i].computeNewPosition();
        this->state.ghostPositions[i] = this->ghosts[i].getPosition(); // Update the global state with the new ghost positions for easier access to them in the next step.
    }
    
    // In the end return the new state to be printed on the screen
    return this->state;
};

/**
 * @brief Update the AI mode of all ghosts based on the current level and mode phase, using the predefined durations from PROGMEM.
 * 
 * @note called each tick in Game::step()
 */
void Game::updateGhostModes() {
    // Combien de temps s'est écoulé depuis le dernier changement de mode ?
    unsigned long elapsed = millis() - this->state.lastModeChangeMs;

    // Quel ligne du tableau utiliser selon le niveau ?
    uint8_t levelIndex = (this->state.level == 1) ? 0 :
                         (this->state.level <= 4) ? 1 : 2;

    // Combien de secondes doit durer la phase actuelle ?
    uint16_t durationSeconds = pgm_read_word(&MODE_DURATIONS[levelIndex][this->state.modePhase]);

    // 0 = Chase indéfinie finale, on ne bascule plus jamais
    if (durationSeconds == 0) return;

    unsigned long durationMs = (unsigned long)durationSeconds * 1000UL;

    // Le temps est écoulé → on change de mode
    if (elapsed >= durationMs) {
        this->state.lastModeChangeMs = millis(); // Réinitialise le chrono
        this->state.modePhase++;                 // Passe à la phase suivante

        // Phase paire = Scatter, phase impaire = Chase
        GhostAiMode newMode = (this->state.modePhase % 2 == 0) ? GM_Scatter : GM_Chase;

        for (uint8_t i = 0; i < GHOSTS_COUNT; i++)
        {
            this->ghosts[i].setAiMode(newMode);
        }
        
    }
}

void Game::registerInputs(inputs &newInputs) {
    this->currentInputs = newInputs;
};

void Game::computePacmanPosition() {}