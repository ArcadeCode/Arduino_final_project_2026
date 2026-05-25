# editor/gui/stats_panel.py
# Left-side statistics panel: dot count, wall count, entity count, hover info.
# Phase 2: dot counter is color-coded and warns when the MAX_DOTS limit is approached.

import tkinter as tk
from typing import List, List

from ..constants import MAX_DOTS, GRID_W, GRID_H
from ..models import BG, ENT, Cell
from ..rules import count_dots
from .theme import BG_DARK, FG_PRIMARY, FG_SECONDARY, FG_DIM


class StatsPanel:
    """
    Statistics widget rendered inside a parent tk.Frame.

    Call refresh(grid) after any grid modification to update all counters.
    Call set_hover(x, y, cell) when the mouse moves over a cell.
    Call clear_hover() when the mouse leaves the canvas.
    """

    def __init__(self, parent: tk.Frame) -> None:
        tk.Label(parent, text="", bg=BG_DARK).pack(pady=4)
        tk.Label(
            parent, text="STATISTICS",
            font=("Courier", 9, "bold"),
            fg=FG_SECONDARY, bg=BG_DARK,
        ).pack(anchor=tk.W)

        self._labels: dict = {}
        for key in ["Walls", "Pac-gums", "Super gums", "Dots total", "Entities"]:
            row = tk.Frame(parent, bg=BG_DARK)
            row.pack(fill=tk.X, pady=1)
            tk.Label(
                row, text=key + ":", font=("Courier", 8),
                fg=FG_DIM, bg=BG_DARK, width=11, anchor=tk.W,
            ).pack(side=tk.LEFT)
            lbl = tk.Label(
                row, text="0", font=("Courier", 8, "bold"),
                fg=FG_PRIMARY, bg=BG_DARK,
            )
            lbl.pack(side=tk.LEFT)
            self._labels[key] = lbl

        # Hover info
        tk.Label(parent, text="", bg=BG_DARK).pack(pady=2)
        self._hover_label = tk.Label(
            parent, text="[x=-, y=-]",
            font=("Courier", 8), fg="#555588",
            bg=BG_DARK, wraplength=150, justify=tk.LEFT,
        )
        self._hover_label.pack(anchor=tk.W)

    def refresh(self, grid: List[List[Cell]]) -> None:
        """Recompute all counters from the current grid and update labels."""
        walls = gums = energize = entities = 0
        for x in range(GRID_W):
            for y in range(GRID_H):
                c = grid[x][y]
                bg, ent = c.get_bg(), c.get_ent()
                if bg == BG.WALL:     walls     += 1
                if bg == BG.GUM:      gums      += 1
                if bg == BG.ENERGIZE: energize  += 1
                if ent != ENT.EMPTY:  entities  += 1

        total_dots = gums + energize

        self._labels["Walls"].configure(text=str(walls))
        self._labels["Pac-gums"].configure(text=str(gums))
        self._labels["Super gums"].configure(text=str(energize))
        self._labels["Entities"].configure(text=str(entities))

        # Phase 2: color-code the dot total — warn when approaching MAX_DOTS
        if total_dots > MAX_DOTS:
            dot_color = "#FF3333"   # red  — limit exceeded
        elif total_dots > MAX_DOTS * 0.9:
            dot_color = "#FF8800"   # orange — within 10 % of the limit
        else:
            dot_color = FG_PRIMARY  # normal yellow

        self._labels["Dots total"].configure(
            text=f"{total_dots}/{MAX_DOTS}",
            fg=dot_color,
        )

    def set_hover(self, x: int, y: int, cell: Cell) -> None:
        from .theme import cell_visual
        _, _, name = cell_visual(cell)
        self._hover_label.configure(
            text=f"x={x}, y={y}\n{name}\ndata=0x{cell.data:02X}"
        )

    def clear_hover(self) -> None:
        self._hover_label.configure(text="[x=-, y=-]")
