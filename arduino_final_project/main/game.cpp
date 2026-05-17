#include "game.hpp"

Game::Game()
    : blueGhost(&this->state, GP_BLUE),
      redGhost(&this->state, GP_RED),
      pinkGhost(&this->state, GP_PINK),
      orangeGhost(&this->state, GP_ORANGE)
{};

Game::~Game() = default;

void Game::start() {
    // Direct in-place reset of the existing heap-allocated GameState, avoiding
    // a 1008 bytes stack-allocated temporary that would cause a stack overflow.
    // thank C++ for that !
    this->state.tick = 0;
    memset(this->state.grid, 0, sizeof(this->state.grid));

    this->pacmanPosition = {20, 20}; // Dummy position for pacman, we will change it in loadLevel() with the real position of pacman.
    this->pacmanFacing = EF_WEST; // Dummy facing for pacman, can be changed in loadLevel() with the real facing of pacman.
    // TODO:: Implement starting facing for pacman in the editor.

    // Initialize ghosts is done in the constructor due to presence of consts in them.
    // Initializing ghosts facing to dummy value based on the official first level, can be changed in loadLevel() with the real starting facing of each ghost.
    // TODO:: Implement starting facing for ghosts in the editor.
    this->blueGhost.setFacing(EF_NORTH);
    this->redGhost.setFacing(EF_WEST);
    this->pinkGhost.setFacing(EF_SOUTH);
    this->orangeGhost.setFacing(EF_WEST);
};

GameState& Game::step() {
    this->state.tick++;

    // Share entities positions to the global state for easier access to ghosts AI movement, instead of searching the grid for them.
    this->state.pacmanPosition      = this->pacmanPosition;
    this->state.pacmanFacing        = this->pacmanFacing;
    this->state.blueGhostPosition   = this->blueGhost.getPosition();
    this->state.redGhostPosition    = this->redGhost.getPosition();
    this->state.pinkGhostPosition   = this->pinkGhost.getPosition();
    this->state.orangeGhostPosition = this->orangeGhost.getPosition();

    // Compute new positions for all entities
    this->computePacmanPosition();

    this->blueGhost.computeNewPosition();
    this->redGhost.computeNewPosition();
    this->pinkGhost.computeNewPosition();
    this->orangeGhost.computeNewPosition();

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

        this->blueGhost.setAiMode(newMode);
        this->redGhost.setAiMode(newMode);
        this->pinkGhost.setAiMode(newMode);
        this->orangeGhost.setAiMode(newMode);
    }
}

void Game::registerInputs(inputs &newInputs) {
    this->currentInputs = newInputs;
};

int Game::moveEntity(CellEntitiesType Entity, uint8_t old_x, uint8_t old_y, uint8_t new_x, uint8_t new_y) {
    // Try to move an entity, fail if the new cell contain a wall
    if (this->state.grid[new_y][new_x].getBackground() == BG_WALL) {
        return 1; // Error, the new position contain a wall.
    } else if(new_x >= GAME_GRID_X_AXIS_LEN || new_y >= GAME_GRID_Y_AXIS_LEN) {
        return 2; // Out of the grid
    } else {
        this->state.grid[new_y][new_x].setEntity(Entity);
        this->state.grid[old_y][old_x].setEntity(ENT_EMPTY);
        switch (Entity) {
        case ENT_PACMAN:
            this->pacmanPosition = {new_x, new_y};
            break;
        default:
            break;
        }
        return 0;
    }
};

// TODO: Move this function to a better place, it's used by ghosts more than game.
static uint8_t lfsrRandomDirection() {
    static uint16_t state = 0xACE1;

    uint16_t bit = state & 1;
    state >>= 1;
    if (bit != 0) {
        state ^= 0xB400;
    }

    return (uint8_t)(state & 0x03);
}

void Game::computePacmanPosition() {}