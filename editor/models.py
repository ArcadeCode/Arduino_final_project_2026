"""
models.py — Core data model: BG/ENT enums and the Cell class.

This module has NO imports from other editor/ modules so it can be
imported freely by constants.py, rules.py, grid_io.py, and views.
"""

from enum import IntEnum
from typing import List


# ── Background enum (matches C++ BG_* values) ────────────────────────────────

class BG(IntEnum):
    EMPTY    = 0   # BG_EMPTY
    WALL     = 1   # BG_WALL
    GUM      = 2   # BG_GUM
    ENERGIZE = 3   # BG_ENERGIZE
    GHOST_HOUSE=4  # GB_GHOST_HOUSE


# ── Entity enum (matches C++ ENT_* values) ───────────────────────────────────

class ENT(IntEnum):
    EMPTY        = 0   # ENT_EMPTY
    PACMAN       = 1   # ENT_PACMAN
    BLUE_GHOST   = 2   # ENT_BLUE_GHOST
    RED_GHOST    = 3   # ENT_RED_GHOST
    PINK_GHOST   = 4   # ENT_PINK_GHOST
    ORANGE_GHOST = 5   # ENT_ORANGE_GHOST
    FRUIT        = 6   # ENT_FRUIT


# ── Convenience sets ──────────────────────────────────────────────────────────

GHOST_ENTITIES = {ENT.BLUE_GHOST, ENT.RED_GHOST, ENT.PINK_GHOST, ENT.ORANGE_GHOST}


# ── Cell ──────────────────────────────────────────────────────────────────────

class Cell:
    """
    One grid cell, bit-packed with 3 bits for BG and 3 bits for ENT:

        bit:  7 6 5 4 3 2 1 0
              - - E E E B B B
              └───────┘└─────┘
               entity   bg
    """

    def __init__(self, data: int = 0):
        self.data: int = data & 0xFF

# ── Background accessors ──────────────────────────────────────────────────

    def get_bg(self) -> BG:
        return BG(self.data & 0b00000111)

    def set_bg(self, bg: BG) -> None:
        self.data = (self.data & 0b11111000) | (int(bg) & 0b00000111)

    # ── Entity accessors ──────────────────────────────────────────────────────

    def get_ent(self) -> ENT:
        return ENT((self.data >> 3) & 0b00000111)

    def set_ent(self, ent: ENT) -> None:
        self.data = (self.data & 0b11000111) | ((int(ent) & 0b00000111) << 3)

    # ── ASCII representation ──────────────────────────────────────────────────

    def to_char(self) -> str:
        """Single character that uniquely represents this cell's content."""
        ent = self.get_ent()
        if ent == ENT.PACMAN:       return 'P'
        if ent == ENT.RED_GHOST:    return 'R'
        if ent == ENT.BLUE_GHOST:   return 'B'
        if ent == ENT.PINK_GHOST:   return 'K'
        if ent == ENT.ORANGE_GHOST: return 'O'
        if ent == ENT.FRUIT:        return 'F'
        bg = self.get_bg()
        if bg == BG.WALL:           return '#'
        if bg == BG.GUM:            return '.'
        if bg == BG.ENERGIZE:       return '*'
        if bg == BG.GHOST_HOUSE:    return '-'  # Ajout de GB_GHOST_HOUSE
        return ' '

    # ── Utilities ─────────────────────────────────────────────────────────────

    def copy(self) -> "Cell":
        return Cell(self.data)

    def __repr__(self) -> str:  # pragma: no cover
        return f"Cell(bg={self.get_bg().name}, ent={self.get_ent().name})"

# ── Grid factory ─────────────────────────────────────────────────────────────

def empty_grid(width: int, height: int) -> List[List[Cell]]:
    """Return a blank width × height grid indexed [x][y]."""
    return [[Cell() for _ in range(height)] for _ in range(width)]
