# editor/rules.py
# Phase 2 — Grid validation rules.
#
# validate_grid() scans the level and returns a (possibly empty) list of
# human-readable violation strings.  The GUI calls this before any export
# or save operation and shows the violations to the user.
#
# Rules enforced:
#   R1 — Exactly one Pac-Man spawn point (MAX_PACMAN = 1).
#   R2 — At most one of each ghost personality (max MAX_GHOSTS total).
#   R3 — At most one fruit spawn point (MAX_FRUIT = 1).
#   R4 — Total collectible cells (BG_GUM + BG_ENERGIZE) ≤ MAX_DOTS (255).
#        This is a hard Arduino constraint: totalDots is stored as uint8_t.

from typing import List

from .constants import MAX_DOTS, MAX_GHOSTS, MAX_PACMAN, MAX_FRUIT
from .models import BG, ENT, Cell, GRID_W, GRID_H


def count_dots(grid: List[List[Cell]]) -> int:
    """Return the total number of collectible cells (BG_GUM + BG_ENERGIZE)."""
    return sum(
        1
        for x in range(GRID_W)
        for y in range(GRID_H)
        if grid[x][y].get_bg() in (BG.GUM, BG.ENERGIZE)
    )


def validate_grid(grid: List[List[Cell]]) -> List[str]:
    """
    Validate the level against all declared rules.

    Returns a list of violation strings (empty list = level is valid).
    Each string is ready to be displayed directly in a messagebox or status bar.
    """
    violations: List[str] = []

    # ── Count entities ────────────────────────────────────────────────────────
    pacman_count = 0
    fruit_count  = 0
    ghost_counts = {
        ENT.RED_GHOST:    0,
        ENT.PINK_GHOST:   0,
        ENT.BLUE_GHOST:   0,
        ENT.ORANGE_GHOST: 0,
    }

    for x in range(GRID_W):
        for y in range(GRID_H):
            ent = grid[x][y].get_ent()
            if ent == ENT.PACMAN:
                pacman_count += 1
            elif ent == ENT.FRUIT:
                fruit_count += 1
            elif ent in ghost_counts:
                ghost_counts[ent] += 1

    # R1 — Pac-Man
    if pacman_count == 0:
        violations.append("No Pac-Man spawn point found. Place exactly one 'P'.")
    elif pacman_count > MAX_PACMAN:
        violations.append(
            f"Too many Pac-Man spawn points ({pacman_count}). "
            f"Only {MAX_PACMAN} allowed."
        )

    # R2 — Ghosts (each personality is unique; total ≤ MAX_GHOSTS)
    ghost_names = {
        ENT.RED_GHOST:    "Red ghost",
        ENT.PINK_GHOST:   "Pink ghost",
        ENT.BLUE_GHOST:   "Blue ghost",
        ENT.ORANGE_GHOST: "Orange ghost",
    }
    for ent, name in ghost_names.items():
        count = ghost_counts[ent]
        if count > 1:
            violations.append(
                f"Duplicate {name} ({count} placed). Each ghost personality must appear at most once."
            )

    total_ghosts = sum(ghost_counts.values())
    if total_ghosts > MAX_GHOSTS:
        violations.append(
            f"Too many ghosts ({total_ghosts}). Maximum is {MAX_GHOSTS}."
        )

    # R3 — Fruit
    if fruit_count > MAX_FRUIT:
        violations.append(
            f"Too many fruit spawn points ({fruit_count}). Only {MAX_FRUIT} allowed."
        )

    # R4 — Dot count (Arduino uint8_t constraint)
    dots = count_dots(grid)
    if dots > MAX_DOTS:
        violations.append(
            f"Too many collectible cells ({dots}). "
            f"Maximum is {MAX_DOTS} (Arduino uint8_t limit). "
            f"Remove {dots - MAX_DOTS} Pac-gum(s) or Super pac-gum(s)."
        )

    return violations
