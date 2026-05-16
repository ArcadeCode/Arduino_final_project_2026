"""
rules.py — Placement validation rules for the level editor.

All rule-checking logic lives here so the painting code stays clean.
Rules are enforced before a cell is painted, not after.
"""

from typing import List, Tuple, Optional

from models import Cell, ENT, GHOST_ENTITIES, BG
from constants import GRID_W, GRID_H, MAX_GHOSTS, MAX_PACMAN, MAX_FRUIT


# ── Public API ────────────────────────────────────────────────────────────────

def can_place(
    grid: List[List[Cell]],
    x: int,
    y: int,
    new_bg: BG,
    new_ent: ENT,
) -> Tuple[bool, Optional[str]]:
    """
    Check whether placing (new_bg, new_ent) at (x, y) is allowed.

    Returns:
        (True, None)          — placement is valid.
        (False, reason_str)   — placement violates a rule; reason_str explains why.

    The cell at (x, y) is considered *replaced*, so its current entity
    is excluded from the counts before checking the new one.
    """
    if new_ent == ENT.EMPTY:
        # Removing / background-only placement is always valid.
        return True, None

    current_ent = grid[x][y].get_ent()

    if new_ent == ENT.PACMAN:
        count = _count_entity(grid, ENT.PACMAN, exclude=(x, y))
        if count >= MAX_PACMAN:
            return False, f"Only {MAX_PACMAN} Pac-Man allowed on the grid."

    elif new_ent == ENT.FRUIT:
        count = _count_entity(grid, ENT.FRUIT, exclude=(x, y))
        if count >= MAX_FRUIT:
            return False, f"Only {MAX_FRUIT} fruit allowed on the grid."

    elif new_ent in GHOST_ENTITIES:
        count = _count_ghosts(grid, exclude=(x, y))
        if count >= MAX_GHOSTS:
            return False, (
                f"Only {MAX_GHOSTS} ghosts (of any colour) allowed on the grid."
            )

    return True, None


# ── Helpers ──────────────────────────────────────────────────────────────────

def _count_entity(
    grid: List[List[Cell]],
    ent: ENT,
    exclude: Optional[Tuple[int, int]] = None,
) -> int:
    """Count all occurrences of `ent` in the grid, optionally skipping one cell."""
    total = 0
    for gx in range(GRID_W):
        for gy in range(GRID_H):
            if exclude and (gx, gy) == exclude:
                continue
            if grid[gx][gy].get_ent() == ent:
                total += 1
    return total


def _count_ghosts(
    grid: List[List[Cell]],
    exclude: Optional[Tuple[int, int]] = None,
) -> int:
    """Count all ghost entities (any colour) in the grid."""
    total = 0
    for gx in range(GRID_W):
        for gy in range(GRID_H):
            if exclude and (gx, gy) == exclude:
                continue
            if grid[gx][gy].get_ent() in GHOST_ENTITIES:
                total += 1
    return total


def grid_summary(grid: List[List[Cell]]) -> dict:
    """
    Return a dict of entity/background counts for the stats panel.

    Keys: "walls", "gums", "power_pellets", "ghosts",
          "pacman", "fruit", "entities"
    """
    from models import BG as _BG  # local import avoids circularity
    walls = gums = pellets = ghosts = pacman = fruit = 0
    for gx in range(GRID_W):
        for gy in range(GRID_H):
            c = grid[gx][gy]
            bg, ent = c.get_bg(), c.get_ent()
            if bg == _BG.WALL:     walls   += 1
            if bg == _BG.GUM:      gums    += 1
            if bg == _BG.ENERGIZE: pellets += 1
            if ent in GHOST_ENTITIES: ghosts += 1
            if ent == ENT.PACMAN:  pacman  += 1
            if ent == ENT.FRUIT:   fruit   += 1
    return {
        "walls":         walls,
        "gums":          gums,
        "power_pellets": pellets,
        "ghosts":        ghosts,
        "pacman":        pacman,
        "fruit":         fruit,
        "entities":      ghosts + pacman + fruit,
    }
