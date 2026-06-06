# editor/levelset.py
# Phase 3 — LevelSet data model and .levels.json persistence.
#
# A LevelSet groups up to MAX_LEVELS individual levels under a single file.
# Each level is stored exactly like a standalone v2 level JSON, embedded inside
# the "levels" array.  The outer envelope adds name, version, and level count.
#
# File format — .levels.json v1:
#   {
#     "version":      1,
#     "name":         "My Game",
#     "level_count":  N,            # redundant but useful for quick inspection
#     "levels": [
#       {
#         "name":       "Level 1",
#         "version":    2,          # inner level format (same as standalone .json v2)
#         "width":      28,
#         "height":     36,
#         "total_dots": ...,
#         "grid":       [[...], ...]
#       },
#       ...
#     ]
#   }
#
# Individual levels can still be saved / loaded as standalone .json v2 files
# and imported into a LevelSet at any position.

import json
from dataclasses import dataclass, field
from typing import List, Tuple

from .constants import GRID_W, GRID_H, MAX_LEVELS
from .models import Cell, empty_grid
from .rules import count_dots

# Outer envelope version (the per-level inner version remains 2).
LEVELSET_VERSION = 1

# Fallback name used for levels that have no explicit name (e.g. plain .json imports).
DEFAULT_LEVEL_NAME = "Level"


# ── LevelSet ─────────────────────────────────────────────────────────────────

@dataclass
class LevelSet:
    """
    Ordered collection of up to MAX_LEVELS grids with their display names.

    Attributes:
        name     — human-readable name for the whole set (shown in the title bar).
        levels   — list of (level_name: str, grid: List[List[Cell]]) tuples.
                   Always has at least one entry; never more than MAX_LEVELS.
    """
    name:   str
    levels: List[Tuple[str, List[List[Cell]]]]

    # ── Factory helpers ───────────────────────────────────────────────────────

    @classmethod
    def new(cls, set_name: str = "New Level Set") -> "LevelSet":
        """Create a fresh level set containing one empty level."""
        return cls(
            name=set_name,
            levels=[("Level 1", empty_grid())],
        )

    # ── Convenience accessors ─────────────────────────────────────────────────

    def __len__(self) -> int:
        return len(self.levels)

    def get_grid(self, index: int) -> List[List[Cell]]:
        return self.levels[index][1]

    def get_name(self, index: int) -> str:
        return self.levels[index][0]

    def set_name(self, index: int, name: str) -> None:
        grid = self.levels[index][1]
        self.levels[index] = (name, grid)

    def set_grid(self, index: int, grid: List[List[Cell]]) -> None:
        name = self.levels[index][0]
        self.levels[index] = (name, grid)

    # ── Mutation helpers (used by the level manager panel) ────────────────────

    def add_level(self, name: str = "", grid: List[List[Cell]] | None = None) -> int:
        """
        Append a new level and return its index.
        Raises ValueError if MAX_LEVELS would be exceeded.
        """
        if len(self.levels) >= MAX_LEVELS:
            raise ValueError(
                f"Cannot add level: maximum of {MAX_LEVELS} levels already reached."
            )
        if not name:
            name = f"Level {len(self.levels) + 1}"
        self.levels.append((name, grid if grid is not None else empty_grid()))
        return len(self.levels) - 1

    def remove_level(self, index: int) -> None:
        """
        Remove the level at *index*.
        The set always keeps at least one level; raises ValueError otherwise.
        """
        if len(self.levels) <= 1:
            raise ValueError("Cannot remove the last level in the set.")
        self.levels.pop(index)

    def move_up(self, index: int) -> int:
        """Swap level at *index* with the one above; return the new index."""
        if index <= 0:
            return index
        self.levels[index - 1], self.levels[index] = (
            self.levels[index], self.levels[index - 1]
        )
        return index - 1

    def move_down(self, index: int) -> int:
        """Swap level at *index* with the one below; return the new index."""
        if index >= len(self.levels) - 1:
            return index
        self.levels[index], self.levels[index + 1] = (
            self.levels[index + 1], self.levels[index]
        )
        return index + 1

    def duplicate_level(self, index: int) -> int:
        """
        Insert a deep copy of level *index* immediately after it.
        Returns the index of the new copy.
        Raises ValueError if MAX_LEVELS would be exceeded.
        """
        if len(self.levels) >= MAX_LEVELS:
            raise ValueError(
                f"Cannot duplicate: maximum of {MAX_LEVELS} levels already reached."
            )
        src_name, src_grid = self.levels[index]
        copy_grid = [[src_grid[x][y].copy() for y in range(GRID_H)]
                     for x in range(GRID_W)]
        new_name = f"{src_name} (copy)"
        self.levels.insert(index + 1, (new_name, copy_grid))
        return index + 1


# ── Serialization ─────────────────────────────────────────────────────────────

