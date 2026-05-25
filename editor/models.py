# editor/models.py
# Data model: BG/ENT enums and Cell class, mirroring the Arduino C++ types.
# grid[x][y] convention: x = column (horizontal), y = row (vertical).

from enum import IntEnum
from typing import List

from .constants import GRID_W, GRID_H


# ── Enums matching C++ (types.hpp) ───────────────────────────────────────────

class BG(IntEnum):
    """Cell background type — 2 bits, matches CellBackgroundType in C++."""
    EMPTY    = 0  # BG_EMPTY
    WALL     = 1  # BG_WALL
    GUM      = 2  # BG_GUM      ("Pac-gum")
    ENERGIZE = 3  # BG_ENERGIZE ("Super pac-gum")


class ENT(IntEnum):
    """Cell entity type — 3 bits, matches CellEntitiesType in C++."""
    EMPTY        = 0  # ENT_EMPTY
    PACMAN       = 1  # ENT_PACMAN
    BLUE_GHOST   = 2  # ENT_BLUE_GHOST
    RED_GHOST    = 3  # ENT_RED_GHOST
    PINK_GHOST   = 4  # ENT_PINK_GHOST
    ORANGE_GHOST = 5  # ENT_ORANGE_GHOST
    FRUIT        = 6  # ENT_FRUIT


# ── Cell ─────────────────────────────────────────────────────────────────────

class Cell:
    """
    Single grid cell, bit-packed identically to the Arduino C++ Cell struct.

    Bit layout (8 bits):
        bit:  7 6 5 4 3 2 1 0
              - - - E E E B B
        B = background (2 bits)
        E = entity     (3 bits)
    """

    def __init__(self, data: int = 0) -> None:
        self.data = data & 0xFF

    # -- Background -----------------------------------------------------------

    def get_bg(self) -> BG:
        return BG(self.data & 0b00000011)

    def set_bg(self, bg: BG) -> None:
        self.data = (self.data & 0b11111100) | (int(bg) & 0b00000011)

    # -- Entity ---------------------------------------------------------------

    def get_ent(self) -> ENT:
        return ENT((self.data >> 2) & 0b00000111)

    def set_ent(self, ent: ENT) -> None:
        self.data = (self.data & 0b11100011) | ((int(ent) & 0b00000111) << 2)

    # -- Helpers --------------------------------------------------------------

    def to_char(self) -> str:
        """ASCII representation used for debug and the ASCII export tab."""
        ent = self.get_ent()
        if ent == ENT.PACMAN:       return 'P'
        if ent == ENT.RED_GHOST:    return 'R'
        if ent == ENT.BLUE_GHOST:   return 'B'
        if ent == ENT.PINK_GHOST:   return 'K'
        if ent == ENT.ORANGE_GHOST: return 'O'
        if ent == ENT.FRUIT:        return 'F'
        bg = self.get_bg()
        if bg == BG.WALL:     return '#'
        if bg == BG.GUM:      return '.'
        if bg == BG.ENERGIZE: return '*'
        return ' '

    def copy(self) -> "Cell":
        return Cell(self.data)

    def __repr__(self) -> str:
        return f"Cell(bg={self.get_bg().name}, ent={self.get_ent().name})"


# ── Grid factory ─────────────────────────────────────────────────────────────

def empty_grid() -> List[List[Cell]]:
    """
    Return a blank GRID_W × GRID_H grid, indexed [x][y].
    x = column (0..GRID_W-1), y = row (0..GRID_H-1).
    """
    return [[Cell() for _ in range(GRID_H)] for _ in range(GRID_W)]
