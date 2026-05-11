#include "game.hpp"

Game::Game(/* args */) {};
Game::~Game() = default;

void Game::start() {
    this->state = gameState();
    this->pacmanPosition = {20, 20};
    this->blueGhostPosition = {0, 0};
    this->redGhostPosition = {1, 0};
    this->pinkGhostPosition = {2, 0};
    this->orangeGhostPosition = {3, 0};
};

gameState& Game::step() {
    this->state.tick++;

    // Compute new positions for all entities
    this->computePacmanPosition();
    

    // In the end return the new state to be printed on the screen
    return this->state;
};

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
        case ENT_RED_GHOST:
            this->redGhostPosition = {new_x, new_y};
            break;
        case ENT_BLUE_GHOST:
            this->blueGhostPosition = {new_x, new_y};
            break;
        case ENT_PINK_GHOST:
            this->pinkGhostPosition = {new_x, new_y};
            break;
        case ENT_ORANGE_GHOST:
            this->orangeGhostPosition = {new_x, new_y};
            break;
        default:
            break;
        }
        return 0;
    }
};


///////////////////////////////////////////
// DEBUG ONLY
///////////////////////////////////////////

static uint8_t lfsrRandomDirection() {
    static uint16_t state = 0xACE1;

    uint16_t bit = state & 1;
    state >>= 1;
    if (bit != 0) {
        state ^= 0xB400;
    }

    return (uint8_t)(state & 0x03);
}

void Game::computePacmanPosition() {
    int touchWall = 1;
    
    // Reroll until the next position doesn't touch a wall.
    while (touchWall != 0) {
        uint8_t direction = lfsrRandomDirection();
        
        switch (direction) {
            case 0:
                touchWall = this->moveEntity(ENT_PACMAN, this->pacmanPosition.x, this->pacmanPosition.y, this->pacmanPosition.x, this->pacmanPosition.y + 1);
                break;
            case 1:
                touchWall = this->moveEntity(ENT_PACMAN, this->pacmanPosition.x, this->pacmanPosition.y, this->pacmanPosition.x + 1, this->pacmanPosition.y);
                break;
            case 2:
                touchWall = this->moveEntity(ENT_PACMAN, this->pacmanPosition.x, this->pacmanPosition.y, this->pacmanPosition.x, this->pacmanPosition.y - 1);
                break;
            case 3:
                touchWall = this->moveEntity(ENT_PACMAN, this->pacmanPosition.x, this->pacmanPosition.y, this->pacmanPosition.x - 1, this->pacmanPosition.y);
                break;
        }
    }
}

// Niveau généré par Pac-Man Level Editor
// Taille : 28×36