def _grid_to_raw(grid: List[List[Cell]]) -> list:
    """Serialize a grid to the nested list format used by both v1 and v2 JSON."""
    return [
        [grid[x][y].data for x in range(GRID_W)]
        for y in range(GRID_H)
    ]


def _raw_to_grid(raw: list) -> List[List[Cell]]:
    """Deserialize a nested list (v1/v2 format: outer = y) into a grid."""
    grid = empty_grid()
    for y in range(min(GRID_H, len(raw))):
        for x in range(min(GRID_W, len(raw[y]))):
            grid[x][y] = Cell(raw[y][x])
    return grid


def save_levelset(path: str, levelset: LevelSet) -> None:
    """
    Serialize a LevelSet to a .levels.json file (format v1).

    Each level is embedded as a full v2 level object (same layout as a
    standalone .json level file) so individual levels can be copy-pasted.
    """
    level_objs = []
    for name, grid in levelset.levels:
        level_objs.append({
            "name":       name,
            "version":    2,
            "width":      GRID_W,
            "height":     GRID_H,
            "total_dots": count_dots(grid),
            "grid":       _grid_to_raw(grid),
        })

    payload = {
        "version":     LEVELSET_VERSION,
        "name":        levelset.name,
        "level_count": len(levelset.levels),
        "levels":      level_objs,
    }
    with open(path, "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)


def load_levelset(path: str) -> Tuple[LevelSet, List[str]]:
    """
    Deserialize a .levels.json file.

    Returns:
        levelset  — the loaded LevelSet (always has ≥ 1 level)
        warnings  — list of human-readable warning strings
    """
    warnings: List[str] = []

    with open(path, encoding="utf-8") as f:
        obj = json.load(f)

    outer_ver = obj.get("version", 0)
    if outer_ver != LEVELSET_VERSION:
        warnings.append(
            f"Unknown level-set format version ({outer_ver}). "
            "Attempting to load anyway."
        )

    set_name = obj.get("name", "Imported Level Set")
    raw_levels = obj.get("levels", [])

    if not raw_levels:
        warnings.append("No levels found in the file. An empty level was created.")
        return LevelSet.new(set_name), warnings

    loaded: List[Tuple[str, List[List[Cell]]]] = []
    for i, lobj in enumerate(raw_levels[:MAX_LEVELS]):
        lname   = lobj.get("name", f"{DEFAULT_LEVEL_NAME} {i + 1}")
        raw     = lobj.get("grid", [])
        lver    = lobj.get("version", 1)

        if lver >= 1:
            grid = _raw_to_grid(raw)
        else:
            # v0 legacy: outer = x (axes swapped)
            grid = empty_grid()
            for x in range(min(GRID_W, len(raw))):
                for y in range(min(GRID_H, len(raw[x]))):
                    grid[x][y] = Cell(raw[x][y])
            warnings.append(
                f"Level {i + 1} ({lname!r}): legacy v0 format detected "
                "(swapped axes). Converted automatically."
            )
        loaded.append((lname, grid))

    if len(raw_levels) > MAX_LEVELS:
        warnings.append(
            f"File contains {len(raw_levels)} levels but the maximum is "
            f"{MAX_LEVELS}. Only the first {MAX_LEVELS} were loaded."
        )

    return LevelSet(name=set_name, levels=loaded), warnings


# ── Individual-level import/export (bridge to standalone .json) ───────────────

def export_level_json(path: str, name: str, grid: List[List[Cell]]) -> None:
    """
    Save a single level as a standalone .json v2 file.
    Identical to serialization.save_json() but adds a "name" field.
    """
    payload = {
        "name":       name,
        "version":    2,
        "width":      GRID_W,
        "height":     GRID_H,
        "total_dots": count_dots(grid),
        "grid":       _grid_to_raw(grid),
    }
    with open(path, "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)


def import_level_json(path: str) -> Tuple[str, List[List[Cell]], List[str]]:
    """
    Load a single level from a standalone .json file (v0, v1, or v2).

    Returns:
        name      — level name (from "name" field, or filename stem)
        grid      — loaded grid
        warnings  — list of warning strings
    """
    import os
    warnings: List[str] = []

    with open(path, encoding="utf-8") as f:
        obj = json.load(f)

    # Derive a display name: prefer the embedded field, fall back to filename.
    stem = os.path.splitext(os.path.basename(path))[0]
    name = obj.get("name", stem)

    raw     = obj.get("grid", [])
    version = obj.get("version", 0)

    if version >= 1:
        grid = _raw_to_grid(raw)
    else:
        grid = empty_grid()
        for x in range(min(GRID_W, len(raw))):
            for y in range(min(GRID_H, len(raw[x]))):
                grid[x][y] = Cell(raw[x][y])
        warnings.append(
            f"Legacy format v0 detected in {os.path.basename(path)} (swapped axes). "
            "Converted automatically."
        )

    return name, grid, warnings
