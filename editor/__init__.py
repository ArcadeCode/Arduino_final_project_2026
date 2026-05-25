# editor/__init__.py
# Public API of the editor package.
# Import everything a caller might need from a single place.

from .constants import (
    GRID_W, GRID_H,
    CELL_SIZE_MIN, CELL_SIZE_DEFAULT, CELL_SIZE_MAX, ZOOM_STEP,
    MAX_DOTS, MAX_GHOSTS, MAX_PACMAN, MAX_FRUIT,
)
from .models    import BG, ENT, Cell, empty_grid
from .rules     import validate_grid, count_dots
from .serialization import save_json, load_json, JSON_VERSION
from .converters import (
    grid_to_cpp,
    grid_to_ascii, ascii_to_grid,
    grid_to_raw_array, raw_array_to_grid,
    bg_cpp_name, ent_cpp_name,
)
