# editor/constants.py
# Shared constants mirroring the Arduino C++ defines and game rules.

# ── Grid dimensions (must match Arduino types.hpp) ───────────────────────────
GRID_W = 28   # GAME_GRID_X_AXIS_LEN  (columns, horizontal axis)
GRID_H = 36   # GAME_GRID_Y_AXIS_LEN  (rows,    vertical axis)

# ── Zoom controls ─────────────────────────────────────────────────────────────
CELL_SIZE_MIN     =  6   # px minimum per cell
CELL_SIZE_DEFAULT = 18   # px default
CELL_SIZE_MAX     = 48   # px maximum per cell
ZOOM_STEP         =  2   # zoom increment in px

# ── Game rules (Phase 2) ─────────────────────────────────────────────────────
MAX_DOTS          = 255  # Maximum collectible cells (BG_GUM + BG_ENERGIZE) per level.
                         # Enforced because totalDots is stored as uint8_t on the Arduino.
MAX_GHOSTS        = 4    # One per personality: Red, Pink, Blue, Orange.
MAX_PACMAN        = 1    # Exactly one Pac-Man spawn point.
MAX_FRUIT         = 1    # At most one fruit spawn point.