void Game::loadLevel(char level) {
    memset(state.grid, 0, sizeof(state.grid));

    this->state.grid[0][0].setBackground(BG_WALL);
    this->state.grid[1][0].setBackground(BG_WALL);
    this->state.grid[2][0].setBackground(BG_WALL);
    this->state.grid[3][0].setBackground(BG_WALL);
    this->state.grid[4][0].setBackground(BG_WALL);
    this->state.grid[5][0].setBackground(BG_WALL);
    this->state.grid[6][0].setBackground(BG_WALL);
    this->state.grid[7][0].setBackground(BG_WALL);
    this->state.grid[8][0].setBackground(BG_WALL);
    this->state.grid[9][0].setBackground(BG_WALL);
    this->state.grid[10][0].setBackground(BG_WALL);
    this->state.grid[11][0].setBackground(BG_WALL);
    this->state.grid[12][0].setBackground(BG_WALL);
    this->state.grid[13][0].setBackground(BG_WALL);
    this->state.grid[14][0].setBackground(BG_WALL);
    this->state.grid[15][0].setBackground(BG_WALL);
    this->state.grid[16][0].setBackground(BG_WALL);
    this->state.grid[17][0].setBackground(BG_WALL);
    this->state.grid[18][0].setBackground(BG_WALL);
    this->state.grid[19][0].setBackground(BG_WALL);
    this->state.grid[20][0].setBackground(BG_WALL);
    this->state.grid[21][0].setBackground(BG_WALL);
    this->state.grid[22][0].setBackground(BG_WALL);
    this->state.grid[23][0].setBackground(BG_WALL);
    this->state.grid[24][0].setBackground(BG_WALL);
    this->state.grid[25][0].setBackground(BG_WALL);
    this->state.grid[26][0].setBackground(BG_WALL);
    this->state.grid[27][0].setBackground(BG_WALL);
    this->state.grid[0][1].setBackground(BG_WALL);
    this->state.grid[13][1].setBackground(BG_WALL);
    this->state.grid[14][1].setBackground(BG_WALL);
    this->state.grid[27][1].setBackground(BG_WALL);
    this->state.grid[0][2].setBackground(BG_WALL);
    this->state.grid[2][2].setBackground(BG_WALL);
    this->state.grid[3][2].setBackground(BG_WALL);
    this->state.grid[4][2].setBackground(BG_WALL);
    this->state.grid[5][2].setBackground(BG_WALL);
    this->state.grid[7][2].setBackground(BG_WALL);
    this->state.grid[8][2].setBackground(BG_WALL);
    this->state.grid[9][2].setBackground(BG_WALL);
    this->state.grid[10][2].setBackground(BG_WALL);
    this->state.grid[11][2].setBackground(BG_WALL);
    this->state.grid[13][2].setBackground(BG_WALL);
    this->state.grid[14][2].setBackground(BG_WALL);
    this->state.grid[16][2].setBackground(BG_WALL);
    this->state.grid[17][2].setBackground(BG_WALL);
    this->state.grid[18][2].setBackground(BG_WALL);
    this->state.grid[19][2].setBackground(BG_WALL);
    this->state.grid[20][2].setBackground(BG_WALL);
    this->state.grid[22][2].setBackground(BG_WALL);
    this->state.grid[23][2].setBackground(BG_WALL);
    this->state.grid[24][2].setBackground(BG_WALL);
    this->state.grid[25][2].setBackground(BG_WALL);
    this->state.grid[27][2].setBackground(BG_WALL);
    this->state.grid[0][3].setBackground(BG_WALL);
    this->state.grid[2][3].setBackground(BG_WALL);
    this->state.grid[3][3].setBackground(BG_WALL);
    this->state.grid[4][3].setBackground(BG_WALL);
    this->state.grid[5][3].setBackground(BG_WALL);
    this->state.grid[7][3].setBackground(BG_WALL);
    this->state.grid[8][3].setBackground(BG_WALL);
    this->state.grid[9][3].setBackground(BG_WALL);
    this->state.grid[10][3].setBackground(BG_WALL);
    this->state.grid[11][3].setBackground(BG_WALL);
    this->state.grid[13][3].setBackground(BG_WALL);
    this->state.grid[14][3].setBackground(BG_WALL);
    this->state.grid[16][3].setBackground(BG_WALL);
    this->state.grid[17][3].setBackground(BG_WALL);
    this->state.grid[18][3].setBackground(BG_WALL);
    this->state.grid[19][3].setBackground(BG_WALL);
    this->state.grid[20][3].setBackground(BG_WALL);
    this->state.grid[22][3].setBackground(BG_WALL);
    this->state.grid[23][3].setBackground(BG_WALL);
    this->state.grid[24][3].setBackground(BG_WALL);
    this->state.grid[25][3].setBackground(BG_WALL);
    this->state.grid[27][3].setBackground(BG_WALL);
    this->state.grid[0][4].setBackground(BG_WALL);
    this->state.grid[27][4].setBackground(BG_WALL);
    this->state.grid[0][5].setBackground(BG_WALL);
    this->state.grid[2][5].setBackground(BG_WALL);
    this->state.grid[3][5].setBackground(BG_WALL);
    this->state.grid[4][5].setBackground(BG_WALL);
    this->state.grid[5][5].setBackground(BG_WALL);
    this->state.grid[7][5].setBackground(BG_WALL);
    this->state.grid[8][5].setBackground(BG_WALL);
    this->state.grid[10][5].setBackground(BG_WALL);
    this->state.grid[11][5].setBackground(BG_WALL);
    this->state.grid[12][5].setBackground(BG_WALL);
    this->state.grid[13][5].setBackground(BG_WALL);
    this->state.grid[14][5].setBackground(BG_WALL);
    this->state.grid[15][5].setBackground(BG_WALL);
    this->state.grid[16][5].setBackground(BG_WALL);
    this->state.grid[17][5].setBackground(BG_WALL);
    this->state.grid[27][5].setBackground(BG_WALL);
    this->state.grid[0][6].setBackground(BG_WALL);
    this->state.grid[2][6].setBackground(BG_WALL);
    this->state.grid[3][6].setBackground(BG_WALL);
    this->state.grid[4][6].setBackground(BG_WALL);
    this->state.grid[5][6].setBackground(BG_WALL);
    this->state.grid[7][6].setBackground(BG_WALL);
    this->state.grid[8][6].setBackground(BG_WALL);
    this->state.grid[10][6].setBackground(BG_WALL);
    this->state.grid[11][6].setBackground(BG_WALL);
    this->state.grid[12][6].setBackground(BG_WALL);
    this->state.grid[13][6].setBackground(BG_WALL);
    this->state.grid[14][6].setBackground(BG_WALL);
    this->state.grid[15][6].setBackground(BG_WALL);
    this->state.grid[16][6].setBackground(BG_WALL);
    this->state.grid[17][6].setBackground(BG_WALL);
    this->state.grid[27][6].setBackground(BG_WALL);
    this->state.grid[0][7].setBackground(BG_WALL);
    this->state.grid[13][7].setBackground(BG_WALL);
    this->state.grid[14][7].setBackground(BG_WALL);
    this->state.grid[27][7].setBackground(BG_WALL);
    this->state.grid[0][8].setBackground(BG_WALL);
    this->state.grid[1][8].setBackground(BG_WALL);
    this->state.grid[2][8].setBackground(BG_WALL);
    this->state.grid[3][8].setBackground(BG_WALL);
    this->state.grid[4][8].setBackground(BG_WALL);
    this->state.grid[5][8].setBackground(BG_WALL);
    this->state.grid[7][8].setBackground(BG_WALL);
    this->state.grid[8][8].setBackground(BG_WALL);
    this->state.grid[13][8].setBackground(BG_WALL);
    this->state.grid[14][8].setBackground(BG_WALL);
    this->state.grid[20][8].setBackground(BG_WALL);
    this->state.grid[21][8].setBackground(BG_WALL);
    this->state.grid[27][8].setBackground(BG_WALL);
    this->state.grid[5][9].setBackground(BG_WALL);
    this->state.grid[7][9].setBackground(BG_WALL);
    this->state.grid[8][9].setBackground(BG_WALL);
    this->state.grid[13][9].setBackground(BG_WALL);
    this->state.grid[14][9].setBackground(BG_WALL);
    this->state.grid[19][9].setBackground(BG_WALL);
    this->state.grid[20][9].setBackground(BG_WALL);
    this->state.grid[27][9].setBackground(BG_WALL);
    this->state.grid[5][10].setBackground(BG_WALL);
    this->state.grid[7][10].setBackground(BG_WALL);
    this->state.grid[8][10].setBackground(BG_WALL);
    this->state.grid[9][10].setBackground(BG_WALL);
    this->state.grid[10][10].setBackground(BG_WALL);
    this->state.grid[11][10].setBackground(BG_WALL);
    this->state.grid[13][10].setBackground(BG_WALL);
    this->state.grid[14][10].setBackground(BG_WALL);
    this->state.grid[19][10].setBackground(BG_WALL);
    this->state.grid[27][10].setBackground(BG_WALL);
    this->state.grid[5][11].setBackground(BG_WALL);
    this->state.grid[7][11].setBackground(BG_WALL);
    this->state.grid[8][11].setBackground(BG_WALL);
    this->state.grid[9][11].setBackground(BG_WALL);
    this->state.grid[10][11].setBackground(BG_WALL);
    this->state.grid[11][11].setBackground(BG_WALL);
    this->state.grid[19][11].setBackground(BG_WALL);
    this->state.grid[27][11].setBackground(BG_WALL);
    this->state.grid[5][12].setBackground(BG_WALL);
    this->state.grid[7][12].setBackground(BG_WALL);
    this->state.grid[8][12].setBackground(BG_WALL);
    this->state.grid[27][12].setBackground(BG_WALL);
    this->state.grid[0][13].setBackground(BG_WALL);
    this->state.grid[1][13].setBackground(BG_WALL);
    this->state.grid[2][13].setBackground(BG_WALL);
    this->state.grid[3][13].setBackground(BG_WALL);
    this->state.grid[4][13].setBackground(BG_WALL);
    this->state.grid[5][13].setBackground(BG_WALL);
    this->state.grid[7][13].setBackground(BG_WALL);
    this->state.grid[8][13].setBackground(BG_WALL);
    this->state.grid[10][13].setBackground(BG_WALL);
    this->state.grid[11][13].setBackground(BG_WALL);
    this->state.grid[12][13].setBackground(BG_WALL);
    this->state.grid[15][13].setBackground(BG_WALL);
    this->state.grid[27][13].setBackground(BG_WALL);
    this->state.grid[0][14].setBackground(BG_WALL);
    this->state.grid[27][14].setBackground(BG_WALL);
    this->state.grid[0][15].setBackground(BG_WALL);
    this->state.grid[27][15].setBackground(BG_WALL);
    this->state.grid[0][16].setBackground(BG_WALL);
    this->state.grid[3][16].setBackground(BG_WALL);
    this->state.grid[4][16].setBackground(BG_WALL);
    this->state.grid[5][16].setBackground(BG_WALL);
    this->state.grid[9][16].setBackground(BG_WALL);
    this->state.grid[10][16].setBackground(BG_WALL);
    this->state.grid[11][16].setBackground(BG_WALL);
    this->state.grid[14][16].setBackground(BG_WALL);
    this->state.grid[15][16].setBackground(BG_WALL);
    this->state.grid[16][16].setBackground(BG_WALL);
    this->state.grid[17][16].setBackground(BG_WALL);
    this->state.grid[20][16].setBackground(BG_WALL);
    this->state.grid[21][16].setBackground(BG_WALL);
    this->state.grid[22][16].setBackground(BG_WALL);
    this->state.grid[23][16].setBackground(BG_WALL);
    this->state.grid[27][16].setBackground(BG_WALL);
    this->state.grid[0][17].setBackground(BG_WALL);
    this->state.grid[3][17].setBackground(BG_WALL);
    this->state.grid[4][17].setBackground(BG_WALL);
    this->state.grid[5][17].setBackground(BG_WALL);
    this->state.grid[9][17].setBackground(BG_WALL);
    this->state.grid[10][17].setBackground(BG_WALL);
    this->state.grid[11][17].setBackground(BG_WALL);
    this->state.grid[14][17].setBackground(BG_WALL);
    this->state.grid[15][17].setBackground(BG_WALL);
    this->state.grid[16][17].setBackground(BG_WALL);
    this->state.grid[17][17].setBackground(BG_WALL);
    this->state.grid[20][17].setBackground(BG_WALL);
    this->state.grid[21][17].setBackground(BG_WALL);
    this->state.grid[22][17].setBackground(BG_WALL);
    this->state.grid[23][17].setBackground(BG_WALL);
    this->state.grid[27][17].setBackground(BG_WALL);
    this->state.grid[0][18].setBackground(BG_WALL);
    this->state.grid[20][18].setBackground(BG_WALL);
    this->state.grid[21][18].setBackground(BG_WALL);
    this->state.grid[22][18].setBackground(BG_WALL);
    this->state.grid[23][18].setBackground(BG_WALL);
    this->state.grid[27][18].setBackground(BG_WALL);
    this->state.grid[0][19].setBackground(BG_WALL);
    this->state.grid[27][19].setBackground(BG_WALL);
    this->state.grid[0][20].setBackground(BG_WALL);
    this->state.grid[27][20].setBackground(BG_WALL);
    this->state.grid[0][21].setBackground(BG_WALL);
    this->state.grid[8][21].setBackground(BG_WALL);
    this->state.grid[9][21].setBackground(BG_WALL);
    this->state.grid[27][21].setBackground(BG_WALL);
    this->state.grid[0][22].setBackground(BG_WALL);
    this->state.grid[8][22].setBackground(BG_WALL);
    this->state.grid[10][22].setBackground(BG_WALL);
    this->state.grid[18][22].setBackground(BG_WALL);
    this->state.grid[19][22].setBackground(BG_WALL);
    this->state.grid[27][22].setBackground(BG_WALL);
    this->state.grid[0][23].setBackground(BG_WALL);
    this->state.grid[7][23].setBackground(BG_WALL);
    this->state.grid[11][23].setBackground(BG_WALL);
    this->state.grid[18][23].setBackground(BG_WALL);
    this->state.grid[20][23].setBackground(BG_WALL);
    this->state.grid[27][23].setBackground(BG_WALL);
    this->state.grid[0][24].setBackground(BG_WALL);
    this->state.grid[6][24].setBackground(BG_WALL);
    this->state.grid[12][24].setBackground(BG_WALL);
    this->state.grid[13][24].setBackground(BG_WALL);
    this->state.grid[14][24].setBackground(BG_WALL);
    this->state.grid[15][24].setBackground(BG_WALL);
    this->state.grid[16][24].setBackground(BG_WALL);
    this->state.grid[17][24].setBackground(BG_WALL);
    this->state.grid[20][24].setBackground(BG_WALL);
    this->state.grid[27][24].setBackground(BG_WALL);
    this->state.grid[0][25].setBackground(BG_WALL);
    this->state.grid[6][25].setBackground(BG_WALL);
    this->state.grid[10][25].setBackground(BG_WALL);
    this->state.grid[16][25].setBackground(BG_WALL);
    this->state.grid[20][25].setBackground(BG_WALL);
    this->state.grid[27][25].setBackground(BG_WALL);
    this->state.grid[0][26].setBackground(BG_WALL);
    this->state.grid[6][26].setBackground(BG_WALL);
    this->state.grid[7][26].setBackground(BG_WALL);
    this->state.grid[18][26].setBackground(BG_WALL);
    this->state.grid[20][26].setBackground(BG_WALL);
    this->state.grid[27][26].setBackground(BG_WALL);
    this->state.grid[0][27].setBackground(BG_WALL);
    this->state.grid[7][27].setBackground(BG_WALL);
    this->state.grid[20][27].setBackground(BG_WALL);
    this->state.grid[27][27].setBackground(BG_WALL);
    this->state.grid[0][28].setBackground(BG_WALL);
    this->state.grid[7][28].setBackground(BG_WALL);
    this->state.grid[8][28].setBackground(BG_WALL);
    this->state.grid[19][28].setBackground(BG_WALL);
    this->state.grid[20][28].setBackground(BG_WALL);
    this->state.grid[27][28].setBackground(BG_WALL);
    this->state.grid[0][29].setBackground(BG_WALL);
    this->state.grid[8][29].setBackground(BG_WALL);
    this->state.grid[19][29].setBackground(BG_WALL);
    this->state.grid[27][29].setBackground(BG_WALL);
    this->state.grid[0][30].setBackground(BG_WALL);
    this->state.grid[8][30].setBackground(BG_WALL);
    this->state.grid[9][30].setBackground(BG_WALL);
    this->state.grid[10][30].setBackground(BG_WALL);
    this->state.grid[12][30].setBackground(BG_WALL);
    this->state.grid[13][30].setBackground(BG_WALL);
    this->state.grid[14][30].setBackground(BG_WALL);
    this->state.grid[15][30].setBackground(BG_WALL);
    this->state.grid[16][30].setBackground(BG_WALL);
    this->state.grid[17][30].setBackground(BG_WALL);
    this->state.grid[18][30].setBackground(BG_WALL);
    this->state.grid[27][30].setBackground(BG_WALL);
    this->state.grid[0][31].setBackground(BG_WALL);
    this->state.grid[3][31].setEntity(ENT_PACMAN);
    this->state.grid[10][31].setBackground(BG_WALL);
    this->state.grid[11][31].setBackground(BG_WALL);
    this->state.grid[27][31].setBackground(BG_WALL);
    this->state.grid[0][32].setBackground(BG_WALL);
    this->state.grid[27][32].setBackground(BG_WALL);
    this->state.grid[0][33].setBackground(BG_WALL);
    this->state.grid[27][33].setBackground(BG_WALL);
    this->state.grid[0][34].setBackground(BG_WALL);
    this->state.grid[27][34].setBackground(BG_WALL);
    this->state.grid[0][35].setBackground(BG_WALL);
    this->state.grid[1][35].setBackground(BG_WALL);
    this->state.grid[2][35].setBackground(BG_WALL);
    this->state.grid[3][35].setBackground(BG_WALL);
    this->state.grid[4][35].setBackground(BG_WALL);
    this->state.grid[5][35].setBackground(BG_WALL);
    this->state.grid[6][35].setBackground(BG_WALL);
    this->state.grid[7][35].setBackground(BG_WALL);
    this->state.grid[8][35].setBackground(BG_WALL);
    this->state.grid[9][35].setBackground(BG_WALL);
    this->state.grid[10][35].setBackground(BG_WALL);
    this->state.grid[11][35].setBackground(BG_WALL);
    this->state.grid[12][35].setBackground(BG_WALL);
    this->state.grid[13][35].setBackground(BG_WALL);
    this->state.grid[14][35].setBackground(BG_WALL);
    this->state.grid[15][35].setBackground(BG_WALL);
    this->state.grid[16][35].setBackground(BG_WALL);
    this->state.grid[17][35].setBackground(BG_WALL);
    this->state.grid[18][35].setBackground(BG_WALL);
    this->state.grid[19][35].setBackground(BG_WALL);
    this->state.grid[20][35].setBackground(BG_WALL);
    this->state.grid[21][35].setBackground(BG_WALL);
    this->state.grid[22][35].setBackground(BG_WALL);
    this->state.grid[23][35].setBackground(BG_WALL);
    this->state.grid[24][35].setBackground(BG_WALL);
    this->state.grid[25][35].setBackground(BG_WALL);
    this->state.grid[26][35].setBackground(BG_WALL);
    this->state.grid[27][35].setBackground(BG_WALL);
}
