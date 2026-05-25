# editor/serialization.py
# JSON persistence for levels.
#
# File format versions:
#   v0  (legacy)  — grid[x][y]: first dimension = x (column).  Axes were swapped.
#   v1            — grid[y][x]: first dimension = y (row).  Matches C++ Cell[][].
#   v2  (current) — same layout as v1, adds "total_dots" field (Phase 2).
#
# save_json() always writes v2.
# load_json() handles v0, v1, and v2 transparently.

import json
from typing import List, Tuple

from .constants import GRID_W, GRID_H
from .models import Cell, empty_grid
from .rules import count_dots

# Current format version produced by this editor.
JSON_VERSION = 2


def save_json(path: str, grid: List[List[Cell]]) -> None:
    """
    Serialize the grid to a JSON file (format v2).

    Layout: data[y][x] — outer index = row (y), inner index = column (x).
    This matches the C++ Cell grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN] convention.

    The "total_dots" field stores the pre-computed dot count so the Arduino
    sketch can load it without scanning the whole background array at startup.
    """
    data = [
        [grid[x][y].data for x in range(GRID_W)]
        for y in range(GRID_H)
    ]
    payload = {
        "version":    JSON_VERSION,
        "width":      GRID_W,
        "height":     GRID_H,
        "total_dots": count_dots(grid),   # Phase 2: dot count cached in the file
        "grid":       data,
    }
    with open(path, "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)


def load_json(path: str) -> Tuple[List[List[Cell]], int, List[str]]:
    """
    Deserialize a level JSON file (supports v0, v1, v2).

    Returns:
        grid      — loaded Cell[x][y] grid
        version   — file format version that was read
        warnings  — list of human-readable warning strings (e.g. legacy format notice)
    """
    warnings: List[str] = []

    with open(path, encoding="utf-8") as f:
        obj = json.load(f)

    raw     = obj["grid"]
    version = obj.get("version", 0)
    grid    = empty_grid()

    if version >= 1:
        # v1 / v2: raw[y][x] — outer = row
        for y in range(min(GRID_H, len(raw))):
            for x in range(min(GRID_W, len(raw[y]))):
                grid[x][y] = Cell(raw[y][x])
    else:
        # v0 (legacy): raw[x][y] — outer = column (axes were swapped)
        for x in range(min(GRID_W, len(raw))):
            for y in range(min(GRID_H, len(raw[x]))):
                grid[x][y] = Cell(raw[x][y])
        warnings.append(
            "Legacy format v0 detected (swapped axes).\n"
            "The level was loaded correctly and will be saved as v2."
        )

    return grid, version, warnings
