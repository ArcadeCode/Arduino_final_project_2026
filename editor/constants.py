"""
constants.py — All compile-time constants for the Pac-Man Level Editor.

Single source of truth: every magic number and visual colour lives here.
The C++ side mirrors the grid/enum values.
"""

from models import BG, ENT

# ── Grid dimensions (must match C++ GAME_GRID_X/Y_AXIS_LEN) ─────────────────

GRID_W: int = 28   # columns (x-axis)
GRID_H: int = 36   # rows    (y-axis)

# ── Cell size / zoom ─────────────────────────────────────────────────────────

CELL_SIZE_MIN:     int = 6    # minimum pixels per cell
CELL_SIZE_DEFAULT: int = 18   # pixels per cell at startup
CELL_SIZE_MAX:     int = 48   # maximum pixels per cell
ZOOM_STEP:         int = 2    # pixels added/removed per zoom step

# ── Placement rules ───────────────────────────────────────────────────────────

MAX_GHOSTS: int = 4   # all ghost colors combined
MAX_PACMAN: int = 1
MAX_FRUIT:  int = 1

# ── Visual palette: (BG, ENT) → (hex fill, canvas symbol, human label) ───────

CELL_COLORS: dict = {
    (BG.EMPTY,    ENT.EMPTY):        ("#0A0A1A", "",  "Empty"),
    (BG.WALL,     ENT.EMPTY):        ("#1A3A8F", "",  "Wall"),
    (BG.GUM,      ENT.EMPTY):        ("#1A1A2E", "·", "Pac-gum"),
    (BG.ENERGIZE, ENT.EMPTY):        ("#1A1A2E", "●", "Power pellet"),
    (BG.EMPTY,    ENT.PACMAN):       ("#FFD700", "P", "Pac-Man"),
    (BG.EMPTY,    ENT.RED_GHOST):    ("#FF3333", "R", "Red Ghost"),
    (BG.EMPTY,    ENT.BLUE_GHOST):   ("#33CCFF", "B", "Blue Ghost"),
    (BG.EMPTY,    ENT.PINK_GHOST):   ("#FF88CC", "K", "Pink Ghost"),
    (BG.EMPTY,    ENT.ORANGE_GHOST): ("#FF8800", "O", "Orange Ghost"),
    (BG.EMPTY,    ENT.FRUIT):        ("#FF4488", "F", "Fruit"),
}

# ── Ordered palette items rendered in the left sidebar ────────────────────────

PALETTE_ITEMS: list = [
    ("Empty",         BG.EMPTY,    ENT.EMPTY),
    ("Wall",          BG.WALL,     ENT.EMPTY),
    ("Pac-gum",       BG.GUM,      ENT.EMPTY),
    ("Power pellet",  BG.ENERGIZE, ENT.EMPTY),
    ("Pac-Man",       BG.EMPTY,    ENT.PACMAN),
    ("Red Ghost",     BG.EMPTY,    ENT.RED_GHOST),
    ("Blue Ghost",    BG.EMPTY,    ENT.BLUE_GHOST),
    ("Pink Ghost",    BG.EMPTY,    ENT.PINK_GHOST),
    ("Orange Ghost",  BG.EMPTY,    ENT.ORANGE_GHOST),
    ("Fruit",         BG.EMPTY,    ENT.FRUIT),
]
